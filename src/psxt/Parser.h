#ifndef __PARSER_H
#define __PARSER_H

#include "Integer"

namespace psxt
{
	/**
	 * @brief Simple implementation of a syntax parser. This class will obtain tokens from a scanner and compose objects lexical and grammatical rules from it.
	 */

	using asr::utils::Integer;

	class Parser
	{
		protected:

		/**
		**	Compilation unit context.
		*/
		Context *context;

		/**
		**	Indicates if finalization rules have already been added to the context.
		*/
		int finished;

		public:

		/**
		**	Constructs the parser instance.
		*/
		Parser (Context *context)
		{
			this->context = context;
			this->finished = 0;
		}

		/**
		**	Destroys the instance of the parser.
		*/
		virtual ~Parser ()
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
		**	Verifies that all non terminals in the system have consistent return-types.
		*/
		private: bool ensureTypeConsistency (int section)
		{
			List<Integer*> *processed = new List<Integer*> ();

			bool retry = true;
			bool value = true;

			while (retry)
			{
				retry = false;

				for (Linkable<Pair<LString*, NonTerminal*>*> *i = context->getNonTerminalPairs(section)->head(); i; i = i->next())
				{
					if (i->value->value->getReturnType() != nullptr || processed->get(i->value->value->getId()) != nullptr)
						continue;

					LString *type = nullptr;
					bool hasTerminals = false;
					int emptyRules = 0;

					for (Linkable<ProductionRule*> *j = i->value->value->getRules()->head(); j; j = j->next())
					{
						if (j->value->getElems()->length() == 0)
						{
							emptyRules++;
							continue;
						}

						LString *nonTermName = nullptr;

						for (Linkable<Token*> *k = j->value->getElems()->head(); k; k = k->next())
						{
							if (k->value->getType() == Token::Type::END)
								continue;

							if (k->value->getType() == Token::Type::IDENTIFIER && k->value->getValue()->equals(i->value->value->getName()))
								continue;

							if (k->value->getType() != Token::Type::IDENTIFIER)
								continue;

							if (nonTermName == nullptr)
							{
								nonTermName = k->value->getValue();
								continue;
							}

							nonTermName = nullptr;
							break;
						}

						if (nonTermName == nullptr)
						{
							errmsg (nullptr, E_INFER_FAILED, i->value->value->getName()->c_str());
							value = false;

							processed->push (new Integer (i->value->value->getId()));
							break;
						}

						NonTerminal *nt = context->getNonTerminal(section, nonTermName);
						if (nt == nullptr)
						{
							hasTerminals = true;
							continue;
						}

						if (nt->getReturnType() == nullptr)
						{
							if (processed->get(nt->getId()) != nullptr)
							{
								errmsg (nullptr, E_INFER_FAILED, i->value->value->getName()->c_str());
								processed->push (new Integer (i->value->value->getId()));
							}
							else
								retry = true;

							break;
						}

						if (type == nullptr) type = nt->getReturnType();

						if (!type->equals (nt->getReturnType()))
						{
							errmsg ((Token *)(type = nullptr), E_INCONSISTENT_TYPE, i->value->value->getName()->c_str());
							value = false;

							processed->push (new Integer (i->value->value->getId()));
							break;
						}
					}

					if (type == nullptr && (hasTerminals || i->value->value->getRules()->length() == emptyRules))
					{
						i->value->value->setReturnType (LString::alloc("void *"));
					}
					else
					{
						if (type != nullptr)
							i->value->value->setReturnType (LString::alloc(type));
					}
				}
			}

			delete processed->clear();
			return value;
		}

