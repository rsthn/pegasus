/*
**	psxt::Token
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
	using asr::utils::Linkable;

	class NonTerminal;

	/**
	**	Describes a token, that is a string obtained from a source file that belongs to a certain group which gives it meaning.
	*/

	class Token
	{
		protected:

		/**
		**	String value of the token.
		*/
		LString *value;

		/**
		**	Type of the token.
		*/
		int type;

		/**
		**	Source of the token.
		*/
		String *source;

		/**
		**	Linked NVALUE of the element (if any).
		*/
		Token *nvalue;

		/**
		**	Non-Terminal related to this token.
		*/
		NonTerminal *nonterm;

		/**
		**	Line number and column number where the token appears in the source file.
		*/
		unsigned line, col;

		public:

		/**
		**	Initializes the token object with the specified parameters.
		*/
		Token (String *source, const char *value="", int type=0, int line=0, int col=0)
		{
			this->value = LString::alloc(value);
			this->type = type;

			this->source = source;
			this->line = line;
			this->col = col;

			this->nvalue = nullptr;
			this->nonterm = nullptr;
		}

		/**
		**	Constructs a new token as a copy of the specified token but with a different value. If the value is not
		**	specified the value of the input token will be used.
		*/
		Token (Token *token, LString *value=nullptr, bool alloc=true)
		{
			this->value = value != nullptr ? (alloc ? LString::alloc(value) : value) : LString::alloc(token->value);
			this->type = token->type;

			this->source = token->source;
			this->line = token->line;
			this->col = token->col;

			this->nvalue = nullptr;
			this->nonterm = nullptr;
		}

		/**
		**	Destroys the token instance.
		*/
		virtual ~Token ()
		{
			this->value->free();

			if (nvalue != nullptr)
				delete nvalue;
		}

		/**
		**	Compares two tokens and returns true if both are equal.
		*/
		bool equals (Token *item)
		{
			if (item == nullptr || item->type != type)
				return false;

			return item->value->equals (value);
		}

		/**
		**	Compares two tokens (and optionally the nvalues) and returns true if both are equal.
		*/
		bool equals (Token *item, bool nvalues)
		{
			if (item == this) return true;

			if (item == nullptr || item->type != type)
				return false;

			if (!item->value->equals(value))
				return false;

			if (!nvalues)
				return true;

			return (this->nvalue == nullptr && item->nvalue == nullptr)
				|| (this->nvalue != nullptr && item->nvalue != nullptr && this->nvalue->equals (item->nvalue));
		}

		/**
		**	Returns the hash code of the string.
		*/
		uint32_t getHash ()
		{
			return this->value->getHash() - (this->nvalue != nullptr ? this->nvalue->getHash() : 0);
		}

		/**
		**	Performs a comparison by a known key of the object. Returns true if both keys are equal.
		*/
		int compare (Token *item)
		{
			return equals ((Token *)item, true) ? 0 : 1;
		}

		/**
		**	Returns the string value of the token.
		*/
		LString *getValue ()
		{
			return this->value;
		}

		/**
		**	Returns the value of the token as a character.
		*/
		int getChar ()
		{
			return this->value->getValue()[0];
		}

		/**
		**	Returns a C-style string of the token's value.
		*/
		const char *getCstr ()
		{
			return this->value->c_str();
		}

		/**
		**	Sets the type of the token.
		*/
		Token *setType (int type)
		{
			this->type = type;
			return this;
		}

		/**
		**	Returns the token type.
		*/
		int getType ()
		{
			return this->type;
		}

		/**
		**	Sets the NValue of the token.
		*/
		Token *setNValue (Token *nvalue)
		{
			this->nvalue = nvalue;
			return this;
		}

		/**
		**	Returns the NValue of the token (if any).
		*/
		Token *getNValue ()
		{
			return this->nvalue;
		}

		/**
		**	Sets the non-terminal of the token.
		*/
		Token *setNonTerminal (NonTerminal *nonterm)
		{
			this->nonterm = nonterm;
			return this;
		}

		/**
		**	Returns the nonterminal of the token.
		*/
		NonTerminal *getNonTerminal ()
		{
			return this->nonterm;
		}

		/**
		**	Returns the source of the token.
		*/
		String *getSource ()
		{
			return this->source;
		}

		/**
		**	Returns the column number where the token appears in the source file.
		*/
		int getCol ()
		{
			return this->col;
		}

		/**
		**	Returns the line number where the token appears in the source file.
		*/
		int getLine ()
		{
			return this->line;
		}

		/**
		**	Sets the attributes of the token.
		*/
		void set (const char *value, int type, int line, int col)
		{
			if (this->value != nullptr)
				this->value->free();

			this->value = LString::alloc(value);
			this->type = type;

			this->line = line;
			this->col = col;
		}
	};
};
