/*
**	psxt::LString
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
	using asr::utils::LList;
	using asr::utils::Linkable;
int count0=0, count1=0, count2=0, count3=0;
	/**
	**	Implementation of a pooLed string, that is, a string whose value can be found uniquely in a pool. This allows
	**	to make a better use of memory since there are no duplicate strings, and also improve comparison speed because
	**	strings are now compared by their memory addresses.
	*/
	class LString : public traits::no_delete
	{
		protected:

		/**
		**	String pool.
		*/
		static LList<LString*> *pool;

		/**
		**	The actual underlying string, only accessible to the caller by using getString().
		*/
		String *string;

		/**
		**	Node of the LString within the pool.
		*/
		Linkable<LString*> *node;

		/**
		**	Number of active references to this string.
		*/
		int refCount;

		/**
		**	Hash code of the string.
		*/
		uint32_t hash;

		/**
		**	Internal constructor.
		*/
		LString()
		{
			this->string = nullptr;
			this->node = nullptr;

			this->refCount = 1;
		}

		/**
		**	Internal destructor.
		*/
		private: virtual ~LString()
		{
			if (this->string != nullptr)
				delete this->string;

			LString::remove(this);
		}

		/**
		**	Searches the pool for a string match.
		*/
		protected: static LString *get (const char *value)
		{
			return pool != nullptr ? pool->get(value, String::getHash(value)) : nullptr;
		}

		/**
		**	Registers a string on the pool.
		*/
		static LString *put (LString *value)
		{
			if (pool == nullptr)
				pool = new LList<LString*> ();

			pool->push(value);
			value->node = pool->bottom;

			return value;
		}

		/**
		**	Removes a string from the pool.
		*/
		static void remove (LString *value)
		{
			if (pool == nullptr || value->node == nullptr)
				return;

			pool->remove(value->node);
		}

		public:

		/**
		**	Length of the string.
		*/
		int length;

		/**
		**	Removes all strings from the pool.
		*/
		static void finish ()
		{
			if (pool != nullptr)
			{
				for (LString *i = pool->pop(); i; i = pool->pop())
				{
					i->node = nullptr;
					delete i;
				}

				delete pool;
			}

			printf("Count0: %u\n", count0);
			printf("Count1: %u\n", count1);
			printf("Count2: %u\n", count2);
			printf("Count3: %u\n", count3);
		}

		/**
		**	Allocates a new string with the provided value.
		*/
		static LString *alloc (const char *value)
		{
			LString *s = LString::get(value);
			if (s != nullptr)
			{
				s->refCount++;
				return s;
			}

			s = new LString();

			s->string = new String (value);
			s->length = s->string->length;
			s->hash = s->string->getHash();

			return LString::put(s);
		}

		/**
		**	Allocates a string from another string.
		*/
		static LString *alloc (LString *value)
		{
			value->refCount++;
			return value;
		}

		/**
		**	Releases the string reference.
		*/
		void free ()
		{
			if (--refCount == 0)
				delete this;
		}

		/**
		**	Closes the string.
		*/
		LString *clone()
		{
			return LString::alloc(this);
		}

		/**
		**	Returns the underlying string object.
		*/
		String *getString()
		{
			return this->string;
		}

		/**
		**	Returns the buffer of the string.
		*/
		const char *getValue()
		{
			return this->string->value;
		}

		/**
		**	Returns a zero-terminated string representing the string object.
		*/
		const char *c_str()
		{
			return this->string->c_str();
		}

		/**
		**	Returns a single character at the specified index.
		*/
		int charAt (int index)
		{
			return this->string->charAt(index);
		}

		/**
		**	Compares the string with the specified value (full comparison).
		*/
		bool cequals (const char *value, int length=-1)
		{
			if (length == -1) length = strlen(value);

			if (this->length != length)
				return false;

			return !memcmp (this->string->value, value, length);
		}

		/**
		**	Returns the hash code of the string.
		*/
		uint32_t getHash ()
		{
			return this->hash;
		}

		/**
		**	Compares if the string objects are the same (reference comparison).
		*/
		bool equals (LString *value)
		{
			return this == value;
		}

		/**
		**	Compares strings by reference.
		*/
		int compare (const char *value)
		{
			return cequals(value) ? 0 : 1;
		}

		int compare (LString *value)
		{
			return this - value;
		}
	};

	/**
	**	String pool.
	*/
	LList<LString*> *LString::pool = nullptr;

};
