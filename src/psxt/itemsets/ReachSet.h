/*
**	psxt::ReachSet
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
	class ReachSetNode;
	class ReachElement;
	class ReachPath;
	class ReachSet;

	/**
	**	Base abstract class for nodes related to a reach-sets.
	*/
	class ReachSetNode
	{
		protected:

		/**
		**	Number of references.
		*/
		public: int refCount;

		public:

		/**
		**	Type of the node. One of the constants on this class.
		*/
		int nodeType;

		/**
		**	Types of follow set nodes.
		*/
		static const int REACH_SET = 0;
		static const int REACH_PATH = 1;
		static const int REACH_ELEMENT = 2;

		/**
		**	Constructs an empty node.
		*/
		ReachSetNode (int nodeType)
		{
			this->nodeType = nodeType;
			this->refCount = 0;
count0++;
		}

		/**
		**	Destructs the follow set node.
		*/
		protected: virtual ~ReachSetNode()
		{
count0--;
		}

		/**
		**	Deletes the object if there are no more references.
		*/
		public: void destroy()
		{
			if (--this->refCount == 0)
				delete this;
		}

		/**
		**	Deletes the object if there are no more references.
		*/
		ReachSetNode *getReference()
		{
			this->refCount++;
			return this;
		}

		/**
		**	Returns the type of the node.
		*/
		int getNodeType ()
		{
			return this->nodeType;
		}

		/**
		**	Dumps the node to stdout.
		*/
		virtual void dump (FILE *os) = 0;
	};
