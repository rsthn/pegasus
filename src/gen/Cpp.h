/*
**	gen::Cpp
**
**	Copyright (c) 2006-2018, RedStar Technologies, All rights reserved.
**	https://rsthn.com/rstf/pegasus/
**
**	LICENSED UNDER THE TERMS OF THE "REDSTAR TECHNOLOGIES LIBRARY LICENSE" (RSTLL), YOU MAY NOT USE
**	THIS FILE EXCEPT IN COMPLIANCE WITH THE RSTLL. FIND A COPY IN THE "LICENSE.md" FILE IN YOUR SOURCE
**	OR BINARY DISTRIBUTION. THIS FILE IS PROVIDED BY REDSTAR TECHNOLOGIES "AS IS" AND ANY EXPRESS OR
**	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
**	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL REDSTAR TECHNOLOGIES BE LIABLE FOR
**	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
**	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
**	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
**	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
**	EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <tpl/scanner_cpp.h>
#include <tpl/parser_cpp.h>

namespace gen
{
	using namespace psxt;

	/**
	**	Generator for C++ code.
	*/

	class GeneratorCpp : public Generator
	{
		/**
		**	Temporal charset object used to get characters of a pattern string.
		*/
		protected: CharSet *charset;

		/**
		**	Generates the code of a state machine given its states.
		*/
		public: GeneratorCpp (Context *context) : Generator(context)
		{
			charset = new CharSet ();
		}

		/**
		**	Destructor.
		*/
		virtual ~GeneratorCpp ()
		{
			delete charset;
		}

		/**
		**	Writes code for a reduction to the output stream.
		*/
		protected: void writeReduction (FsmState::ReduceAction *reduce, int section, const char *nl)
		{
			writef ("%s nonterm = %u;", nl, reduce->rule->getNonTerminal()->getId());
			writef (" release = %u;", reduce->rule->getElems()->count);
			writef (" reduce = %u;", reduce->rule->getVisibility() + 1);

			if (reduce->rule->getVisibility() == 1)
				writef (" code = %d;", getExportId (section, reduce->rule->getElems()->top->value));

			if (section == SECTION_LEXICON)
				return;

			bool isNull = false;

			if (reduce->rule->getNonTerminal()->getReturnType() != nullptr && reduce->rule->getNonTerminal()->getReturnType()->cequals("null"))
				isNull = true;

			writef (" rule = %u;", reduce->rule->getId());
			writef (" shifted = %u;", reduce->rule->getLength());

			writef ("%s ", nl);

			if (!isNull) write ("temp = (void *)(");

			int maxMarkers = reduce->rule->getLength();

			List<Pair<int, const char*>*> *left = replaceMarkers (reduce->rule->getAction() ? reduce->rule->getAction()->c_str() : "nullptr", 
				maxMarkers, section, reduce->rule, "Token *");

			if (!isNull) write (")");

			write (";");

			for (Linkable<Pair<int, const char*>*> *i = left->top; i; i = i->next)
			{
				if (isConstPtr(i->value->b))
					continue;

				writef ("%s if (argv[bp-%u]) delete (%s)argv[bp-%u];", nl, 
					maxMarkers - i->value->a,
					i->value->b, maxMarkers - i->value->a);
			}

			delete left->clear();

			writef ("%s argv[bp-shifted] = %s;", nl, isNull ? "nullptr" : "temp");
		}

		/**
		**	Writes code for a lookahead condition.
		*/
		protected: void writeCondition (List<Token*> *list)
		{
			for (Linkable<Token*> *i = list->top; i; i = i->next)
			{
				if (i != list->top) write (" || ");

				if (i->value->getNValue() != nullptr)
					writef ("token->equals(%d, \"%s\")", getExportId(SECTION_LEXICON, i->value), i->value->getNValue()->getCstr());
				else
					writef ("symbol==%d", getExportId(SECTION_LEXICON, i->value));
			}
		}

		/**
		**	Generates code for a state machine.
		*/
		protected: virtual void generate (List<FsmState*> *states, int section)
		{
			const char *nl1 = "\n   ";
			const char *nl2 = "\n       ";
			const char *nl3 = "\n           ";
			const char *nl4 = "\n               ";

			write ("switch (state)\n{");

			for (Linkable<psxt::FsmState*> *i = states->top; i; i = i->next)
			{
				FsmState::ReduceAction *defaultReduction = nullptr;
				psxt::FsmState *state = i->value;

				writef ("%s case %u:", nl1, state->getId());

				// Write GOTO actions.
				if (state->getGotoActions() != nullptr)
				{
					writef ("%s if (reduce)", nl2);
					writef ("%s {", nl2);
					writef ("%s switch (nonterm)", nl3);
					writef ("%s {", nl3);

					for (Linkable<psxt::FsmState::GotoAction*> *i = state->getGotoActions()->top; i; i = i->next)
						writef ("%s case %u: state = %u; break;", nl4, i->value->nonterm->getId(), i->value->nextState->getId());

					writef ("%s }\n", nl3);
					writef ("%s reduce = 0;", nl3);
					writef ("%s break;", nl3);
					writef ("%s }\n", nl2);
				}

				// Write REDUCE actions.
				if (state->getReduceActions() != nullptr && section != SECTION_LEXICON)
				{
					for (Linkable<psxt::FsmState::ReduceAction*> *i = state->getReduceActions()->top; i; i = i->next)
					{
						if (i->value->follow == nullptr)
						{
							defaultReduction = i->value;
							continue;
						}

						writef ("%s if (", nl2);
						writeCondition (i->value->follow);
						write (") {");

						writeReduction (i->value, section, nl3);

						writef ("%s break;%s }\n", nl3, nl2);
					}
				}

				// Write SHIFT actions.
				if (state->getShiftActions() != nullptr)
				{
					writef ("%s switch (symbol)", nl2);
					writef ("%s {", nl2);

					switch (section)
					{
						case SECTION_LEXICON:
							for (Linkable<psxt::FsmState::ShiftAction*> *i = state->getShiftActions()->top; i; i = i->next)
							{
								writef ("%s ", nl3);

								if (i->value->value->getType() == TTYPE_EOF)
								{
									write ("case -1: ");
								}
								else
								{
									const char *buff = charset->set (i->value->value->getValue())->getBuffer();
									int j = 256;

									while (j--)
									{
										if (*buff++)
											writef ("case %u: ", 255 - j);
									}
								}

								writef ("state = %u; shift = 1; break;", i->value->nextState->getId());
							}

							break;

						case SECTION_GRAMMAR:
							for (Linkable<psxt::FsmState::ShiftAction*> *i = state->getShiftActions()->top; i; i = i->next)
							{
								if (i->value->nextState == nullptr)
									continue;

								writef ("%s ", nl3);

								if (i->value->value->getType() == TTYPE_EOF)
								{
									write ("case -1: ");
									writef ("state = %u; shift = 1; break;", i->value->nextState->getId());
									continue;
								}

								writef ("case %u: ", getExportId (SECTION_LEXICON, i->value->value));

								for (Linkable<psxt::FsmState::ShiftAction*> *j = i; j; j = j->next)
								{
									if (!j->value->value->equals (i->value->value))
										continue;

									if (j->value->value->getNValue() != nullptr)
										writef ("%s if (token->equals(\"%s\")) { ", nl4, j->value->value->getNValue()->getCstr());

									writef ("state = %u; shift = 1;", j->value->nextState->getId());

									if (j->value->value->getNValue() != nullptr)
										writef (" break; }");

									j->value->nextState = nullptr;
								}

								writef ("%s break;", nl4);
							}

							break;
					}

					writef ("%s }\n", nl2);
				}

				// Write last REDUCE action.
				if (defaultReduction != nullptr || (state->getReduceActions() != nullptr && section == SECTION_LEXICON))
				{
					if (!defaultReduction)
						defaultReduction = state->getReduceActions()->top->value;

					if (state->getShiftActions() != nullptr)
						writef ("%s if (shift) break;\n", nl2);

					writeReduction (defaultReduction, section, nl2);
				}

				if (defaultReduction == nullptr && state->getShiftActions() != nullptr)
					writef ("%s if (!shift) error = 1;", nl2);

				writef ("%s break;\n", nl2);
			}

			write ("}");
		}

		/**
		**	Generates the epilogue code to replace marker $E on the template.
		*/
		virtual void epilogue (int section)
		{
			if (section != SECTION_LEXICON)
				return;

			const char *nl3 = "           ";
			const char *nl4 = "\n               ";

			for (Linkable<Pair<LString*, NonTerminal*>*> *i = context->getNonTerminalPairs(SECTION_ARRAYS)->top; i; i = i->next)
			{
				int id = getExportId (SECTION_LEXICON, i->value->b->getReturnType());
				int id2 = getExportId (SECTION_LEXICON, i->value->a);

				writef ("%s if (token->getType() == %u)\n", nl3, id);
				writef ("%s {", nl3);

				for (Linkable<Token*> *j = i->value->b->getRules()->top->value->getElems()->top; j; j = j->next)
				{
					writef ("%s if (token->equals(\"%s\")) { return token->setType(%u); }", nl4, j->value->getCstr(), id2);
				}

				writef ("\n%s }\n", nl3);
			}

			writef ("\n%s return token;", nl3);
		}

		/**
		**	Generates code for the specified states of a given section and saves it to the specified output. This function should simply determine
		**	what kind of code template buffer should be used to generate code and used it on a call to generate/5.
		*/
		public: virtual void generate (List<FsmState*> *states, int section, FILE *output, String *name)
		{
			setOutputStream (output);

			if (section == SECTION_LEXICON)
				writeTemplate (states, section, name, (char*)&scanner_cpp[0], sizeof(scanner_cpp));
			else
				writeTemplate (states, section, name, (char*)&parser_cpp[0], sizeof(parser_cpp));
		}
	};

};
