/*
**	psxt::Item
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
			this->elem = rule->getElems()->top;
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
		**	Returns the transition itemset.
		*/
		ItemSet *getTransition ()
		{
			return this->transition;
		}

		/**
		**	Sets the transition itemset.
		*/
		void setTransition (ItemSet *transition)
		{
			this->transition = transition;
		}

		/**
		**	Returns the related production rule.
		*/
		ProductionRule *getRule ()
		{
			return rule;
		}

		/**
		**	Returns the element in focus.
		*/
		Token *getElem ()
		{
			return celem != nullptr ? celem : (elem != nullptr ? elem->value : nullptr);
		}

		/**
		**	Returns the node of the element in focus.
		*/
		Linkable<Token*> *getNode ()
		{
			return celem != nullptr ? nullptr : elem;
		}

		/**
		**	Returns the index of the element in focus.
		*/
		int getIndex ()
		{
			return index;
		}

		/**
		**	Rewires the item such that all follows and the element currently wired to oldv will now be wired to newv.
		*/
		void rewire (ItemSet *oldv, ItemSet *newv)
		{
			if (this->transition == oldv)
				this->transition = newv;
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

		int compare (Item *item)
		{
			return equals(item) ? 0 : 1;
		}

		/**
		**	Moves the focus to the next element in the production rule.
		*/
		Item *moveNext ()
		{
			if (celem)
			{
				delete celem;
				celem = nullptr;
			}

			if (elem == nullptr) return this;

			elem = elem->next;
			index++;

			updateHash();

			return this;
		}

		/**
		**	Dumps the item to the specified stream.
		*/
		void dump (FILE *output, int (*getId) (ItemSet*))
		{
			int j = 0;

			for (Linkable<Token*> *i = this->rule->getElems()->top; i; i = i->next)
			{
				if (index == j++)
				{
					fprintf (output, "*");

					if (celem != nullptr)
					{
						fprintf (output, "%s ", celem->getCstr());
						continue;
					}
				}

				fprintf (output, "%s%s%s ", i->value->getType() == TTYPE_EOF ? "(EOF)" : i->value->getCstr(),
					i->value->getNValue() != nullptr ? ":" : "",
					i->value->getNValue() != nullptr ? i->value->getNValue()->getCstr() : ""
				);
			}

			if (index >= rule->getElems()->count)
			{
				fprintf (output, "* [%s#%u]", rule->getNonTerminal()->getName()->c_str(), rule->getId());
			}
		}
	};
};
