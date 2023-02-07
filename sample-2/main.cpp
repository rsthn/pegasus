
#include <asr/utils/List>
#include "scanner.h"

int main()
{
	asx::FileDataProvider src("input.txt");
	asx::Scanner scanner(&src);

	while (true)
	{
		auto token = scanner.shiftToken();
		if (token->getType() == -1)
			break;

		printf("%d,%d => [%d] %s\n", token->getLine(), token->getCol(), token->getType(), token->getValue());
	}

	return 0;
}