		/**
		**	Parses the input from the given scanner and combines the data into the parser's context.
		*/
		public: int parse (Scanner *scanner)
		{
			int i, j, eof = 0, reuse = 0, state = 0, stateB, stateC;

			NonTerminal *nonterm, *temp;
			ProductionRule *production;

			Token *token = scanner->getToken();

			while (token != nullptr)
			{
				if (!reuse && !scanner->nextToken())
					eof = 1;

				reuse = 0;

				switch (state)
				{
					// Section definition.
					case 0:
						if (eof) { state = -1; break; }

						if (token->getType() != Token::Type::SYMBOL || token->getChar() != '[')
							return errmsg (token, E_EXPECTED_LBRACKET);

						if (!scanner->nextToken())
							return errmsg (token, E_UNEXPECTED_EOF);

						if (token->getType() != Token::Type::IDENTIFIER)
							return errmsg (token, E_EXPECTED_IDENTIFIER);

						if (!token->getValue()->cequals("lexicon") && !token->getValue()->cequals("grammar") && !token->getValue()->cequals("arrays"))
							return errmsg (token, E_INCORRECT_SECTION, token->getCstr());

						state = 1;

						if (token->getValue()->cequals("arrays"))
							state = 9;
						else
							stateB = token->getValue()->cequals("lexicon") ? Context::SectionType::LEXICON : Context::SectionType::GRAMMAR;

						if (!scanner->nextToken())
							return errmsg (token, E_UNEXPECTED_EOF);

						if (token->getType() != Token::Type::SYMBOL || token->getChar() != ']')
							return errmsg (token, E_EXPECTED_RBRACKET);

						break;

					// Grammar rule definition.
					case 1:
						if (eof) { state = -1; break; }

						if (token->getType() != Token::Type::IDENTIFIER)
						{
							if (token->getType() != Token::Type::SYMBOL)
								return errmsg (token, E_EXPECTED_IDENTIFIER);

							state = (reuse = 1) - 1;
							break;
						}

						nonterm = new NonTerminal (context->nextId(stateB), token->getValue()->clone());
						nonterm = context->addNonTerminal (stateB, nonterm);

						state = 2;
						break;

					// First production rule definition.
					case 2:
						if (eof) return errmsg (token, E_UNEXPECTED_EOF);

						if (token->getType() == Token::Type::BLOCK)
						{
							nonterm->setReturnType (LString::alloc(token->getValue()->getString()->trim()->c_str()));
							break;
						}

						if (token->getType() != Token::Type::SYMBOL || token->getChar() != ':')
							return errmsg (token, E_EXPECTED_COLON, nonterm->getName()->c_str());

						production = new ProductionRule (nonterm->nextId(), nonterm);
						nonterm->addRule (production);

						state = 4;
						break;

					// Alternative production rule definition.
					case 3:
						if (eof) return errmsg (token, E_UNEXPECTED_EOF);

						if (token->getType() == Token::Type::BLOCK)
						{
							production->setAction (LString::alloc(token->getValue()->getString()->trim()->c_str()));
							break;
						}

						if (token->getType() != Token::Type::SYMBOL || (token->getChar() != '|' && token->getChar() != ';'))
							return errmsg (token, E_MISSING_SEMICOLON, nonterm->getName()->c_str());

						if (token->getChar() == ';')
						{
							state = 1;
							break;
						}

						production = new ProductionRule (nonterm->nextId(), nonterm);
						nonterm->addRule (production);

						state = 4;
						break;

					// Production rule contents.
					case 4:
						if (eof) return errmsg (token, E_UNEXPECTED_EOF);

						if (token->getType() == Token::Type::BLOCK)
						{
							state = reuse = 3;
							break;
						}

						if (token->getType() == Token::Type::SYMBOL)
						{
							if (token->getChar() == '(')
							{
								if (stateB == Context::SectionType::LEXICON)
								{
									errmsg (token, E_NOT_ALLOWED_VALUE);
									state = 7;
								}
								else
									state = 5;

								break;
							}

							if (token->getChar() == ';' || token->getChar() == '|')
							{
								if (production->getElems()->length() == 1 && production->getElems()->head()->value->getValue()->equals (nonterm->getName()))
									errmsg (token, E_INVALID_RECURSION, production->getId(), nonterm->getName()->c_str());

								if (production->getVisibility() == 1)
								{
									if (stateB == Context::SectionType::LEXICON)
									{
										if (production->getElems()->length() == 0)
											errmsg (token, E_REQUIRED_ELEM, nonterm->getName()->c_str(), production->getId());
										else
											context->addExport (stateB, production->getElems()->head()->value->getValue(), production);
									}
									else
										printf("!!!!!!!!!!!");//context->addExport (stateB, production);
								}

								state = reuse = 3;
								break;
							}

							if (token->getChar() == '+' || token->getChar() == '-')
							{
								production->setVisibility (token->getChar() == '+' ? ProductionRule::Visibility::PUBLIC : ProductionRule::Visibility::PRIVATE);
								break;
							}

							return errmsg (token, E_UNEXPECTED_ELEM, token->getCstr(), nonterm->getName()->c_str());
						}

						if (token->getType() == Token::Type::IDENTIFIER)
						{
							production->addElem (new Token(token));
						}
						else if (token->getType() == Token::Type::SQSTRING || token->getType() == Token::Type::DQSTRING)
						{
							if (stateB == Context::SectionType::LEXICON)
							{
								if (token->getValue()->charAt(0) != '[' && token->getValue()->length > 1 && token->getValue()->charAt(0) != '\\')
								{
									char *temp = (char *)token->getValue()->c_str();

									for (int i = 0; i < token->getValue()->length; i++)
									{
										int j = temp[i+1];
										temp[i+1] = 0;
										production->addElem (new Token(token, LString::alloc(&temp[i]), false));
										temp[i+1] = j;
									}
								}
								else
									production->addElem (new Token(token));
							}
							else
								production->addElem (new Token(token));
						}
						else
							return errmsg (token, E_UNEXPECTED_ELEM, token->getCstr(), nonterm->getName()->c_str());

						break;

					// Production rule per-element value specifier. Allowed to appear only in language grammar section, used to
					// specify the value of a non-terminal (nvalue).
					case 5:
						if (eof) return errmsg (token, E_UNEXPECTED_EOF);

						if (token->getType() != Token::Type::SQSTRING && token->getType() != Token::Type::DQSTRING)
							return errmsg (token, E_EXPECTED_VALUE, production->getElems()->tail()->value->getCstr(), nonterm->getName()->c_str(), production->getId());

						production->addElem ((new Token(token))->setType(Token::Type::NVALUE));

						state = 6;
						break;

					// End of production rule per-element value specifier. (RPAREN)
					case 6:
						if (eof) return errmsg (token, E_UNEXPECTED_EOF);

						if (token->getType() != Token::Type::SYMBOL && token->getChar() != ')')
							return errmsg (token, E_EXPECTED_RPAREN, production->getElems()->tail()->value->getCstr(), nonterm->getName()->c_str(), production->getId());

						state = 4;
						break;

					// Ignoring: Production rule per-element value specifier.
					case 7:
						if (eof) return errmsg (token, E_UNEXPECTED_EOF);

						if (token->getType() != Token::Type::SQSTRING && token->getType() != Token::Type::DQSTRING)
							return errmsg (token, E_EXPECTED_VALUE, production->getElems()->tail()->value->getCstr(), nonterm->getName()->c_str(), production->getId());

						state = 8;
						break;

					// Ignoring: End of production rule per-element value specifier. (RPAREN)
					case 8:
						if (eof) return errmsg (token, E_UNEXPECTED_EOF);

						if (token->getType() != Token::Type::SYMBOL && token->getChar() != ')')
							return errmsg (token, E_EXPECTED_RPAREN, production->getElems()->tail()->value->getCstr(), nonterm->getName()->c_str(), production->getId());

						state = 4;
						break;

					// Start of array definition.
					case 9:
						if (eof) { state = -1; break; }

						if (token->getType() == Token::Type::SYMBOL && token->getChar() == '[')
						{
							state = 0; reuse = 1;
							break;
						}

						if (token->getType() != Token::Type::IDENTIFIER)
							return errmsg (token, E_EXPECTED_IDENTIFIER);

						// VIOLET:ENSURE NO DUPLICATE
						nonterm = new NonTerminal (context->nextId(Context::SectionType::LEXICON), token->getValue()->clone());
						nonterm = context->addNonTerminal (Context::SectionType::ARRAYS, nonterm);

						if (!scanner->nextToken())
							return errmsg (token, E_UNEXPECTED_EOF);

						if (token->getType() != Token::Type::SYMBOL || token->getChar() != '(')
							return errmsg (token, E_EXPECTED_LPAREN, nonterm->getName()->c_str());

						if (!scanner->nextToken())
							return errmsg (token, E_UNEXPECTED_EOF);

						if (token->getType() != Token::Type::IDENTIFIER)
							return errmsg (token, E_EXPECTED_LNONTERM, nonterm->getName()->c_str());

						nonterm->setReturnType (LString::alloc(token->getValue()));

						if (!scanner->nextToken())
							return errmsg (token, E_UNEXPECTED_EOF);

						if (token->getType() != Token::Type::SYMBOL || token->getChar() != ')')
							return errmsg (token, E_EXPECTED_RPAREN2, nonterm->getName()->c_str());

						if (!scanner->nextToken())
							return errmsg (token, E_UNEXPECTED_EOF);

						if (token->getType() != Token::Type::SYMBOL || token->getChar() != ':')
							return errmsg (token, E_EXPECTED_COLON2, nonterm->getName()->c_str());

						production = new ProductionRule (nonterm->nextId(), nonterm);
						nonterm->addRule (production);

						while (true)
						{
							if (!scanner->nextToken())
								return errmsg (token, E_MISSING_SEMICOLON, nonterm->getName()->c_str());

							if (token->getType() == Token::Type::SYMBOL && token->getChar() == ';')
								break;

							if (token->getType() != Token::Type::SQSTRING && token->getType() != Token::Type::DQSTRING)
								return errmsg (token, E_EXPECTED_ITEM, nonterm->getName()->c_str());

							//VIOLET:DO NOT ALLOW DUPLICATES
							production->addElem (new Token(token));

							if (!scanner->nextToken())
								return errmsg (token, E_MISSING_SEMICOLON, nonterm->getName()->c_str());

							if (token->getType() == Token::Type::SYMBOL && token->getChar() == ';')
								break;

							if (token->getType() != Token::Type::SYMBOL || token->getChar() != ',')
								return errmsg (token, E_EXPECTED_COMMA, production->getElems()->tail()->value->getCstr(), nonterm->getName()->c_str());
						}

						// VIOLET:ENSURE NO DUPLICATE
						context->addExport (Context::SectionType::LEXICON, nonterm->getName(), production);
						break;
				}

				if (state == -1) break;
			}

			if (state != -1)
				return errmsg (token, E_UNEXPECTED_EOF);

			Linkable<Pair<LString*, NonTerminal*>*> *pair;

			pair = context->getNonTerminalPairs(Context::SectionType::LEXICON)->head();
			if (pair != nullptr && !(finished & 1))
			{
				for (Linkable<ProductionRule*> *i = pair->value->value->getRules()->head(); i; i = i->next())
				{
					if (i->value->getVisibility() != 0) continue;

					if (i->value->getElems()->length() == 0) {
						i->value->setVisibility (ProductionRule::Visibility::PRIVATE);
					}
					else {
						context->addExport (Context::SectionType::LEXICON, i->value->getElems()->head()->value->getValue(), i->value);
						i->value->setVisibility (ProductionRule::Visibility::PUBLIC);
					}
				}

				nonterm = new NonTerminal (0, LString::alloc("__start__"));
				context->addNonTerminal (Context::SectionType::LEXICON, nonterm);

					temp = new NonTerminal (context->nextId (Context::SectionType::LEXICON), LString::alloc("__tokens"));
					context->addNonTerminal (Context::SectionType::LEXICON, temp);

					temp->addRule (production = new ProductionRule (temp->nextId(), temp));
					production->addElem (scanner->buildToken (Token::Type::IDENTIFIER, "__tokens"));
					production->addElem (scanner->buildToken (Token::Type::IDENTIFIER, (pair->value->key)->c_str()));

					temp->addRule (production = new ProductionRule (temp->nextId(), temp));
					production->addElem (scanner->buildToken (Token::Type::IDENTIFIER, (pair->value->key)->c_str()));

				nonterm->addRule (production = new ProductionRule (nonterm->nextId(), nonterm));
				production->addElem (scanner->buildToken (Token::Type::IDENTIFIER, "__tokens"));
				production->addElem (scanner->buildToken (Token::Type::END, ""));

				nonterm->addRule (production = new ProductionRule (nonterm->nextId(), nonterm));
				production->addElem (scanner->buildToken (Token::Type::END, ""));

				finished |= 1;
			}

			pair = context->getNonTerminalPairs(Context::SectionType::GRAMMAR)->head();
			if (pair != nullptr && !(finished & 2))
			{
				nonterm = new NonTerminal (0, LString::alloc("__start__"));
				context->addNonTerminal (Context::SectionType::GRAMMAR, nonterm);

				nonterm->addRule (production = new ProductionRule (nonterm->nextId(), nonterm));
				production->addElem (scanner->buildToken (Token::Type::IDENTIFIER, (pair->value->key)->c_str()));
				production->addElem (scanner->buildToken (Token::Type::END, ""));
				production->setAction (LString::alloc("$0"));

				finished |= 2;

				if (!ensureTypeConsistency(Context::SectionType::GRAMMAR))
					return 2;
			}

			return 0;
		}
	};
};

#endif
