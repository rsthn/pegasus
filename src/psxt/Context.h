/*
**	psxt::Context
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

#include <asr/utils/Pair>

#include <psxt/ProductionRule.h>
#include <psxt/NonTerminal.h>

namespace psxt
{
	using asr::utils::Pair;
	using asr::utils::LList;

	class ReachSet;

	/**
	**	Sets of non-terminals.
	*/
	#define SECTION_NULL		-1
	#define	SECTION_LEXICON		0
	#define	SECTION_GRAMMAR		1
	#define SECTION_ARRAYS		2
	#define	SECTION_COUNT		3

	/**
	**	Describes the global source file context. That is all the grammar and lexicon in the syntax file.
	*/

	class Context
	{
		protected:

		/**
		**	Sections for non-terminals.
		*/
		LList<Pair<LString*, NonTerminal*>*> *sections[SECTION_COUNT];

		/**
		**	Exported symbols on each section.
		*/
		LList<Pair<LString*, ProductionRule*>*> *exports[SECTION_COUNT];

		/**
		**	Next available ID for a nonterminal in a section.
		*/
		int availableId[SECTION_COUNT];

		/**
		**	Map with the global reach-sets.
		*/
		LList<Pair<LString*, ReachSet*>*> *reachSets;
		LList<Pair<LString*, ReachSet*>*> *reachSets_deletable;

		public:

		/**
		**	Initializes the context.
		*/
		Context ()
		{
			reachSets = new LList<Pair<LString*, ReachSet*>*> ();
			reachSets_deletable = new LList<Pair<LString*, ReachSet*>*> ();

			for (int i = 0; i < SECTION_COUNT; i++)
			{
				sections[i] = new LList<Pair<LString*, NonTerminal*>*> ();
				exports[i] = new LList<Pair<LString*, ProductionRule*>*> ();

				availableId[i] = 1;
			}
		}

		/**
		**	Destructor.
		*/
		virtual ~Context ()
		{
			for (int j = 0; j < SECTION_COUNT; j++)
			{
				for (Linkable<Pair<LString*, NonTerminal*>*> *i = sections[j]->head(); i; i = i->next())
				{
					i->value->key->free();
					delete i->value;
				}

				for (Linkable<Pair<LString*, ProductionRule*>*> *i = exports[j]->head(); i; i = i->next())
				{
					delete i->value->reset();
				}

				delete exports[j]->reset();
				delete sections[j]->reset();
			}

			/* ** */
			for (Linkable<Pair<LString*, ReachSet*>*> *i = reachSets_deletable->head(); i; i = i->next())
			{
				i->value->key->free();
				delete i->value;
			}

			for (Linkable<Pair<LString*, ReachSet*>*> *i = reachSets->head(); i; i = i->next())
			{
				i->value->key->free();
				i->value->value = nullptr;
				delete i->value;
			}

			delete reachSets_deletable->reset();
			delete reachSets->reset();
		}

		/**
		**	Returns the next available ID for a non-terminal on the specified section.
		*/
		int nextId (int section)
		{
			if (section < 0 || section >= SECTION_COUNT)
				return 0;

			return availableId[section]++;
		}

		/**
		**	Returns the definition of non-terminals in a section.
		*/
		List<NonTerminal*> *getNonTerminals (int section)
		{
			if (section < 0 || section >= SECTION_COUNT)
				return nullptr;

			List<NonTerminal*> *list = new List<NonTerminal*> ();

			for (Linkable<Pair<LString*, NonTerminal*>*> *node = sections[section]->head(); node != nullptr; node = node->next())
				list->push (node->value->value);

			return list;
		}

		/**
		**	Returns the definition of non-terminals in a section as a list of pairs.
		*/
		LList<Pair<LString*, NonTerminal*>*> *getNonTerminalPairs (int section)
		{
			if (section < 0 || section >= SECTION_COUNT)
				return nullptr;

			return sections[section];
		}

		/**
		**	Returns the reach sets list.
		*/
		LList<Pair<LString*, ReachSet*>*> *getReachSets ()
		{
			return this->reachSets;
		}

		/**
		**	Returns the reach sets list.
		*/
		ReachSet *getReachSet (LString *name)
		{
			Pair<LString*, ReachSet*> *value = this->reachSets->get (name, name->getHash());
			return value ? value->value : nullptr;
		}

		/**
		**	Adds an exported production rule to the section's export list.
		*/
		ReachSet *addReachSet (LString *name, ReachSet *set, bool is_deletable)
		{
			if (is_deletable)
				this->reachSets_deletable->push (new Pair<LString*, ReachSet*> (name, set));

			this->reachSets->push (new Pair<LString*, ReachSet*> (name, set));
			return set;
		}

		/**
		**	Returns the list of exports on a section.
		*/
		LList<Pair<LString*, ProductionRule*>*> *getExports (int section)
		{
			if (section < 0 || section >= SECTION_COUNT)
				return nullptr;

			return exports[section];
		}

		/**
		**	Adds an exported production rule to the section's export list.
		*/
		ProductionRule *addExport (int section, LString *name, ProductionRule *rule)
		{
			if (section < 0 || section >= SECTION_COUNT)
				return nullptr;

			exports[section]->push (new Pair<LString*, ProductionRule*> (name, rule));
			return rule;
		}

		/**
		**	Adds a non-terminal to a section.
		*/
		NonTerminal *addNonTerminal (int section, NonTerminal *nonterm)
		{
			if (section < 0 || section >= SECTION_COUNT)
				return nullptr;

			NonTerminal *tmp = getNonTerminal (section, nonterm->getName());
			if (tmp != nullptr)
			{
				delete nonterm;
				return tmp;
			}

			sections[section]->push (new Pair<LString*, NonTerminal*> (nonterm->getName()->clone(), nonterm));
			return nonterm;
		}

		/**
		**	Searches and returns a non-terminal in a section given its name.
		*/
		NonTerminal *getNonTerminal (int section, LString *name)
		{
			if (section < 0 || section >= SECTION_COUNT)
				return nullptr;

			Pair<LString*, NonTerminal*> *pair = sections[section]->get (name, name->getHash());
			return pair == nullptr ? nullptr : pair->value;
		}

		/**
		**	Returns a non-terminal in a section given its ID.
		*/
		NonTerminal *getNonTerminal (int section, int id)
		{
			if (section < 0 || section >= SECTION_COUNT)
				return nullptr;

			Linkable<Pair<LString*, NonTerminal*>*> *i = sections[section]->head();

			while (i && i->value->value->getId() != id)
				i = i->next();

			return i == nullptr ? nullptr : i->value->value;
		}
	};
};
