/*
**	psxt::ItemSet
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
	using asr::utils::Integer;
	using asr::utils::Linkable;
	using asr::utils::Triad;

	// Forward declarations.
	ReachSet *ReachSet_buildFromItemSet (ItemSet *itemset, int offset, Context *context);

	/**
	**	Describes an Item Set.
	*/

	class ItemSet
	{
		protected:

		/**
		**	ID of the item set.
		*/
		int id;

		/**
		**	List with one or more items.
		*/
		List<Item*> *list;

		/**
		**	All parents of the itemset. Usually should be only one, however when an itemset is recycled it
		**	may have several paths that lead to it.
		*/
		List<ItemSet*> *parents;

		/**
		**	Hash code of the item set. Set internally after the set is closed.
		*/
		uint32_t hash;

		/**
		**	Signature of the itemset. Set using buildSignature().
		*/
		LString *signature;

		/**
		**	Reach set of the item set. Generated after the itemset is closed.
		*/
		ReachSet *reachSet;

		public:

		/**
		**	Constructs an empty item set.
		*/
		ItemSet (int id=0)
		{
			this->id = id;
			this->signature = nullptr;
			this->reachSet = nullptr;

			this->list = new List<Item*> ();
			this->parents = new List<ItemSet*> ();
		}

		/**
		**	Destroys the item set.
		*/
		virtual ~ItemSet()
		{
			delete list->clear();
			delete parents->reset();

			if (this->signature != nullptr)
				this->signature->free();
		}

		/**
		**	Sets the ID of the item set.
		*/
		public: void setId (int id)
		{
			this->id = id;
		}

		/**
		**	Returns the ID of the item set.
		*/
		int getId()
		{
			return this->id;
		}

		/**
		**	Returns the signature of the item set.
		*/
		LString *getSignature()
		{
			return this->signature;
		}

		/**
		**	Returns the hash code of the item set.
		*/
		uint32_t getHash()
		{
			return this->hash;
		}

		/**
		**	Returns the reach set of the item set.
		*/
		ReachSet *getReachSet()
		{
			return this->reachSet;
		}

		/**
		**	Adds a parent to the itemset.
		*/
		void addParent (ItemSet *parent)
		{
			this->parents->push (parent);
		}

		/**
		**	Returns all parents of the itemset.
		*/
		List<ItemSet*> *getParents()
		{
			return this->parents;
		}

		/**
		**	Returns the item list.
		*/
		List<Item*> *getItems()
		{
			return this->list;
		}

		/**
		**	Compares the item sets and returns a boolean indicating if they're equal or not.
		*/
		bool equals (ItemSet *item)
		{
			if (item == nullptr) return false;

			if (item->list->count != this->list->count)
				return false;

			for (Linkable<Item*> *i = this->list->top; i; i = i->next)
			{
				Item *k = item->getItems()->get (i->value);
				if (k == nullptr) return false;

				// violet:COMPARE FOLLOW SET
			}

			return true;
		}

		/**
		**	Compares the key of the item set (ID). The parameter should be a casted int.
		*/
		int compare (int value)
		{
			return this->id - value;
		}

		int compare (ItemSet *item)
		{
			return equals(item) ? 0 : 1;
		}

		/**
		**	Returns true if the specified item is contained in the set.
		*/
		bool contains (Item *item)
		{
			return this->list->contains (item);
		}

		/**
		**	Structure that groups several items with a charset pattern. This is used to factorize an input charset
		**	for further processing.
		*/
		private: struct Group
		{
			List<Item*> *items;
			CharSet *charset;

			Group (CharSet *charset, Item *item = nullptr)
			{
				this->items = new List<Item*> ();
				this->charset = charset;

				if (item != nullptr)
					addItem (item);
			}

			~Group ()
			{
				delete this->items->reset();
				delete charset;
			}

			void addItem (Item *item)
			{
				if (items->contains (item))
					return;

				items->push (item);
			}

			void addItems (List<Item*> *list)
			{
				for (Linkable<Item*> *i = list->top; i; i = i->next)
				{
					if (!items->contains (i->value))
						items->push (i->value);
				}
			}

			int compare (CharSet *val)
			{
				return this->charset->equals ((CharSet *)val) ? 0 : 1;
			}
		};

		/**
		**	Detects common pattern strings in the specified input and introduces new rules if necessary.
		*/
		public: void factorize()
		{
			Linkable<Item*> *ni;

			List<List<Group*>*> *stack = new List<List<Group*>*> ();
			List<Group*> *list = new List<Group*> ();

			for (Linkable<Item*> *i = this->list->top; i; i = ni)
			{
				ni = i->next;

				if (i->value->getElem() == nullptr)
					continue;

				if (i->value->getElem()->getType() != TTYPE_SQSTRING && i->value->getElem()->getType() != TTYPE_DQSTRING)
					continue;

				list->push (new Group (new CharSet (i->value->getElem()->getValue()), i->value));
				this->list->remove (i);
			}

			stack->push (list);

			CharSet *temp = new CharSet();
			CharSet *accum = new CharSet();

			while (stack->bottom->value->count > 1)
			{
				List<Group*> *output = new List<Group*> ();

				list = stack->bottom->value;

				for (Linkable<Group*> *i = list->top; i; i = i->next)
				{
					for (Linkable<Group*> *j = i->next; j; j = j->next)
					{
						if (temp->set(i->value->charset)->_and(j->value->charset)->isEmpty())
							continue;

						Group *g = output->get(temp);
						if (g == nullptr)
						{
							g = new Group (new CharSet (temp));
							output->push (g);
						}

						g->addItems (i->value->items);
						g->addItems (j->value->items);
					}
				}

				stack->push (output);
			}

			accum->clear()->_not();

			while ((list = stack->pop()) != nullptr)
			{
				temp->clear();

				for (Linkable<Group*> *i = list->top; i; i = i->next)
				{
					if (i->value->charset->_and (accum)->isEmpty())
					{
						if (!stack->count)
							i->value->items->clear();

						continue;
					}

					LString *p = i->value->charset->toString();

					for (Linkable<Item*> *j = i->value->items->top; j; j = j->next)
					{
						Item *w = new Item (j->value);
						w->overrideElem (p);

						this->list->push (w);
					}

					if (!stack->count)
						i->value->items->clear();

					p->free();

					temp->_or (i->value->charset);
				}

				delete list->clear();

				accum->_and (temp->_not());
			}

			delete temp;
			delete accum;

			delete stack->clear();
		}

		/**
		**	Adds all rules of a non-terminal to the item set. Each production will be converted to an item
		**	before adding it to the itemset.
		*/
		void addNonTerminal (NonTerminal *nonterm)
		{
			if (!nonterm) return;

			Item *item = nullptr;

			for (Linkable<ProductionRule*> *i = nonterm->getRules()->top; i; i = i->next)
			{
				if (item == nullptr)
					item = new Item (i->value);
				else
					item->set (i->value);

				if (this->contains (item))
					continue;

				this->list->push (item);
				item = nullptr;
			}

			if (item != nullptr) delete item;
		}

		/**
		**	Adds an item to the itemset.
		*/
		bool addItem (Item *item)
		{
			if (item == nullptr) return false;

			if (this->contains (item))
				return false;

			this->list->push (item);
			return true;
		}

		/**
		**	Builds and sets the signature of the itemset.
		*/
		void buildSignature()
		{
			static char buffer[8192];

			List<Item*> *list = new List<Item*> ();

			if (this->list->top)
				list->push (this->list->top->value);

			for (Linkable<Item*> *i = this->list->top ? this->list->top->next : nullptr; i; i = i->next)
			{
				uint32_t h = i->value->getHash();
				bool added = false;

				for (Linkable<Item*> *j = list->top; j; j = j->next)
				{
					if (h < j->value->getHash())
					{
						list->insertBefore (j, i->value);
						added = true;
						break;
					}
				}

				if (!added) list->push (i->value);
			}

			char *buff = buffer;
			*buff = '\0';

			for (Linkable<Item*> *i = list->top; i; i = i->next)
			{
				sprintf(buff, "%s%x", i->prev != nullptr ? ":" : "", i->value->getHash());
				buff += strlen (buff);
			}

			delete list->reset();

			if (this->signature != nullptr)
				this->signature->free();

			this->signature = LString::alloc(buffer);
		}

		/**
		**	Closes the item set. A grammar context and a section is required in order to obtain any required
		**	non-terminal definition.
		*/
		void close (Context *context, int section)
		{
			buildSignature();

			for (Linkable<Item*> *i = this->list->top; i; i = i->next)
			{
				if (i->value->getElem() == nullptr)
					continue;

				if (i->value->getElem()->getType() != TTYPE_IDENTIFIER)
					continue;

				addNonTerminal (i->value->getElem()->getNonTerminal());
			}

			if (section == SECTION_LEXICON)
				this->factorize();

			hash = 0;

			for (Linkable<Item*> *i = this->list->top; i; i = i->next)
				hash += i->value->getHash();
		}

		// VIOLET
		void xxx (Context *context, int section)
		{
			if (section == SECTION_GRAMMAR)
				this->reachSet = ReachSet_buildFromItemSet (this, 0, context);

			if (section == SECTION_LEXICON)
			{
				printf ("--- BUILDING REACHSET FOR %u ---\n", id);
				this->reachSet = ReachSet_buildFromItemSet (this, 0, context);
			}
		}

		/**
		**	Rewires the items in the itemset such that all currently wired to oldv will now be wired to newv.
		*/
		void rewire (ItemSet *oldv, ItemSet *newv)
		{
			for (Linkable<Item*> *i = this->list->top; i; i = i->next)
			{
				i->value->rewire (oldv, newv);
			}
		}

		/**
		**	Returns the ID of an item set.
		*/
		static int __getId (ItemSet *val)
		{
			return val->getId();
		}

		/**
		**	Dumps the item set to the specified stream.
		*/
		void dump (FILE *output)
		{
			fprintf (output, "ITEMSET %u", id);

			for (Linkable<ItemSet*> *i = this->parents->top; i; i = i->next)
				fprintf (output, " [%u]", i->value->getId());

			for (Linkable<Item*> *i = this->list->top; i; i = i->next)
			{
				fprintf (output, "\n  > [%u] ", i->value->getTransition() ? i->value->getTransition()->getId() : 0);
				i->value->dump (output, &__getId);
			}

			fprintf (output, "\n\n");
		}
	};
};
