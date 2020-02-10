/*
**	psxt::ProductionRule
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
	using asr::utils::LList;

	/**
	**	Definition of all the rule constants.
	*/
	#define	RASSOC_NONE		0
	#define	RASSOC_LEFT		1
	#define	RASSOC_RIGHT	2

	class NonTerminal;

	/**
	**	Describes a grammar production rule.
	*/

	class ProductionRule
	{
		protected:

		/**
		**	Production rule ID.
		*/
		int id;

		/**
		**	Specifies the public visibility of the rule (0=default, 1=public, 2=private). This modifies the behavior when generating the
		**	finite state machine and the final code.
		*/
		int visibility;

		/**
		**	Length of the production rule, also known as number of elements in the rule.
		*/
		int length;

		/**
		**	Related non-terminal.
		*/
		NonTerminal *nonterm;

		/**
		**	List of elements on the production rules. Each element is a token object.
		*/
		LList<Token*> *elems;

		/**
		**	Post-reduce action related to this production rule.
		*/
		LString *action;

		/**
		**	Rule precedence and associativity (0=none, 1=left, 2=right).
		*/
		int level, assoc;

		public:

		/**
		**	Initializes the production rule.
		*/
		ProductionRule (int id, NonTerminal *nonterm)
		{
			this->nonterm = nonterm;

			this->elems = new LList<Token*> ();
			this->id = id;
			this->visibility = 0;
			this->length = 0;

			this->level = 0;
			this->assoc = RASSOC_NONE;

			this->action = nullptr;
		}

		/**
		**	Destructor.
		*/
		virtual ~ProductionRule ()
		{
			if (this->action != nullptr)
				this->action->free();

			while (this->elems->count)
			{
				delete this->elems->pop();
			}

			delete this->elems;
		}

		/**
		**	Compares two production rules and returns true if both are equal.
		*/
		bool equals (ProductionRule *item)
		{
			LList<Token*> *listA = item->elems;
			LList<Token*> *listB = elems;

			if (listA->count != listB->count)
				return false;

			Linkable<Token*> *a = listA->top;
			Linkable<Token*> *b = listB->top;

			while (a != nullptr && b != nullptr)
			{
				if (!a->value->equals (b->value))
					break;

				a = a->next;
				b = b->next;
			}

			return a == nullptr && b == nullptr ? true : false;
		}

		/**
		**	Performs a comparison by a known key of the object. Returns true if both keys are equal.
		*/
		int compare (ProductionRule *item)
		{
			return equals(item) ? 0 : 1;
		}

		/**
		**	Returns the rule's ID.
		*/
		int getId ()
		{
			return this->id;
		}

		/**
		**	Returns the rule's related non-terminal.
		*/
		NonTerminal *getNonTerminal ()
		{
			return this->nonterm;
		}

		/**
		**	Returns the rule's level of precedence.
		*/
		int getLevel ()
		{
			return this->level;
		}

		/**
		**	Returns the rule's visibility mode.
		*/
		int getVisibility ()
		{
			return this->visibility;
		}

		/**
		**	Returns the rule's associativity mode.
		*/
		int getAssoc ()
		{
			return this->assoc;
		}

		/**
		**	Returns the rule's elements (tokens).
		*/
		LList<Token*> *getElems ()
		{
			return this->elems;
		}

		/**
		**	Returns the rule's action string.
		*/
		LString *getAction ()
		{
			return this->action;
		}

		/**
		**	Returns the length (number of elements) of the rule. This value is not equal to "getElems()->count" because this only
		**	counts non N-Value and non-EOF tokens.
		*/
		int getLength ()
		{
			return this->length;
		}

		/**
		**	Sets the rule's level of precedence.
		*/
		void setLevel (int level)
		{
			this->level = level;
		}

		/**
		**	Sets the visibility mode of the rule.
		*/
		void setVisibility (int value)
		{
			this->visibility = value;
		}

		/**
		**	Sets the rule's associativity mode.
		*/
		void setAssoc (int value)
		{
			this->assoc = value;
		}

		/**
		**	Sets the rule's action string.
		*/
		void setAction (LString *action)
		{
			if (this->action != nullptr)
				this->action->free();

			this->action = action;
		}

		/**
		**	Adds an element to the rule's element list.
		*/
		void addElem (Token *elem)
		{
			if (elem->getType() == TTYPE_NVALUE)
			{
				this->elems->bottom->value->setNValue (elem);
				return;
			}

			this->elems->push (elem);
			this->length++;
		}

		/**
		**	Returns an element at the specified index.
		*/
		Token *getElem (int index)
		{
			Linkable<Token*> *elem = this->elems->top;

			while (elem && index--)
				elem = elem->next;

			return elem ? elem->value : nullptr;
		}
	};
};