int xxx=0;
	/**
	**	Describes a single element (token) that can be reached through a path.
	*/
	class ReachElement : public ReachSetNode
	{
		protected:

		/**
		**	Value of the element. If nullptr, it is treated as an exit node.
		*/
		Token *value;

		public:

		/**
		**	Constructs a reach element node with the specified token.
		*/
		ReachElement (Token *value) : ReachSetNode(ReachSetNode::REACH_ELEMENT)
		{
			this->value = value;
printf("%*s+ReachElement %x => %d\n", xxx*2, "", this, count1+1);
count1++;
		}

		protected: virtual ~ReachElement()
		{
printf("%*s~ReachElement %x => %d\n", xxx*2, "", this, count1-1);
count1--;
		}

		/**
		**	Returns a boolean indicating if the element is an exit-node.
		*/
		public: bool isExit()
		{
			return this->value == nullptr;
		}

		/**
		**	Returns the value of the element.
		*/
		Token *getValue()
		{
			return this->value;
		}

		/**
		**	Dumps the node to stdout.
		*/
		virtual void dump (FILE *os)
		{
			if (this->value != nullptr)
			{
				fprintf (os, "%s%s%s", value->getType() == TTYPE_EOF ? "EOF" : value->getCstr(), value->getNValue() != nullptr ? ":" : "", value->getNValue() != nullptr ? value->getNValue()->getCstr() : "");
			}
			else
				fprintf (os, "@");
		}
	};

	/**
	**	Describes a single path composed of elements.
	*/
	class ReachPath : public ReachSetNode
	{
		protected:

		/**
		**	List of elements in the path.
		*/
		List<ReachSetNode*> *list;

		public:

		/**
		**	Constructs an empty reach path.
		*/
		ReachPath() : ReachSetNode(ReachSetNode::REACH_PATH)
		{
			this->list = new List<ReachSetNode*> ();
printf("%*s+ReachPath %x => %d\n", xxx*2, "", this, count2+1);
count2++;
		}

		/**
		**	Destructs the reach path.
		*/
		protected: virtual ~ReachPath();

		/**
		**	Adds an item to the path.
		*/
		public: void addItem (ReachSetNode *elem)
		{
			this->list->push (elem->getReference());
printf("%*sReachPath[%x]: ADD %x\n", xxx*2, "", this, elem);
		}

		/**
		**	Builds a reach path for a specified production rule given its first element node.
		**
		**	WARNING: Elements must be bound to their respective non-terminals (ItemSetBuilder::bindTokenNTRefs).
		*/
		static ReachPath *buildFromRule (Linkable<Token*> *elem, Context *context);

		/**
		**	Dumps the node to stdout.
		*/
		virtual void dump (FILE *os);
	};

	/**
	**	Describes a set of one or more reachable paths.
	*/
	class ReachSet : public ReachSetNode
	{
		protected:

		/**
		**	List of reachable paths.
		*/
		List<ReachPath*> *list;

		/**
		**	Non-Terminal used to build the set.
		*/
		NonTerminal *nonterm;

		/**
		**	ItemSet used to build the set.
		*/
		ItemSet *itemset;

		public:

		/**
		**	Constructs an empty reach set.
		*/
		ReachSet() : ReachSetNode(ReachSetNode::REACH_SET)
		{
			this->list = new List<ReachPath*> ();

			this->nonterm = nullptr;
			this->itemset = nullptr;
printf("%*s+ReachSet %x => %d\n", xxx*2, "", this, count3+1);
count3++;
		}

		/**
		**	Destructs the reach set.
		*/
		protected: virtual ~ReachSet()
		{
printf("%*s~ReachSet %x => %d\n", xxx*2, "", this, count3-1);
			delete this->list;
count3--;
		}

		/**
		**	Returns the non-terminal used to generate the set.
		*/
		public: NonTerminal *getNonTerminal ()
		{
			return this->nonterm;
		}

		/**
		**	Returns the itemset used to generate the set.
		*/
		ItemSet *getItemSet ()
		{
			return this->itemset;
		}

		/**
		**	Adds a path to the set.
		*/
		void addPath (ReachPath *path)
		{
			this->list->push ((ReachPath *)path->getReference());
printf("%*sReachSet[%x]: ADD %x\n", xxx*2, "", this, path);
		}

		/**
		**	Builds a reach set for a specified non-terminal. If a context is provided the non-terminal will be first searched
		**	on the context's global reach sets, if found it will be returned, otherwise it will be built.
		**
		**	WARNING: All elements in rules must be bound to their respective non-terminals (ItemSetBuilder::bindTokenNTRefs).
		*/
		static ReachSet *buildFromNonTerminal (NonTerminal *nonterm, Context *context=nullptr)
		{
			ReachSet *set = context != nullptr ? context->getReachSet (nonterm->getName()) : nullptr;
			if (set != nullptr) return set;
xxx++;
			set = new ReachSet();
			set->nonterm = nonterm;

			if (context != nullptr)
				context->addReachSet (nonterm->getName(), set, false);

			for (Linkable<ProductionRule*> *i = nonterm->getRules()->top; i; i = i->next)
			{
				set->addPath (ReachPath::buildFromRule (i->value->getElems()->top, context));
			}
//printf("---------\nReach(%s) [NTERM]:\n", nonterm->getName()->c_str());
//set->dump(stdout);
//printf("\n\n");
xxx--;
			return set;
		}

		/**
		**	Builds a reach set for a specified itemset. If a context is provided the itemset signature will be first searched
		**	on the context's global reach sets, if found it will be returned, otherwise it will be built.
		**
		**	WARNING: All elements in rules must be bound to their respective non-terminals (ItemSetBuilder::bindTokenNTRefs).
		*/
		static ReachSet *buildFromItemSet (ItemSet *itemset, int offset=0, Context *context=nullptr)
		{
			ReachSet *set = context != nullptr ? context->getReachSet (itemset->getSignature()) : nullptr;
			if (set != nullptr) return set;
xxx++;
			set = new ReachSet ();
			set->itemset = itemset;

			if (context != nullptr)
				context->addReachSet (itemset->getSignature(), set, true);

			for (Linkable<Item*> *i = itemset->getItems()->top; i; i = i->next)
			{
				Linkable<Token*> *elem = i->value->getNode();

				for (int j = 0; j < offset && elem != nullptr; j++)
					elem = elem->next;

				if (elem != nullptr)
					set->addPath (ReachPath::buildFromRule (elem, context));
				else
					set->addPath (new ReachPath ());
			}
printf("---------\nReach(%s) [SET]:\n", itemset->getSignature()->c_str());
set->dump(stdout);
printf("\n\n");
xxx--;
			return set;
		}

		/**
		**	Dumps the node to stdout.
		*/
		virtual void dump (FILE *os)
		{
			fprintf (os, "{");

			for (Linkable<ReachPath*> *i = this->list->top; i; i = i->next)
			{
				i->value->dump(os);

				if (i->next != nullptr)
					fprintf (os, ", ");
			}

			fprintf (os, "}");
		}
	};

	/**
	**	Builds a reach path for a specified production rule given its first element node.
	**
	**	WARNING: All elements must be bound to their respective non-terminals (ItemSetBuilder::bindTokenNTRefs).
	*/
	ReachPath *ReachPath::buildFromRule (Linkable<Token*> *elem, Context *context)
	{
xxx++;
		ReachPath *path = new ReachPath ();

		for (Linkable<Token*> *i = elem; i; i = i->next)
		{
			if (i->value->getNonTerminal() != nullptr)
				path->addItem (ReachSet::buildFromNonTerminal(i->value->getNonTerminal(), context));
			else
				path->addItem (new ReachElement (i->value));
		}
xxx--;
		return path;
	}

	/**
	**	Dumps the node to stdout.
	*/
	void ReachPath::dump (FILE *os)
	{
		fprintf (os, "[");

		for (Linkable<ReachSetNode*> *i = this->list->top; i; i = i->next)
		{
			if (i->value->getNodeType() == ReachSetNode::REACH_SET && ((ReachSet *)i->value)->getNonTerminal() != nullptr)
				fprintf (os, "#%s", ((ReachSet *)i->value)->getNonTerminal()->getName()->c_str());
			else
				i->value->dump(os);

			if (i->next != nullptr)
				fprintf (os, ", ");
		}

		fprintf (os, "]");
	}

	/**
	**	Function wrapper to prevent class forward reference errors.
	*/
	ReachSet *ReachSet_buildFromItemSet (ItemSet *itemset, int offset, Context *context)
	{
		xxx++;
		auto x = (ReachSet *)ReachSet::buildFromItemSet (itemset, offset, context)->getReference();
		xxx--;
		return x;
	}
};

