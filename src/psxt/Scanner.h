/*
**	psxt::Scanner
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

#include <asr/io/FileInputStream.h>

#include <psxt/LString.h>
#include <psxt/Token.h>
#include <psxt/ErrorDefs.h>

namespace psxt
{
	using asr::io::FileInputStream;

	/**
	**	Definition of all the tokens recognizable.
	*/
	#define	TTYPE_IDENTIFIER		0x01
	#define	TTYPE_NUMBER			0x02
	#define TTYPE_SQSTRING			0x03
	#define	TTYPE_DQSTRING			0x04
	#define	TTYPE_SYMBOL			0x05
	#define	TTYPE_NVALUE			0x06
	#define TTYPE_BLOCK				0x07
	#define	TTYPE_EOF				0x70
	#define	TTYPE_UNKNOWN			0x71

	/**
	**	Simple implementation of a scanner. Its responsibility is to scan symbols from an input stream and return
	**	token definitions.
	*/

	class Scanner
	{
		protected:

		/**
		**	Input stream.
		*/
		asr::io::FileInputStream *input;
		
		/**
		**	Name of the source being scanned.
		*/
		String *source;

		/**
		**	Internal token used to store the most recently scanned token.
		*/
		Token *tempToken;

		/**
		**	Temporal buffer to read tokens.
		*/
		char buffer[2048];

		/**
		**	Last pushed-back character to be used when reading from the input.
		*/
		int lastch;

		/**
		**	Current line number and column number.
		*/
		unsigned line, col;

		public:
		
		/**
		**	Constructs the scanner instance to obtain data from the specified data stream. The specified source
		**	name is used only to provide a more accurate description of the source where a token was found. The
		**	input stream will be deleted upon deletion of the scanner.
		*/
		Scanner (String *sourceName, FileInputStream *input)
		{
			this->source = sourceName;
			this->input = input;

			this->tempToken = new Token (this->source);

			this->lastch = -1;
			this->line = this->col = 1;
		}

		/**
		**	Opens a read only data stream for the specified filename and constructs the scanner instance to obtain
		**	data from it. The input stream will be deleted upon deletion of the scanner.
		*/
		Scanner (String *filename)
		{
			this->source = filename;
			this->input = new FileInputStream (filename->c_str());

			this->tempToken = new Token (this->source);

			this->lastch = -1;
			this->line = this->col = 1;
		}

		/**
		**	Destroys the instance of the scanner.
		*/
		virtual ~Scanner ()
		{
			delete this->input;
			delete this->tempToken;
		}

		/**
		**	Returns the last token successfully scanned. This object is overwritten on each call to nextToken(),
		**	therefore if you need a persistent copy of the token you should clone it first.
		*/
		Token *getToken ()
		{
			return this->tempToken;
		}

		/**
		**	Returns the source file currently being scanned.
		*/
		String *getSource ()
		{
			return this->source;
		}

		/**
		**	Scans the next token from the input stream and returns true or false if it was extracted or not.
		*/
		bool nextToken ()
		{
			int s_line, s_col, a = lastch, type = TTYPE_UNKNOWN, ps, s = 0;
			char *buff = buffer;
			bool cont = true;

			//violet
			//if (!this->input->isReady())
			//	return false;

			bool reuse = a == -1 ? false : true;
			lastch = -1;

			while (cont)
			{
				if (reuse) {
					if (a == 10) line--;
					col--;
				}

				if (s == 0) {
					s_line = line;
					s_col = col;
				}

				if (!reuse) a = input->readByte();

				col++;

				if (a == 10) {
					col = 1; line++;
				}

				reuse = false;

				switch (s)
				{
					case 0: // Initial State
						if (a == -1) { cont = false; break; }
						if (a <= 32) { s = 1; break; }
						if (a == '/') { s = 2; break; }

						if (a >= '0' && a <= '9') { *buff++ = a; s = 6; break; }
						if (a == '@' || a == '$' || a == '_' || (a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z')) { *buff++ = a; s = 8; break; }

						if (a == '\'') { s = 9; break; }
						if (a == '\"') { s = 10; break; }

						if (a == '{') { s = 11; break; }

						s = 7; *buff++ = a;
						break;

					case 1: // Eat-up white space.
						if (a == -1 || a > 32) { s = 0; reuse = true; }
						break;

					case 2: // [/]/ | [/]* | [/]
						if (a == '/') { s = 3; break; }
						if (a == '*') { s = 4; break; }

						*buff++ = '/'; cont = false; type = TTYPE_SYMBOL;
						break;

					case 3: // [//]
						if (a == -1 || a == 10) s = 0;
						break;

					case 4: // [/*] ... */
						if (a == -1) { s = 0; break; }
						if (a == '*') { s = 5; break; }
						break;

					case 5: // [/* ... *]/
						if (a == -1 || a == '/') s = 0; else s = 4;
						break;

					case 6: // Number
						if (a == -1 || !(a >= '0' && a <= '9'))
						{
							cont = false; type = TTYPE_NUMBER;
						}
						else
							*buff++ = a;

						break;

					case 7: // Symbol
						cont = false; type = TTYPE_SYMBOL;
						break;

					case 8: // Identifier
						if (a == -1 || !((a == '@' || a == '$' || a == '_' || a == '-' || (a >= '0' && a <= '9') || (a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z'))))
						{
							cont = false; type = TTYPE_IDENTIFIER;
						}
						else
							*buff++ = a;

						break;

					case 9: // Single-quoted string.
						if (a == '\\') { ps = 9; s = 128; break; }

						if (a == -1 || a == '\'')
						{
							s = 255; type = TTYPE_SQSTRING;
						}
						else
							*buff++ = a;

						break;

					case 10: // Double-quoted string.
						if (a == '\\') { ps = 10; s = 128; break; }

						if (a == -1 || a == '\"')
						{
							s = 255; type = TTYPE_DQSTRING;
						}
						else
							*buff++ = a;

						break;

					case 11: // Block contents.
						if (a == -1 || a == '}')
						{
							s = 255; type = TTYPE_BLOCK;
						}
						else
							*buff++ = a;

						break;

					case 128: // Escape sequence.

						*buff++ = '\\';
						*buff++ = a;

						/*switch (a)
						{
							case 'b':	*buff++ = '\b'; break;
							case 'f':	*buff++ = '\f'; break;
							case 'v':	*buff++ = '\v'; break;
							case 'r':	*buff++ = '\r'; break;
							case 'n':	*buff++ = '\n'; break;
							case 't':	*buff++ = '\t'; break;
							case 's':	*buff++ = ' '; break;
							case '\\':	*buff++ = '\''; break;
							case '\"':	*buff++ = '\"'; break;
							case '\'':	*buff++ = '\''; break;
							default:	*buff++ = a; break;
						}*/

						s = ps;
						break;

					case 255: // Stop.
						cont = false;
						break;
				}

				lastch = a;
			}

			if (a == -1 && type == TTYPE_UNKNOWN)
				return false;

			*buff++ = '\0';

			tempToken->set(buffer, type, s_line, s_col);

			return true;
		}

		/**
		**	Returns an injected token with the specified data.
		*/
		Token *buildToken(int type, const char *value)
		{
			tempToken->set(value, type, line, col);
			return new Token (tempToken);
		}
	};
};
