# Pegasus Syntax Translator (PSXT)

Copyright (c) 2006-2023, RedStar Technologies, All rights reserved.
https://rsthn.com/rstf/pegasus/

<br/>

# How to Use

Adding a plus or a minus before the name of a rule indicates its visibility (public or private respectively). Private rules are used internally by the scanner/parser.

Lexical rules are defined in the `[lexicon]` section as production-rules with identifiers, strings, escape-sequence and charsets.

	token
		:	identifier
		|	"[0-9]"
		|	"\n"
		|	-blanks
		;


Arrays are defined in the `[arrays]` section, as follows:

[arrays]

	keyword (identifier) : "hello", "world" ;

Grammar is defined as non-terminals with production-rules in the `[grammar]` section, composed of identifiers and tokens. Each element in a production rule can be referenced by its index and the $ symbol (i.e. $0, $1, $$2, etc) - when using double-dollar the element is marked as "unused" which will cause auto-deletion code to be added by the generator.

[grammar]

	rule-name {returntype}
		:	+ identifier("value")	{ action }
		|	- identifier("value")	{ action }
		;

The parser uses a variable named "context" of type Context* which can also be used in rule actions. When using the element reference variable ($0, $1, etc) each type will be consistent with the return-type of the non-terminal, when the element is actually a token from the scanner it will be of type Token* as included in the generated source files.

Note that production rule actions can have any format and its content will be interpreted by the generator used (C, C++, etc). When using the C/C++ generator the action rule is considered a single value and a "return" is implicitly prepended to the action.
