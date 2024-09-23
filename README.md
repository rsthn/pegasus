# Pegasus Syntax Translator (PSXT)

Pegasus is a modern, lightweight tool for creating parsers. It integrates both tokenization (breaking down input into meaningful symbols) and grammar analysis (defining the structure of valid inputs) in one package. Pegasus supports a variation of LALR(k) and LR(k) parsing, making it efficient for handling complex language structures while remaining easy to use and highly performant.

<br/>

# Syntax

Lexical rules are defined in the `[lexicon]` section as production-rules with identifiers, strings, escape-sequence and charsets.

```
[lexicon]

token
    :	identifier
    |	"[0-9]"
    |	"\n"
    |	-blanks
    ;
```

Arrays are collection of strings and are defined in the `[arrays]` section, each array is defined with a name and a base terminal defined in the lexicon section, for example:

```
[arrays]

keyword (identifier) : "hello", "world" ;
```

Grammar rules are defined as non-terminals with production-rules in the `[grammar]` section, composed of identifiers and tokens. Each element in a production rule can be referenced by its index and the `$` symbol (i.e. $0, $1, $$2, etc) - when using double-dollar the element is marked as "unused" which will cause auto-deletion code to be added by the generator.

```
[grammar]

rule-name {returntype}
    :	+ identifier("value")	{ action }
    |	- identifier("value")	{ action }
    ;
```

The parser uses a variable named "context" of type Context* which can also be used in rule actions. When using the element reference variable ($0, $1, etc) each type will be consistent with the return-type of the non-terminal, when the element is actually a token from the scanner it will be of type Token* as included in the generated source files.

Note that production rule actions can have any format and its content will be interpreted by the generator used (C, C++, etc). When using the C/C++ generator the action rule is considered a single value and a "return" is implicitly prepended to the action.

<small>NOTE: Adding a plus or a minus before the name of a rule indicates its visibility (public or private respectively). Private rules are used internally by the scanner/parser.
</small>
