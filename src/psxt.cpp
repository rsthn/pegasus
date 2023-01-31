
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
#include "psxt/Parser.h"
#include "psxt/itemsets/ItemSetBuilder"
#include "psxt/states/FsmStateBuilder"

#include "gen/Cpp"

using namespace asr::utils;
using namespace psxt;

int main (int argc, char *argv[])
{
	List<String*> *sources = new List<String*> ();
	String *fmt = new String("cpp"), *name = new String(""), *outdir = new String(""), *str;
	bool dumpStates = false, dumpItemSets = false;
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
	Generator *gen;
	gen = new gen::GeneratorCpp (context, suffix);

	// ------------------------
	LString *initialSymbol = LString::alloc("__start__");
	int numScannerStates = 0, numParserStates = 0;

	// Generate itemsets for the lexer.
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

		//str = outdir->concat("scanner")->append(suffix);
		//os = fopen (str->c_str(), "wb");
		//if (os == nullptr) printf ("Crap! Unable to open %s\n", str->c_str()); else printf ("Ready to write %s\n", str->c_str());
		//gen->generate (states, SECTION_LEXICON, os, name);
		//fclose (os);
		//delete str;

	delete states;
	delete itemsets;

	// Cleanup.
	initialSymbol->free();

	delete gen;
	delete sources;
	delete context;
	delete name;
	delete outdir;
	delete fmt;

	LString::finish();

	printf ("psxt: Generated %u scanner states, and %u parser states.\n", numScannerStates, numParserStates);
	printf ("psxt: Finished.\n");

	if (asr::memblocks) printf ("Warning: System might be leaking (left %u blocks wandering).\n", asr::memblocks);

	printf ("\n");
	return 0;
}
