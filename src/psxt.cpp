
#define __YEAR__ 2018
#define ENABLE_GENERATOR 0

#include <assert.h>
#include <time.h>
#include <stdlib.h>

#include <asr/utils/List>
#include <asr/utils/String>
#include <asr/utils/Triad>
#include <asr/utils/LList>
#include <asr/utils/Traits>

#include "psxt/Scanner.h"
#include "psxt/Context"
#include "psxt/Parser"
#include "psxt/itemsets/ItemSetBuilder"
#include "psxt/states/FsmStateBuilder"

#include "gen/Cpp"

using namespace asr::utils;
using namespace psxt;

int main (int argc, char *argv[])
{
	List<String*> *sources = new List<String*> ();
	String *fmt = new String("cpp"), *name = new String(""), *outdir = new String(""), *str;
	bool dumpStates = false, dumpItemSets = false, generateCode = true;
	const char *suffix;

	printf ("Pegasus v5.00 Copyright (c) 2006-%d RedStar Technologies, All rights reserved.\n", __YEAR__);
	if (argc < 2)
	{
		printf (
			"Syntax: psxt [options] sx-files\n\n"
			"Options:\n"
			"    -f xx    Set code generation output format.\n"
			"    -n xx    Set base file output name.\n"
			"    -o xx    Set output directory for the generated files.\n"
			"    -d       Dump states.\n"
			"    -i       Dump item sets.\n"
			"\n"
		);

		return 1;
	}

	printf ("\n");

	for (int i = 1; i < argc; i++)
	{
		char *arg = argv[i];
		if (*arg == '-')
		{
			bool needsVal = true;
			if (arg[1] == 'd' || arg[1] == 'i')
				needsVal = false;

			char *val = needsVal ? (arg[2] != '\0' ? arg+2 : argv[++i]) : nullptr;
			switch (arg[1])
			{
				case 'f':
					fmt->set (val);
					break;

				case 'n':
					name->set (val);
					break;

				case 'd':
					dumpStates = true;
					break;

				case 'i':
					dumpItemSets = true;
					break;

				case 'o':
					outdir->set(val);
					if (outdir->charAt(-1) != '/' && outdir->charAt(-1) != '\\') outdir->append("/");
					break;
			}
		}
		else
			sources->push (new String (arg));
	}

	// Prepare context to store the parsed sections and rules.
	Context *context = new Context();

	// Parse all provided source files.
	Parser parser (context);
	for (Linkable<String*> *i = sources->head(); i; i = i->next())
	{
		Scanner scanner (i->value);
		parser.parse(&scanner);
	}

	/* printf("\n---------\n");	
	for (auto i = context->getNonTerminalPairs(Context::SectionType::LEXICON)->head(); i; i = i->next())
	{
		printf("\n%s#%d\n", i->value->key->c_str(), i->value->value->getId());
		for (auto j = i->value->value->getRules()->head(); j; j = j->next())
		{
			printf("    #%d ", j->value->getId());
			for (auto k = j->value->getElems()->head(); k; k = k->next())
				printf(" %s@%d", k->value->getCstr(), k->value->getType());
			printf("\n");
		}
	}
	printf("\n---------\n\n"); */

	// Use the C++ code generator.
	Generator *generator;
	generator = new gen::GeneratorCpp (context, suffix);

	// ---------------------
	// Generate the itemsets, FSM states, and code for the scanner.

	LString *initialSymbol = LString::alloc("__start__");
	int numScannerStates = 0, numParserStates = 0;

	LList<ItemSet*> *itemsets = ItemSetBuilder::build (context, Context::SectionType::LEXICON, initialSymbol);
	if (itemsets != nullptr) {
		numScannerStates = itemsets->length();
		if (dumpItemSets) {
			FILE *os = fopen("lexicon-itemsets.txt", "wb");
			for (Linkable<ItemSet*> *i = itemsets->head(); i; i = i->next())
				i->value->dump(os);
			fclose(os);
		}
	}

	List<FsmState*> *states = FsmStateBuilder::build (context, Context::SectionType::LEXICON, itemsets->head()->value);
	if (dumpStates) {
		FILE *os = fopen("lexicon-states.txt", "wb");
		for (Linkable<FsmState*> *i = states->head(); i; i = i->next())
			i->value->dump(os);
		fclose(os);
	}

	if (generateCode) {
		str = outdir->concat("scanner")->append(suffix);
		FILE *os = fopen(str->c_str(), "wb");
		if (os != nullptr)  {
			generator->generate (states, Context::SectionType::LEXICON, os, name);
			fclose (os);
		}
		delete str;
	}

	// ---------------------
	// Clean up everything.

	initialSymbol->free();

	delete states;
	delete itemsets;
	delete generator;
	delete sources;
	delete context;
	delete name;
	delete outdir;
	delete fmt;

	LString::finish();

	if (numScannerStates && numParserStates)
		printf ("psxt: Generated %u scanner states, and %u parser states.\n", numScannerStates, numParserStates);
	else if (numScannerStates)
		printf ("psxt: Generated %u scanner states.\n", numScannerStates);
	else if (numParserStates)
		printf ("psxt: Generated %u parser states.\n", numParserStates);

	printf ("psxt: Finished.\n");

	if (asr::memblocks)
		printf ("\n\x1B[93mWarning:\x1B[0m Possible memory leak detected, left %u blocks wandering.\n", asr::memblocks);

	return 0;
}
