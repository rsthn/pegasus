/*
**	psxt::ItemSetBuilder
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

#include <psxt/CharSet.h>

#include <psxt/itemsets/Item.h>
#include <psxt/itemsets/ItemSet.h>
#include <psxt/itemsets/ReachSet.h>

namespace psxt
{	
	/**
	**	The responsibility of this class is to build the set of item sets given a grammar context.
	*/

	class ItemSetBuilder
	{
		protected:

		/**
		**	Prints an error message and returns the value of 1.
		*/
		static int errmsg (Token *token, int code, char *msg, ...)
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
		**	Binds all tokens that reference a Non-Terminal name to its respective Non-Terminal object.
		*/
		private: static void bindTokenNTRefs (Context *context, int section)
		{
			for (Linkable<Pair<LString*, NonTerminal*>*> *n = context->getNonTerminalPairs(section)->top; n; n = n->next)
			{
				for (Linkable<ProductionRule*> *r = n->value->b->getRules()->top; r; r = r->next)
				{
					for (Linkable<Token*> *t = r->value->getElems()->top; t; t = t->next)
					{
						if (t->value->getType() != TTYPE_IDENTIFIER)
							continue;

						NonTerminal *nonterm = context->getNonTerminal (section, t->value->getValue());
						if (nonterm != nullptr) t->value->setNonTerminal (nonterm);
					}
				}
			}
		}

		/**
		**	Builds a list of itemsets from a section in a grammar context. The start symbol name parameter indicates the name
		**	of the first non-terminal to process in the section.
		*/
		public: static LList<ItemSet*> *build (Context *context, int section, LString *startSymbolName)
		{
			NonTerminal *nonterm = context->getNonTerminal (section, startSymbolName);
			if (nonterm == nullptr) return nullptr;

			bindTokenNTRefs (context, section);

			int nextItemId = 1;

			LList<ItemSet*> *itemsets = new LList<ItemSet*> ();
			List<ItemSet*> *queue = new List<ItemSet*> ();
			List<ItemSet*> *tqueue = new List<ItemSet*> ();

			ItemSet *itemset;

			// Build item set 0.
			itemset = new ItemSet ();
			itemset->addNonTerminal (nonterm);
			itemset->close (context, section);

			queue->push (itemset);

			while (queue->count)
			{
				itemset = queue->shift();

				for (Linkable<Item*> *i = itemset->getItems()->top; i; i = i->next)
				{
					if (i->value->getTransition() != nullptr || i->value->getElem() == nullptr)
						continue;

					ItemSet *nextItemset = new ItemSet ();
					nextItemset->addParent (itemset);

					i->value->setTransition (nextItemset);
					nextItemset->addItem ((new Item (i->value))->moveNext());

					for (Linkable<Item*> *j = i->next; j; j = j->next)
					{
						if (j->value->getTransition() != nullptr || j->value->getElem() == nullptr)
							continue;

						if (!j->value->getElem()->equals (i->value->getElem(), true))
							continue;

						j->value->setTransition (nextItemset);
						nextItemset->addItem ((new Item (j->value))->moveNext());
					}

					nextItemset->close (context, section);
					tqueue->push (nextItemset);
				}

				ItemSet *temp = itemsets->get (itemset);
				if (temp != nullptr)
				{
					itemset->getParents()->top->value->rewire (itemset, temp);
					temp->addParent (itemset->getParents()->top->value);

					tqueue->clear ();
					delete itemset;

					continue;
				}

				itemsets->push (itemset);
				itemset->setId (nextItemId++);

				queue->append (tqueue);
				tqueue->reset ();
			}

			for (auto i = itemsets->top; i; i = i->next)
				i->value->xxx(context, section);

printf("LList: Items=%u, Buckets=%u, MaxFill=%u\n", itemsets->count, itemsets->numBuckets, itemsets->maxFill);
			delete queue->reset();
			delete tqueue->reset();

			return itemsets;
		}
	};
};
