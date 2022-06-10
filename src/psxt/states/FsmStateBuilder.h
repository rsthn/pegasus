/*
**	psxt::FsmStateBuilder
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

#include <psxt/states/FsmState.h>

namespace psxt
{	
	/**
	**	The responsibility of this class is to convert itemsets into finite-state-machine states.
	*/

	class FsmStateBuilder
	{
		protected:

		/**
		**	Prints an error message and returns the value of 1.
		*/
		static int errmsg (Token *token, int code, const char *msg, ...)
		{
			va_list args;
			va_start (args, msg);

			if (token != nullptr)
				printf ("[%c%u] %s %u,%u: ", code <= 4999 ? 'W' : 'E', code, token->getSource()->c_str(), token->getLine(), token->getCol());
			else
				printf ("[%c%u]: ", code <= 4999 ? 'W' : 'E', code);

			vprintf (msg, args);

			printf ("\n");

			return 1;
		}

		/**
		**	Builds an FSM state from an itemset, adds it to the completed list and returns it.
		*/
		static FsmState *build (Context *context, int section, ItemSet *root, List<FsmState*> *completed)
		{
			FsmState *state = completed->get(root->getId());
			if (state != nullptr) return state;

			// Insert new state in ascending order (by id).
			state = new FsmState (root->getId());

			if (completed->head() == nullptr || completed->head()->value->getId() > state->getId())
			{
				completed->unshift (state);
			}
			else
			{
				for (Linkable<FsmState*> *i = completed->tail(); i; i = i->prev())
				{
					if (i->value->getId() < state->getId())
					{
						completed->insertAfter (i, state);
						break;
					}
				}
			}

			// Process each item in the itemset.
			for (Linkable<Item*> *i = root->getItems()->head(); i; i = i->next())
			{
				Token *elem = i->value->getElem();

				// Reduction.
				if (elem == nullptr)
				{
					state->addAction (new FsmState::ReduceAction (i->value));
					continue;
				}

				FsmState *nextState = build (context, section, i->value->getTransition(), completed);

				// Shift or Goto.
				if (elem->getType() == TTYPE_IDENTIFIER)
				{
					// Goto.
					NonTerminal *nonterm = context->getNonTerminal (section, elem->getValue());
					if (nonterm != nullptr)
					{
						if (state->getGotoActions() != nullptr && state->getGotoActions()->contains(nextState->getId()))
							continue;

						state->addAction (new FsmState::GotoAction (nonterm, nextState));
					}
					// Shift.
					else
					{
						if (section == SECTION_LEXICON || !context->getExports(section-1)->contains(elem->getValue(), elem->getValue()->getHash()))
						{
							errmsg (elem, E_UNDEF_NONTERM, elem->getCstr());
						}
						else
						{
							if (state->getShiftActions() != nullptr && state->getShiftActions()->contains(elem))
								continue;

							state->addAction (new FsmState::ShiftAction (i->value->getRule(), elem, nextState));
						}
					}

					continue;
				}

				// Shift.
				if (state->getShiftActions() != nullptr && state->getShiftActions()->contains(elem))
					continue;

				state->addAction (new FsmState::ShiftAction (i->value->getRule(), elem, nextState));
			}

			// No reductions, no need for lookaheads.
			List<FsmState::ReduceAction*> *reductions = state->getReduceActions();
			if (reductions == nullptr) return state;

			// Single reduction without shifts, no need for lookaheads.
			if (state->getShiftActions() == nullptr && reductions->length() == 1)
				return state;

			// Single reduction with shifts, verify if look-aheads collide with the shifts.
			if (state->getShiftActions() != nullptr && reductions->length() == 1)
			{
				Linkable<FsmState::ShiftAction*> *i;

printf ("NEED TO CHECK FOR STATE %u (ITEM SIGNATURE=%s)\n", state->getId(), root->getSignature()->c_str());
root->dump(stdout);
printf ("\nREACH SET:\n");
auto rs = root->getReachSet();
if (rs == nullptr) printf("\nUNABLE TO GET REACH SET\n"); else rs->dump(stdout);
printf ("\n\n");
return state;
				List<Pair<Token*, ItemSet*>*> *tmp = nullptr;
				// TODO use this: reductions->head()->value->item->getFollow();

				for (i = state->getShiftActions()->head(); i; i = i->next())
				{
					if (tmp->contains (i->value->value))
						break;
				}

				// If no collision occurs, the single reduction will be a fallback when shifts fail.
				if (i == nullptr || section == SECTION_LEXICON)
					return state;

				errmsg (nullptr, E_SHIFT_REDUCE, SECTION_CODE(section), state->getId());
				// root->dump (stdout);

				return state;
			}

			// Lexicon section does not support lookaheads. Only single reductions must be present.
			if (section == SECTION_LEXICON)
			{
				if (reductions->length() > 1)
				{
					errmsg (nullptr, E_REDUCE_REDUCE, SECTION_CODE(section), state->getId());
				}

				return state;
			}

			// Load initial follow set of all reductions.
			for (Linkable<FsmState::ReduceAction*> *i = reductions->head(); i; i = i->next())
				i->value->loadFollow();

			// Verify if a reduction follow symbol collides with a shift symbol.
			bool err = false;

			if (state->getShiftActions() != nullptr)
			{
				for (Linkable<FsmState::ReduceAction*> *i = reductions->head(); i && !err; i = i->next())
				{
					for (Linkable<FsmState::ShiftAction*> *j = state->getShiftActions()->head(); j; j = j->next())
					{
						if (i->value->follow->contains (j->value->value))
						{
							err = true;
							break;
						}
					}
				}
			}

			if (err)
			{
				errmsg (nullptr, E_SHIFT_REDUCE, SECTION_CODE(section), state->getId());
				// root->dump (stdout);

				return state;
			}

			// Verify if a reduction follow symbol collides with another reduction.
			err = false;

			for (Linkable<FsmState::ReduceAction*> *i = reductions->head(); i && !err; i = i->next())
			{
				for (Linkable<FsmState::ReduceAction*> *j = i->next(); j && !err; j = j->next())
				{
					for (Linkable<Token*> *k = j->value->follow->head(); k; k = k->next())
					{
						if (i->value->follow->contains (k->value))
						{
							err = true;
							break;
						}
					}
				}
			}

			if (err)
			{
				errmsg (nullptr, E_REDUCE_REDUCE, SECTION_CODE(section), state->getId());
				// root->dump (stdout);

				return state;
			}

			return state;
		}

		/**
		**	Builds a sequence of states for a finite state machine (FSM) from a tree of itemsets.
		*/
		public: static List<FsmState*> *build (Context *context, int section, ItemSet *root)
		{
			List<FsmState*> *output = new List<FsmState*> ();

			build (context, section, root, output);

			return output;
		}
	};

};