namespace asr {
namespace utils {

	template<>
	struct Destroyer<psxt::ReachSetNode*> {
		static inline void destroy (psxt::ReachSetNode*&item, bool _delete=true) { if (item != nullptr && _delete) item->destroy(); item = nullptr; }
		static inline void destroyb (psxt::ReachSetNode*item, bool _delete=true) { if (item != nullptr && _delete) item->destroy(); }
	};

	template<>
	struct Destroyer<psxt::ReachElement*> {
		static inline void destroy (psxt::ReachElement*&item, bool _delete=true) { if (item != nullptr && _delete) item->destroy(); item = nullptr; }
		static inline void destroyb (psxt::ReachElement*item, bool _delete=true) { if (item != nullptr && _delete) item->destroy(); }
	};

	template<>
	struct Destroyer<psxt::ReachPath*> {
		static inline void destroy (psxt::ReachPath*&item, bool _delete=true) { if (item != nullptr && _delete) item->destroy(); item = nullptr; }
		static inline void destroyb (psxt::ReachPath*item, bool _delete=true) { if (item != nullptr && _delete) item->destroy(); }
	};

	template<>
	struct Destroyer<psxt::ReachSet*> {
		static inline void destroy (psxt::ReachSet*&item, bool _delete=true) { if (item != nullptr && _delete) item->destroy(); item = nullptr; }
		static inline void destroyb (psxt::ReachSet*item, bool _delete=true) { if (item != nullptr && _delete) item->destroy(); }
	};

} }

namespace psxt
{
	using asr::utils::Destroyer;

	/**
	**	Destructs the reach path.
	*/
	ReachPath::~ReachPath()
	{
printf("%*s~ReachPath %x => %d\n", xxx*2, "", this, count2-1);
		for (Linkable<ReachSetNode*> *i = this->list->top; i; i = i->next)
		{
printf("#");
			/*if (i->value->nodeType == ReachSetNode::REACH_PATH)
				Destroyer<ReachPath*>::destroy((ReachPath*&)i->value);
			else if (i->value->nodeType == ReachSetNode::REACH_SET)
				Destroyer<ReachSet*>::destroy((ReachSet*&)i->value);
			else if (i->value->nodeType == ReachSetNode::REACH_ELEMENT)
				Destroyer<ReachElement*>::destroy((ReachElement*&)i->value);*/

			//i->value = nullptr;
		}

		delete this->list->clear();
count2--;
	}
}
