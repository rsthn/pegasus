/*
**	Pegasus Syntax Translator (PSXT)
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

#include <Integer.h>

#include <psxt/Scanner.h>
#include <psxt/Context.h>
#include <psxt/Parser.h>

#include <psxt/itemsets/ItemSetBuilder.h>
#include <psxt/states/FsmStateBuilder.h>

#include <psxt/Generator.h>
#include <gen/Cpp.h>

using namespace asr::utils;
using psxt::LString;

int main (int argc, char *argv[])
{
	printf("STARTING_WITH=%d\n", asr::memblocks);

	List<String*> *sources = new List<String*> ();
	String *str, *fmt = new String ("cpp"), *name = new String (""), *outdir = new String ("");
	bool dump = false, dumpi = false;
	const char *suffix;

	printf ("RedStar Pegasus v5.00 Copyright (c) 2006-%d RedStar Technologies, All rights reserved.\n", __YEAR__);

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
			{
				needsVal = false;
			}

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
					dump = true;
					break;

				case 'i':
					dumpi = true;
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

	psxt::Context *context = new psxt::Context ();
	psxt::Parser *parser = new psxt::Parser (context);

	for (Linkable<String*> *i = sources->head(); i; i = i->next())
	{
		psxt::Scanner *scanner = new psxt::Scanner (i->value);
		parser->parse (scanner);

		delete scanner;
	}

	delete parser;

	psxt::Generator *gen;

	/* if (fmt->equals ("c"))
	{
		gen = new gen::GeneratorC (context);
		suffix = ".h";
	} */

	gen = new gen::GeneratorCpp (context);
	suffix = ".h";

	FILE *os;

	int lsnum = 0, psnum = 0;

	LString *initialSymbol = LString::alloc("__start__");

	/* ****************************** */
	LList<psxt::ItemSet*> *itemsets = psxt::ItemSetBuilder::build (context, SECTION_LEXICON, initialSymbol);
	if (itemsets != nullptr)
	{
		lsnum = itemsets->length();

		/*if (dumpi)
		{
			os = fopen ("lexicon-itemsets.txt", "wb");

			for (Linkable<psxt::ItemSet*> *i = itemsets->head(); i; i = i->next())
				i->value->dump (os);

			fclose (os);
		}*/

		#if ENABLE_GENERATOR==1
		printf("[36mAbout to start FsmStateBuilder for the lexicon.[0m\n");
		List<psxt::FsmState*> *states = psxt::FsmStateBuilder::build (context, SECTION_LEXICON, itemsets->head()->value);

		if (dump)
		{
			os = fopen ("lexicon-states.txt", "wb");

			for (Linkable<psxt::FsmState*> *i = states->head(); i; i = i->next())
				i->value->dump (os);

			fclose (os);
		}

			str = outdir->concat("scanner")->append(suffix);
			os = fopen (str->c_str(), "wb");
			if (os == nullptr) printf ("Crap! Unable to open %s\n", str->c_str()); else printf ("Ready to write %s\n", str->c_str());
			gen->generate (states, SECTION_LEXICON, os, name);
			fclose (os);
			delete str;

		delete states;
		#endif
		delete itemsets;

		//violet
	}
#if 0
	/* ****************************** */
	itemsets = psxt::ItemSetBuilder::build (context, SECTION_GRAMMAR, initialSymbol);
	if (itemsets != nullptr)
	{
		psnum = itemsets->length();

		if (dumpi)
		{
			os = fopen ("grammar-itemsets.txt", "wb");

			for (Linkable<psxt::ItemSet*> *i = itemsets->head(); i; i = i->next())
				i->value->dump (os);

			fclose (os);
		}

		#if ENABLE_GENERATOR==1
		printf("[36mAbout to start FsmStateBuilder for the grammar.[0m\n");
		List<psxt::FsmState*> *states = psxt::FsmStateBuilder::build (context, SECTION_GRAMMAR, itemsets->head()->value);

		if (dump)
		{
			os = fopen ("grammar-states.txt", "wb");

			for (Linkable<psxt::FsmState*> *i = states->head(); i; i = i->next())
				i->value->dump (os);

			fclose (os);
		}

			str = outdir->concat("parser")->append(suffix);
			os = fopen (str->c_str(), "wb");
			if (os == nullptr) printf ("Crap! Unable to open %s\n", str->c_str()); else printf ("Ready to write %s\n", str->c_str());
			gen->generate (states, SECTION_GRAMMAR, os, name);
			fclose (os);
			delete str;

		delete states->clear();
		#endif
		delete itemsets->clear();
	}

#endif

	/* ~ */
	initialSymbol->free();

	printf("DELETE sources\n");
	delete sources;
	printf("DELETE context\n");
	delete context;
	printf("DELETE gen\n");
	delete gen;

	printf("DELETE name\n");
	delete name;
	printf("DELETE outdir\n");
	delete outdir;
	printf("DELETE fmt\n");
	delete fmt;

	LString::finish();

	//printf ("psxt: Generated %u scanner states, and %u parser states.\n", lsnum, psnum);
	printf ("psxt: Finished.\n");

	if (asr::memblocks) printf ("Warning: System might be leaking (left %u blocks wandering).\n", asr::memblocks);
	printf("FINISHED WITH = %u", asr::memblocks);

	printf ("\n");
	return 0;
}
