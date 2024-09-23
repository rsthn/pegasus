
#include <assert.h>
#include <time.h>
#include <stdlib.h>

#include "Parser"
#include "itemsets/ItemSetBuilder"
#include "states/FsmStateBuilder"

#include "gen/Cpp"

using namespace asr::utils;
using namespace psxt;

/**
 * @brief Entry point.
 */
int main (int argc, char *argv[])
{
    List<String*> *sources = new List<String*> ();
    String *fmt = new String("cpp"), *name = new String(""), *outdir = new String(""), *str;

    bool dumpStates = false, dumpItemSets = false, generateCode = true;
    const char *suffix;

    printf ("Pegasus v5.00 Copyright (c) 2006-%d RedStar Technologies, All rights reserved.\n", 2024);
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

    // Use the C++ code generator.
    Generator *generator;
    generator = new gen::GeneratorCpp (context, suffix);


    // ---------------------
    // Generate the scanner's itemsets, FSM states, and output code.

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

    if (generateCode)
    {
        str = outdir->concat("scanner")->append(suffix);
        FILE *os = fopen(str->c_str(), "wb");
        if (os != nullptr)  {
            generator->generate (states, Context::SectionType::LEXICON, os, name);
            fclose (os);
        }
        delete str;
    }

    delete states->clear();
    delete itemsets->clear();

    // ---------------------
    // Generate the parser's itemsets, FSM states, and output code.

    itemsets = ItemSetBuilder::build (context, Context::SectionType::GRAMMAR, initialSymbol);
    if (itemsets != nullptr) {
        numParserStates = itemsets->length();
        if (dumpItemSets) {
            FILE *os = fopen("grammar-itemsets.txt", "wb");
            for (Linkable<ItemSet*> *i = itemsets->head(); i; i = i->next())
                i->value->dump(os);
            fclose(os);
        }
    }

    states = FsmStateBuilder::build (context, Context::SectionType::GRAMMAR, itemsets->head()->value);
    if (dumpStates) {
        FILE *os = fopen("grammar-states.txt", "wb");
        for (Linkable<FsmState*> *i = states->head(); i; i = i->next())
            i->value->dump(os);
        fclose(os);
    }

    if (generateCode)
    {
        str = outdir->concat("parser")->append(suffix);
        FILE *os = fopen(str->c_str(), "wb");
        if (os != nullptr)  {
            generator->generate (states, Context::SectionType::GRAMMAR, os, name);
            fclose (os);
        }
        delete str;
    }

    delete states->clear();
    delete itemsets->clear();

    // ---------------------
    // Clean up everything.

    initialSymbol->free();

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

    if (asr::memblocks)
        printf ("\n\x1B[93mWarning:\x1B[0m Possible memory leak detected, left %u blocks wandering.\n", asr::memblocks);

    return 0;
}
