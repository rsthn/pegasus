
namespace psxt
{
	class ItemSet;
	class Item;


	/**
	**	An item is an augmented production rule, that is a production rule with an indicator to specify
	**	which is the element of the production rule that is currently in focus.
	*/

	class Item
	{
		protected:

		/**
		**	Related production rule.
		*/
		ProductionRule *rule;

		/**
		**	Element node currently in focus.
		*/
		Linkable<Token*> *elem;

		/**
		**	Overriden focus element.
		*/
		Token *celem;

		/**
		**	Transition itemset.
		*/
		ItemSet *transition;

		/**
		**	Index of the element in focus.
		*/
		int index;

		/**
		**	Hash code of the item set.
		*/
		uint32_t hash;

		public:

		/**
		**	Initializes the item and moves the indicator to the first element of the production rule.
		*/
		Item (ProductionRule *rule)
		{
			this->set (rule);
		}

		/**
		**	Constructs the item as a clone of the given item.
		*/
		Item (Item *item)
		{
			this->rule = item->rule;
			this->elem = item->elem;
			this->index = item->index;

			this->celem = nullptr;
			this->transition = nullptr;

			this->updateHash();
		}

		/**
		**	Destructor.
		*/
		virtual ~Item ()
		{
			if (celem) delete celem;
		}

		/**
		**	Initializes the item from a given production rule.
		*/
		void set (ProductionRule *rule)
		{
			this->rule = rule;
			this->elem = rule->getElems()->head();
			this->index = 0;

			this->celem = nullptr;
			this->transition = nullptr;

			this->updateHash();
		}

		/**
		**	Updates the hash code of the item.
		*/
		private: void updateHash ()
		{
			hash = this->rule->getNonTerminal()->getId()*1024*1024;
			hash += this->rule->getId()*1024;
			hash += this->index;
		}

		/**
		**	Overrides the focus element.
		*/
		public: void overrideElem (LString *value)
		{
			if (celem) delete celem;

			celem = new Token (elem != nullptr ? elem->value : nullptr, value);
		}

		/**
		**	Returns the hash code of the item.
		*/
		uint32_t getHash()
		{
			return this->hash;
		}

		/**
		 * @brief Returns the transition itemset.
		 * @return ItemSet* 
		 */
		ItemSet *getTransition() {
			return this->transition;
		}

		/**
		 * @brief Sets the transition itemset.
		 * @param transition 
		 */
		void setTransition (ItemSet *transition) {
			this->transition = transition;
		}

		/**
		 * @brief Returns the related production rule.
		 * @return ProductionRule* 
		 */
		ProductionRule *getRule() {
			return rule;
		}

		/**
		 * @brief Returns the element in focus.
		 * @return Token* 
		 */
		Token *getElem() {
			return celem != nullptr ? celem : (elem != nullptr ? elem->value : nullptr);
		}

		/**
		 * @brief Returns the node of the element in focus.
		 * @return Linkable<Token*>* 
		 */
		Linkable<Token*> *getNode() {
			return celem != nullptr ? nullptr : elem;
		}

		/**
		 * @brief Returns the index of the element in focus.
		 * @return int 
		 */
		int getIndex() {
			return index;
		}

		/**
		 * @brief Rewires the item such that all follows and the element currently wired to oldv will now be wired to newv.
		 * @param oldItemSet 
		 * @param newItemSet
		 */
		void rewire (ItemSet *oldItemSet, ItemSet *newItemSet)
		{
			if (this->transition == oldItemSet)
				this->transition = newItemSet;
		}

		/**
		**	Compares the items and returns true if they're equal.
		*/
		bool equals (Item *item)
		{
			if (!item) return false;

			if (item == this)
				return true;

			if (item->rule != this->rule)
				return false;

			if (item->index != this->index)
				return false;

			if (item->getElem() == this->getElem())
				return true;

			if (item->getElem() == nullptr || this->getElem() == nullptr)
				return false;

			return item->getElem()->equals (this->getElem(), true);
		}

		int compare (Item *item) {
			return equals(item) ? 0 : 1;
		}

		/**
		 * @brief Moves the focus to the next element in the production rule.
		 * @return Item* 
		 */
		Item *moveNext()
		{
			if (celem) {
				delete celem;
				celem = nullptr;
			}

			if (elem == nullptr)
				return this;

			elem = elem->next();
			index++;

			updateHash();
			return this;
		}

		/**
		 * @brief Dumps the item to the specified stream.
		 * @param output 
		 */
		void dump (FILE *output)
		{
			int j = 0;

			for (Linkable<Token*> *i = this->rule->getElems()->head(); i; i = i->next())
			{
				if (index == j++)
				{
					fprintf(output, "*");

					if (celem != nullptr) {
						fprintf(output, "%s ", celem->getCstr());
						continue;
					}
				}

				fprintf(output, "%s%s%s ", i->value->getType() == Token::Type::END ? "ε" : i->value->getCstr(),
					i->value->getNValue() != nullptr ? ":" : "",
					i->value->getNValue() != nullptr ? i->value->getNValue()->getCstr() : ""
				);
			}

			if (index >= rule->getElems()->length())
				fprintf(output, "* |> %s#%u", rule->getNonTerminal()->getName()->c_str(), rule->getId());
		}
	};
};
