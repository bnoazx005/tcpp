#include <catch2/catch.hpp>
#include "tcppLibrary.hpp"
#include <iostream>


using namespace tcpp;


TEST_CASE("Preprocessor Tests")
{

	SECTION("TestProcess_PassSourceWithoutMacros_ReturnsEquaivalentSource")
	{
		std::string inputSource = "void main()\n{\n\treturn 42;\n}";
		StringInputStream input(inputSource);
		Lexer lexer(input);

#if 0
		while (lexer.HasNextToken())
		{
			auto token = lexer.GetNextToken();
			;
		}
#endif

		Preprocessor preprocessor(lexer);
		while (preprocessor.HasNext())
		{
			std::cout << preprocessor.Get();
		}
		//REQUIRE(preprocessor.Process("") == "");
	}
}