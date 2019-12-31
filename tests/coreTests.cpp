#include <catch2/catch.hpp>
#include "tcppLibrary.hpp"
#include <iostream>


using namespace tcpp;


TEST_CASE("Preprocessor Tests")
{
	auto errorCallback = []()
	{
		REQUIRE(false);
	};

	SECTION("TestProcess_PassSourceWithoutMacros_ReturnsEquaivalentSource")
	{
		std::string inputSource = "void main/* this is a comment*/(/*void*/)\n{\n\treturn/*   */ 42;\n}";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(!preprocessor.Process().empty());
	}

	SECTION("TestProcess_PassSourceWithSimpleMacro_ReturnsSourceWithExpandedMacro")
	{
		std::string inputSource = "#define VALUE 42\n void main()\n{\n\treturn VALUE;\n}";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		std::cout << preprocessor.Process() << std::endl;
	}
}