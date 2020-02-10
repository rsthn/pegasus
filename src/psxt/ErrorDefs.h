/*
**	psxt::ErrorDefs
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
/*
	First digit: Gravity: 1-4=Warnings, Notices. 5-7=Errors, 8-9=Fatal Errors.
	Second digit: Stage: 1=Load, 2=Scanner, 3=Parser, 4=Generator
	Last two digits: ID of message.
*/

	#define	E_UNEXPECTED_EOF		8201, "Unexpected end of file encountered."

	#define	E_EXPECTED_LBRACKET		8301, "Left bracket '[' expected."
	#define	E_EXPECTED_IDENTIFIER	8302, "Identifier expected."
	#define	E_EXPECTED_RBRACKET		8303, "Right bracket ']' expected."
	#define	E_INCORRECT_SECTION		8304, "Invalid section name specified: `%s'"
	#define	E_EXPECTED_COLON		8305, "Missing ':' after `%s'."
	#define E_MISSING_SEMICOLON		8306, "Missing ';' in definition of `%s'."
	#define	E_UNEXPECTED_ELEM		8307, "Unexpected `%s` found in definition of `%s'."
	#define E_EXPECTED_VALUE		8308, "Expected string value for `%s` in %s#%u."
	#define E_EXPECTED_RPAREN		8309, "Missing right parenthesis after value of `%s` in %s#%u."
	#define E_INCONSISTENT_TYPE		8310, "Return types are inconsistent in `%s`."
	#define E_EXPECTED_LPAREN		8311, "Missing left parenthesis on array: `%s'."
	#define E_EXPECTED_LNONTERM		8312, "Expected lexical non-terminal name in array: `%s`."
	#define E_EXPECTED_RPAREN2		8313, "Missing right parenthesis in array: `%s'."
	#define	E_EXPECTED_COLON2		8314, "Missing ':' in definition of array `%s'."
	#define E_EXPECTED_ITEM			8315, "Expected item value (string) in array: `%s'."
	#define E_EXPECTED_COMMA		8316, "Expected comma (,) after `%s` in array: `%s'."

	#define E_INVALID_RECURSION		4301, "Conflictive recursive rule (#%u) in definition of `%s`."
	#define E_REQUIRED_ELEM			4302, "Required at least one element to export %s#%u."
	#define E_NOT_ALLOWED_VALUE		4303, "Element values are not allowed in lexical grammar."
	#define E_INFER_FAILED			4304, "Unable to infer return type for `%s`."

	#define E_UNDEF_NONTERM			8401, "Undefined non-terminal: `%s`."
	#define E_REDUCE_REDUCE			8402, "(State %c%u) Reduce/Reduce conflict found"
	#define	E_SHIFT_REDUCE			8403, "(State %c%u) Shift/Reduce conflict found"

	#define	SECTION_CODE(x)			(x == SECTION_GRAMMAR ? 'G' : 'L')
};
