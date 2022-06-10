/*
**	psxt::FsmState
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
	using asr::utils::String;
	using asr::utils::Integer;
	using asr::utils::Linkable;

	/**
	**	Describes an state for a finite state machine created specifically for parsing.
	*/

	class FsmState
	{
		public:

		/**
		**	Describes a Shift action.
		*/
		struct ShiftAction
		{
			Token *value;
			FsmState *nextState;
			ProductionRule *rule;

			ShiftAction(ProductionRule *rule, Token *value, FsmState *nextState)
			{
				this->rule = rule;
				this->nextState = nextState;
				this->value = value;
			}

			~ShiftAction()
			{
			}

			/**
			**	Compares the key of the shift. The parameter should be a casted token.
			*/
			int compare (Token *value)
			{
				return this->value->equals(value, true) ? 0 : 1;
			}
		};

		/**
		**	Describes a GoTo action.
		*/
		struct GotoAction
		{
			NonTerminal *nonterm;
			FsmState *nextState;

			GotoAction(NonTerminal *nonterm, FsmState *nextState)
			{
				this->nextState = nextState;
				this->nonterm = nonterm;
			}

			/**
			**	Compares the key of the next state. The parameter should be a casted int.
			*/
			int compare (int value)
			{
				return this->nextState->getId() - value;
			}
		};

		/**
		**	Describes a Reduce action.
		*/
		struct ReduceAction
		{
			ProductionRule *rule;
			List<Token*> *follow;
			Item *item;

			ReduceAction(Item *item)
			{
				this->rule = item->getRule();
				this->follow = nullptr;
				this->item = item;
			}

			void loadFollow ()
			{
				if (this->follow != nullptr) return;

				this->follow = new List<Token*>();

				// TODO fix

				/*for (Linkable<Pair<Token*, ItemSet*>*> *i = this->item->getFollow()->head(); i; i = i->next())
				{
					if (!this->follow->kcontains (i->value->a))
						this->follow->push (i->value->a);
				}*/
			}

			~ReduceAction()
			{
				if (this->follow != nullptr)
					delete this->follow->reset();
			}
		};

		/**
		**	Describes a Jump action.
		*/
		struct JumpAction
		{
			Token *value;
			FsmState *nextState;

			JumpAction(Token *value, FsmState *nextState)
			{
				this->nextState = nextState;
				this->value = value;
			}
		};

		protected:

		/**
		**	ID of the state.
		*/
		int id;

		/**
		**	List of shift actions.
		*/
		List<ShiftAction*> *shifts;

		/**
		**	List of goto actions.
		*/
		List<GotoAction*> *gotos;

		/**
		**	List of reduce actions.
		*/
		List<ReduceAction*> *reductions;

		/**
		**	List of jump actions.
		*/
		List<JumpAction*> *jumps;

		public:

		/**
		**	Initializes the state.
		*/
		FsmState (int id)
		{
			this->id = id;

			this->shifts = nullptr;
			this->gotos = nullptr;
			this->reductions = nullptr;
			this->jumps = nullptr;
		}

		/**
		**	Destroys the state instance.
		*/
		virtual ~FsmState ()
		{
			if (this->shifts)
			{
				this->shifts->clear();
				delete this->shifts;
			}

			if (this->reductions)
			{
				this->reductions->clear();
				delete this->reductions;
			}

			if (this->gotos)
			{
				this->gotos->clear();
				delete this->gotos;
			}

			if (this->jumps)
			{
				this->jumps->clear();
				delete this->jumps;
			}
		}

		/**
		**	Returns the ID of the state.
		*/
		int getId ()
		{
			return this->id;
		}

		/**
		**	Compares the key of the state (ID). The parameter should be a casted int.
		*/
		int compare (int value)
		{
			return this->id - value;
		}

		/**
		**	Adds a shift action to the state.
		*/
		void addAction (ShiftAction *a)
		{
			if (this->shifts == nullptr)
				this->shifts = new List<ShiftAction*> ();

			this->shifts->push (a);
		}

		/**
		**	Adds a goto action to the state.
		*/
		void addAction (GotoAction *a)
		{
			if (this->gotos == nullptr)
				this->gotos = new List<GotoAction*> ();

			this->gotos->push (a);
		}

		/**
		**	Adds a reduce action to the state.
		*/
		void addAction (ReduceAction *a)
		{
			if (this->reductions == nullptr)
				this->reductions = new List<ReduceAction*> ();

			this->reductions->push (a);
		}

		/**
		**	Adds a jump action to the state.
		*/
		void addAction (JumpAction *a)
		{
			if (this->jumps == nullptr)
				this->jumps = new List<JumpAction*> ();

			this->jumps->push (a);
		}

		/**
		**	Returns the list of actions.
		*/
		List<ShiftAction*> *getShiftActions ()
		{
			return this->shifts;
		}

		/**
		**	Returns the list of goto actions.
		*/
		List<GotoAction*> *getGotoActions ()
		{
			return this->gotos;
		}

		/**
		**	Returns the list of reduce actions.
		*/
		List<ReduceAction*> *getReduceActions ()
		{
			return this->reductions;
		}

		/**
		**	Returns the list of actions.
		*/
		List<JumpAction*> *getJumpActions ()
		{
			return this->jumps;
		}

		/**
		**	Dumps a reduction to the specified stream.
		*/
		void dump (FILE *output, psxt::FsmState::ReduceAction *reduce)
		{
			fprintf (output, "\n  > REDUCE %s#%u", reduce->rule->getNonTerminal()->getName()->c_str(), reduce->rule->getId());

			if (reduce->rule->getVisibility() != 0)
				fprintf (output, " %s", reduce->rule->getVisibility() == 1 ? "(EXPORT)" : "(IGNORE)");

			if (reduce->follow != nullptr)
			{
				for (Linkable<Token*> *i = reduce->follow->head(); i; i = i->next())
				{
					fprintf (output, "\n    > ON %s%s%s ", 
						i->value->getType() == TTYPE_EOF ? "(EOF)" : i->value->getCstr(), 
						i->value->getNValue() != nullptr ? ":" : "",
						i->value->getNValue() != nullptr ? i->value->getNValue()->getCstr() : ""
					);
				}
			}
		}

		/**
		**	Dumps the state to the specified stream.
		*/
		void dump (FILE *output)
		{
			fprintf (output, "STATE %u", id);

			if (this->reductions != nullptr)
			{
				for (Linkable<psxt::FsmState::ReduceAction*> *i = this->reductions->head(); i; i = i->next())
				{
					dump (output, i->value);
				}
			}

			if (this->gotos != nullptr)
			{
				for (Linkable<psxt::FsmState::GotoAction*> *i = this->gotos->head(); i; i = i->next())
					fprintf (output, "\n  > GOTO %u ON %s", i->value->nextState->getId(), i->value->nonterm->getName()->c_str());
			}

			if (this->jumps != nullptr)
			{
				for (Linkable<psxt::FsmState::JumpAction*> *i = this->jumps->head(); i; i = i->next())
					fprintf (output, "\n  > JUMP %u ON '%s'", i->value->nextState->getId(), i->value->value->getCstr());
			}

			if (this->shifts != nullptr)
			{
				for (Linkable<psxt::FsmState::ShiftAction*> *i = this->shifts->head(); i; i = i->next())
				{
					if (i->value->value->getType() == TTYPE_EOF)
						fprintf (output, "\n  > SHIFT (EOF) -> %u", i->value->nextState->getId());
					else
						fprintf (output, "\n  > SHIFT '%s%s%s' -> %u", i->value->value->getCstr(), 
							i->value->value->getNValue() != nullptr ? ":" : "",
							i->value->value->getNValue() != nullptr ? i->value->value->getNValue()->getCstr() : "",
							i->value->nextState->getId()
						);
				}
			}

			fprintf (output, "\n\n");
		}
	};
};
