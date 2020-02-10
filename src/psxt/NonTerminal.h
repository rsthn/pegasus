/*
**	psxt::NonTerminal
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
	using asr::utils::List;

	/**
	**	Describes a non-terminal definition, that is a named set with one or more production rules.
	*/

	class NonTerminal
	{
		protected:

		/**
		**	ID of the non-terminal.
		*/
		int id;

		/**
		**	Name of this non terminal.
		*/
		LString *name;

		/**
		**	Return type of the non-terminal.
		*/
		LString *returnType;

		/**
		**	List of production rules.
		*/
		List<ProductionRule*> *rules;

		/**
		**	Next available production rule ID.
		*/
		int availableId;

		public:

		/**
		**	Initializes the non-terminal definition.
		*/
		NonTerminal (int id, LString *name)
		{
			this->id = id;
			this->rules = new List<ProductionRule*> ();

			this->name = name;
			this->availableId = 1;

			this->returnType = nullptr;
		}

		/**
		**	Destructor.
		*/
		virtual ~NonTerminal ()
		{
			delete this->rules->clear();
			this->name->free();

			if (this->returnType != nullptr)
				this->returnType->free();
		}

		/**
		**	Returns the ID of the non-terminal.
		*/
		int getId ()
		{
			return this->id;
		}

		/**
		**	Returns the rule's name.
		*/
		LString *getName ()
		{
			return this->name;
		}

		/**
		**	Performs a comparison by reference (0) or by name (1).
		*/
		int compare (NonTerminal *value)
		{
			return this - value;
		}
		
		int compare (LString *value)
		{
			return name->equals(value) ? 0 : 1;
		}

		/**
		**	Returns the return-type of the non-terminal.
		*/
		NonTerminal *setReturnType (LString *value)
		{
			if (this->returnType != nullptr)
				this->returnType->free();

			this->returnType = value;
			return this;
		}

		/**
		**	Returns the return-type of the non-terminal.
		*/
		LString *getReturnType ()
		{
			return this->returnType;
		}

		/**
		**	Returns the rule's production rules.
		*/
		List<ProductionRule*> *getRules ()
		{
			return this->rules;
		}

		/**
		**	Returns the next ID available for a production rule.
		*/
		int nextId ()
		{
			return availableId++;
		}

		/**
		**	Adds a production rule to the grammar rule.
		*/
		void addRule (ProductionRule *rule)
		{
			if (this->rules->contains (rule))
			{
				delete rule;
				return;
			}

			this->rules->push (rule);
		}
	};
};
