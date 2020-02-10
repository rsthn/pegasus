/*
**	psxt::Generator
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

namespace psxt
{
	/**
	**	Base class responsible for generating code respective to a finite state machine. This class is a base and all derived
	**	implementations should define the abstract functions to generate code for different languages.
	*/

	class Generator
	{
		protected:

		/**
		**	Internal static buffer for string formatting.
		*/
		static char buffer[];

		/**
		**	General context.
		*/
		Context *context;

		/**
		**	Output stream currently used to write data.
		*/
		FILE *os;

		/**
		**	Sets the default output stream for the generator.
		*/
		void setOutputStream (FILE *output)
		{
			this->os = output;
		}

		/**
		**	Writes the given buffer to the output stream.
		*/
		void write (const char *data, int length = -1)
		{
			fwrite (data, 1, length != -1 ? length : strlen(data), os);
		}

		/**
		**	Executes a printf into the output stream.
		*/
		void writef (const char *msg, ...)
		{
			va_list args;
			va_start (args, msg);

			vsprintf (buffer, msg, args);
			write (buffer);
		}

		/**
		**	Replaces argument markers ($0, $1, etc) on the given string and writes to the output stream. Returns a list with the markers
		**	that were not used in the input string. Usually the code generator will add auto-deletion code for all variables that were not
		**	used, if you use a variable but want to still delete it, use double-dollar instead ($$0, $$1, etc) to force a "unused" flag.
		*/
		List<Pair<int, const char*>*> *replaceMarkers (const char *str, int max, int section, ProductionRule *rule, const char *deft)
		{
			List<Pair<int, const char*>*> *left = new List<Pair<int, const char*>*> ();
			int i, m, state = 0;

			for (i = 0; i < max; i++)
			{
				// VIOLET:TODO:REMOVE
				// NonTerminal *nt = context->getNonTerminal (section, rule->getElem(i)->getValue());
				NonTerminal *nt = rule->getElem(i)->getNonTerminal();
				left->push (new Pair <int, const char*> (i, nt != nullptr && nt->getReturnType() != nullptr ? nt->getReturnType()->c_str() : deft));
			}

			while (state != -1)
			{
				switch (state)
				{
					case 0:
						if (*str == '\0')
						{
							state = -1;
							break;
						}

						if (*str++ <= 32) break;
						str--, state = 1;

					case 1:
						if (*str == '\0')
						{
							state = -1;
							break;
						}

						if (*str == '$' && (str[1] >= '0' && str[1] <= '9'))
						{
							str++, i = 0, m = 0, state = 2;
						}
						else if (*str == '$' && str[1] == '$')
						{
							str+=2, i = 0, m = 1, state = 2;
						}
						else
						{
							putc (*str++, os);
							break;
						}

					case 2:
						if (*str < '0' || *str > '9')
						{
							if (i < max)
							{
								// VIOLET:TODO:REMOVE
								// NonTerminal *nt = context->getNonTerminal (section, rule->getElem(i)->getValue());
								NonTerminal *nt = rule->getElem(i)->getNonTerminal();

								if (nt != nullptr && nt->getReturnType() != nullptr)
								{
									if (isNull(nt->getReturnType()->c_str()))
										writef ("argv[bp-%u]", max-i);
									else
										writef ("((%s)argv[bp-%u])", nt->getReturnType()->c_str(), max-i);
								}
								else
								{
									if (isNull(deft))
										writef ("argv[bp-%u]", max-i);
									else
										writef ("((%s)argv[bp-%u])", deft, max-i);
								}

								// The "m" indicates if used variable with $$ or $. When using $$ the variable will be forced to be marked as "unused",
								// and usually unused variables will cause the generator to add code to delete it.
								if (m == 0)
								{
									for (Linkable<Pair<int, const char*>*> *z = left->top; z; z = z->next)
									{
										if (z->value->a == i)
										{
											delete left->remove(z);
											break;
										}
									}
								}
							}
							else
							{
								printf ("Out of bounds\n");
							}

							state = 1;
						}
						else
							i = (i * 10) + (*str++ - '0');

						break;
				}
			}

			return left;
		}

		/**
		**	Returns true if the specified type name is a null name.
		*/
		bool isNull (const char *s)
		{
			if (!strcmp (s, "null")) return true;
			if (!strcmp (s, "nullptr")) return true;
			if (!strcmp (s, "NULL")) return true;

			return false;
		}

		/**
		**	Returns true if the specified type name is a constant pointer.
		*/
		bool isConstPtr (const char *s)
		{
			if (!strcmp (s, "null")) return true;
			if (!strcmp (s, "nullptr")) return true;
			if (!strcmp (s, "char")) return true;
			if (!strcmp (s, "short")) return true;
			if (!strcmp (s, "int")) return true;
			if (!strcmp (s, "long")) return true;
			if (!strcmp (s, "float")) return true;
			if (!strcmp (s, "double")) return true;

			return false;
		}

		/**
		**	Returns the index of a exported symbol found on the specified section.
		*/
		int getExportId (int section, Token *token)
		{
			if (token->getType() == TTYPE_EOF)
				return -1;

			return context->getExports(section)->indexOf(token->getValue(), token->getValue()->getHash()) + 256;
		}

		/**
		**	Returns the index of a exported symbol found on the specified section.
		*/
		int getExportId (int section, LString *name)
		{
			return context->getExports(section)->indexOf(name, name->getHash()) + 256;
		}

		/**
		**	Writes the given template to the output stream. The argument markers in the template will be replaced by its respective
		**	value. Argument markers are $0 for the name parameter, $1 for the state machine code, $T for return type of the state
		**	machine, $R which denotes the return value of the state machine, and $E for the section epilogue. Marker $1 is obtained
		**	by executing generate with a list of states and a section index (method to be overidden by child class).
		*/
		void writeTemplate (List<FsmState*> *states, int section, String *name, char *tem, int length)
		{
			char *s, *end = tem + length;
			LString *tmp;

			for (s = tem; tem < end; tem++)
			{
				if (*tem != '$') continue;

				write (s, tem - s);
				s = tem + 2;

				switch (*++tem)
				{
					case '0':
						write (name->c_str());
						break;

					case '1':
						generate (states, section);
						break;

					case 'E':
						epilogue (section);
						break;

					case 'T':
						// VIOLET:TODO:REMOVE
						// tmp = context->getNonTerminal(section, context->getNonTerminal(section, 0)->getRules()->getAt(0)->getElems()->getAt(0)->getValue())->getReturnType();
						tmp = context->getNonTerminal(section, 0)->getRules()->getAt(0)->getElems()->getAt(0)->getNonTerminal()->getReturnType();
						write (tmp != nullptr ? (tmp->cequals("null") ? "void *" : tmp->c_str()) : "void *");
						break;

					case 'R':
						// VIOLET:TODO:REMOVE
						// tmp = context->getNonTerminal(section, context->getNonTerminal(section, 0)->getRules()->getAt(0)->getElems()->getAt(0)->getValue())->getReturnType();
						tmp = context->getNonTerminal(section, 0)->getRules()->getAt(0)->getElems()->getAt(0)->getNonTerminal()->getReturnType();
						writef ("(%s)%s", tmp != nullptr ? (tmp->cequals("null") ? "void *" : tmp->c_str()) : "void *", tmp != nullptr && tmp->cequals("null") ? "nullptr" : "argv[0]");
						break;
				}
			}

			write (s, tem - s);
		}

		public:

		/**
		**	Constructs the instance of the generator.
		*/
		Generator (Context *context)
		{
			this->context = context;
			this->os = nullptr;
		}

		/**
		**	Destroys the instance of the generator.
		*/
		virtual ~Generator ()
		{
		}

		/**
		**	Prints an error message and returns the value of 1.
		*/
		int errmsg (Token *token, int code, const char *msg, ...)
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
		**	Generates code for the specified states of a given section and saves it to the specified output. This function should simply determine
		**	what kind of code template buffer should be used to generate code and used it on a call to writeTemplate.
		*/
		virtual void generate (List<FsmState*> *states, int section, FILE *output, String *name) = 0;

		/**
		**	Generates the code of a state machine given its states.
		*/
		virtual void generate (List<FsmState*> *states, int section) = 0;

		/**
		**	Generates the epilogue code to replace marker $E on the template.
		*/
		protected: virtual void epilogue (int section) = 0;
	};

	/**
	**	Internal buffer used by Generator::printf as temporal storage.
	*/
	char Generator::buffer[64];

};
