#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include <asx/context.h>

void Record (char *str)
{
	printf ("%s\n", str);
}

#include <asx/scanner.h>
#include <asx/parser.h>

using namespace asr::utils;

int main ()
{
	asx::FileDataProvider *input = new asx::FileDataProvider("input.txt");
	asx::Scanner *scanner = new asx::Scanner (input);
	asx::Token *token;
	asx::Context *context = new asx::Context ();

	/*while ((token = scanner->shiftToken())->getType() != -1)
	{
		printf ("%u: %s\n", token->getType(), token->getValue());
		delete token;
	}

	delete token;*/
	delete scanner;
	delete input;

	asx::Parser::parseFile ("input.txt", context);
	delete context;

	if (asr::memblocks) printf ("Warning: System might be leaking (left %u blocks wandering).\n", asr::memblocks);

	return 0;
}
