/*
**	psxt::CharSet
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
	/**
	**	Describes a set of characters assuming that the super set is ASCII.
	*/

	class CharSet
	{
		protected:

		/**
		**	Character set buffer. This is a small buffer with 256 characters indicating which characters are included
		**	in this set (those set to one) and which are not (those set to zero).
		*/
		char *buff;

		public:

		/**
		**	Initializes the set (empty set).
		*/
		CharSet ()
		{
			this->buff = new char[256];
			this->clear();
		}

		/**
		**	Initializes the set with the specified string representation. Uses the set() method to initialize the object.
		*/
		CharSet (LString *str)
		{
			this->buff = new char[256];
			this->set (str);
		}

		/**
		**	Initializes the set with the data of the given one.
		*/
		CharSet (CharSet *set)
		{
			this->buff = new char[256];
			this->set (set);
		}

		/**
		**	Destroys the character set.
		*/
		virtual ~CharSet ()
		{
			delete this->buff;
		}

		/**
		**	Compares two sets and returns true if both are equal.
		*/
		bool equals (CharSet *item)
		{
			for (int i = 0; i < 256; i++)
				if (item->buff[i] != this->buff[i]) return false;

			return true;
		}

		/**
		**	Returns a boolean indicating if the charset is empty or not.
		*/
		bool isEmpty ()
		{
			for (int i = 0; i < 256; i++)
				if (buff[i]) return false;

			return true;
		}

		/**
		**	Returns the 256-byte buffer of the charset.
		*/
		char *getBuffer ()
		{
			return buff;
		}

		/**
		**	Clears the set and returns it.
		*/
		CharSet *clear ()
		{
			for (int i = 0; i < 256; i++)
				buff[i] = 0;

			return this;
		}

		/**
		**	Copies the specified set into the current one.
		*/
		CharSet *set (CharSet *set)
		{
			for (int i = 0; i < 256; i++)
				buff[i] = set->buff[i];

			return this;
		}

		/**
		**	Appends a formatted character to a string.
		*/
		int __bihex2val (const char *s)
		{
			int a = s[0];
			int b = s[1];

			if ((a >= 'a' && a <= 'f') || (a >= 'A' && a <= 'F'))
				a = (a & 0xDF) - 'A' + 0x0A;
			else
				a = a - '0';

			if ((b >= 'a' && b <= 'f') || (b >= 'A' && b <= 'F'))
				b = (b & 0xDF) - 'A' + 0x0A;
			else
				b = b - '0';

			return a*16+b;
		}

		/**
		**	Expands the specified string pattern into the set. The input string can be formed by using sequences of either the
		**	characters to be included in the set (i.e. [hello]) or a trigraph with the minus sign in the middle to specify a
		**	range of characters (i.e. [A-F]). If the string starts with "^" a pattern negation will be assumed.
		*/
		CharSet *set (LString *str)
		{
			const char *input = str->c_str();
			int tmp0, tmp1, range = 0;
			char val = 1;

			clear();

			if (!*input) return this;

			if (str->cequals ("[:any:]"))
				return _not();

			if (*input != '[' || (*input == '[' && input[1] == '\0'))
			{
				if (*input == '\\')
				{
					switch (*++input)
					{
						case 'n':	tmp1 = '\n'; break;
						case 'r':	tmp1 = '\r'; break;
						case 't':	tmp1 = '\t'; break;
						case 'b':	tmp1 = '\b'; break;
						case 'v':	tmp1 = '\v'; break;
						case 'f':	tmp1 = '\f'; break;
						case '\'':	tmp1 = '\''; break;
						case '\"':	tmp1 = '\"'; break;
						case '\\':	tmp1 = '\\'; break;
						case 's':	tmp1 = ' '; break;
						case 'x':	tmp1 = __bihex2val(input+1); input += 2; break;
					}
				}
				else
					tmp1 = *input;

				buff[tmp1] = 1;
				return this;
			}

			if (*++input == '^')
			{
				val = 0, input++;
				_not();
			}

			while (*input && (*input != ']' || input[1]))
			{
				if (*input == '\\')
				{
					switch (*++input)
					{
						case 'n':	tmp1 = '\n'; break;
						case 'r':	tmp1 = '\r'; break;
						case 't':	tmp1 = '\t'; break;
						case 'b':	tmp1 = '\b'; break;
						case 'v':	tmp1 = '\v'; break;
						case 'f':	tmp1 = '\f'; break;
						case '\'':	tmp1 = '\''; break;
						case '\"':	tmp1 = '\"'; break;
						case '\\':	tmp1 = '\\'; break;
						case 's':	tmp1 = ' '; break;
						case 'x':	tmp1 = __bihex2val(input+1); input += 2; break;
					}
				}
				else
					tmp1 = *input;

				if (range)
				{
					for (int i = tmp0; i <= tmp1; i++)
						buff[i] = val;

					range = 0, input++;
					continue;
				}

				if (input[1] == '-')
				{
					input += (range = 1) + 1;
					tmp0 = tmp1;
					continue;
				}

				buff[tmp1] = val;
				input++;
			}

			return this;
		}

		/**
		**	Appends a formatted character to a string.
		*/
		char *__appendChar (char *o, int i)
		{
			if (i > 32 && i < 128 && i != 92 && i != '^')
			{
				*o++ = i;
				return o;
			}

			sprintf (o, "\\x%02x", i);
			return o + strlen(o);
		}

		/**
		**	Returns a human-readable string representation of the character set.
		*/
		LString *toString ()
		{
			static char temp[256];
			char *out = temp;

			int si = -1, j = 256;

			*out++ = '[';

			if (buff['-'])
			{
				if (!buff['-'-1] || !buff['-'+1])
				{
					*out++ = '-';
					j = 256;
				}
			}

			for (int i = 0; i <= 256; i++)
			{
				if (i == 256 || !buff[i] || i == j)
				{
					if (si == -1) continue;

					if (i - si > 1)
					{
						if (i - si == 2)
						{
							out = __appendChar (out, si);
							out = __appendChar (out, i-1);
						}
						else
						{
							out = __appendChar (out, si);
							*out++ = '-';
							out = __appendChar (out, i-1);
						}
					}
					else
					{
						if (si != '-') out = __appendChar (out, si);
					}

					si = -1;
				}
				else
				{
					if (si == -1) si = i;
				}
			}

			*out++ = ']';
			*out = '\0';

			return LString::alloc(temp);
		}

		/**
		**	Performs a NOT operation on the set.
		*/
		CharSet *_not ()
		{
			for (int i = 0; i < 256; i++)
				buff[i] = !buff[i];

			return this;
		}

		/**
		**	Performs an OR operation on the current set using the specified set as the second operand.
		*/
		CharSet *_or (CharSet *set)
		{
			for (int i = 0; i < 256; i++)
				buff[i] |= set->buff[i];

			return this;
		}

		/**
		**	Performs an AND operation on the current set using the specified set as the second operand.
		*/
		CharSet *_and (CharSet *set)
		{
			for (int i = 0; i < 256; i++)
				buff[i] &= set->buff[i];

			return this;
		}
	};
};
