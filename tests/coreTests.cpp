#include <catch2/catch.hpp>
#include "tcppLibrary.hpp"
#include <iostream>


using namespace tcpp;


TEST_CASE("Preprocessor Tests")
{
	auto errorCallback = [](const TErrorInfo&)
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

	SECTION("TestProcess_PassSourceWithSimpleMacroWithoutValue_ReturnsSourceWithExpandedMacro")
	{
		std::string inputSource = "#define VALUE\nVALUE";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == "1");
	}

	SECTION("TestProcess_PassSourceWithCorrectFuncMacro_ReturnsSourceWithExpandedMacro")
	{
		std::string inputSource = "#define ADD(X, Y) X + Y\n void main()\n{\n\treturn ADD(2, 3);\n}";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		std::cout << preprocessor.Process() << std::endl;
	}

	SECTION("TestProcess_PassSourceWithIncludeDirective_ReturnsSourceStringWithIncludeDirective")
	{
		std::string inputSource = "#include <system>\n#include \"non_system_path\"\n void main()\n{\n\treturn ADD(2, 3);\n}";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		const std::tuple<std::string, bool> expectedPaths[]{ { "system", true }, { "non_system_path", false } };
		short currExpectedPathIndex = 0;

		Preprocessor preprocessor(lexer, errorCallback, [&input, &currExpectedPathIndex, &expectedPaths](const std::string& path, bool isSystem)
		{
			auto expectedResultPair = expectedPaths[currExpectedPathIndex++];

			REQUIRE(path == std::get<std::string>(expectedResultPair));
			REQUIRE(isSystem == std::get<bool>(expectedResultPair));
			return &input;
		});
		preprocessor.Process();
	}

	SECTION("TestProcess_PassSourceWithIncludeDirective_ReturnsSourceStringWithIncludeDirective")
	{
		std::string inputSource = "__LINE__\n__LINE__\n__LINE__";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == "1\n2\n3");
	}

	SECTION("TestProcess_PassSourceWithStringizeOperator_ReturnsSourceWithStringifiedToken")
	{
		std::string inputSource = "#define FOO(Name) #Name\n FOO(Text)";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == " Text");
	}

	/*SECTION("TestProcess_PassSourceWithConcatenationOperator_ReturnsSourceWithConcatenatedTokens")
	{
		std::string inputSource = "#define CAT(X, Y) X ## Y\n CAT(4, 2)";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == " 42");
	}*/

	SECTION("TestProcess_PassSourceWithConditionalBlocks_ReturnsSourceWithoutThisBlock")
	{
		std::string inputSource = "#if FOO\none#endif\n two three";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == "\n two three");
	}

	SECTION("TestProcess_PassSourceWithConditionalBlocks_ReturnsSourceWithoutIfBlock")
	{
		std::string inputSource = "#if FOO\n // this block will be skiped\n if block\n#else\n else block #endif";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == "\n else block ");
	}
	
	SECTION("TestProcess_PassSourceWithConditionalBlocks_ReturnsSourceWithoutElseBlock")
	{
		std::string inputSource = "#if 1\n if block\n#else\n else block #endif";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == " if block\n");
	}

	SECTION("TestProcess_PassSourceWithElifBlocks_ReturnsSourceWithElabledElifBlock")
	{
		std::string inputSource = "#if 0\none\n#elif 1\ntwo\n#else\nthree\n#endif";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == "two\n");
	}

	SECTION("TestProcess_PassSourceWithFewElifBlocks_ReturnsSourceWithElabledElifBlock")
	{
		std::string inputSource = "#if 0\none\n#elif 0\ntwo\n#elif 1\nthree\n#else\nfour\n#endif";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == "three\n");
	}

	SECTION("TestProcess_PassSourceWithInvalidElseBlock_ReturnsError")
	{
		std::string inputSource = "#if 0\none\n#elif 0\ntwo\n#else\nfour\n#elif 1\nthree\n#endif";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		bool result = false;

		Preprocessor preprocessor(lexer, [&result](auto&&)
		{
			result = true;
		}); 

		preprocessor.Process();
		REQUIRE(result);
	}

	SECTION("TestProcess_PassSourceWithNestedConditionalBlocks_CorrectlyProcessedNestedBlocks")
	{
		std::string inputSource = "#if 1\none\n#if 0\ntwo\n#endif\nfour\n#elif 0\nthree\n#endif";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == "one\n\nfour\n");
	}

	SECTION("TestProcess_PassSourceWithIfdefBlock_CorrectlyProcessesIfdefBlock")
	{
		std::string inputSource = "#ifdef FOO\none\n#endif\ntwo";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == "\ntwo");
	}

	SECTION("TestProcess_PassSourceWithIfndefBlock_CorrectlyProcessesIfndefBlock")
	{
		std::string inputSource = "#ifndef FOO\none\n#endif\ntwo";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == "one\n\ntwo");
	}

	SECTION("TestProcess_PassSource_ReturnsProcessedSource")
	{
		std::string inputSource = "#define FOO\n#ifdef FOO\none\n#endif";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == "one\n");
	}

	SECTION("TestProcess_PassSourceWithIncludeDirective_ReturnsProcessedSource")
	{
		StringInputStream input("#include <system>\ntwo");
		StringInputStream systemInput("one\n");
		Lexer lexer(input);

		bool result = true;

		Preprocessor preprocessor(lexer, [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		}, [&systemInput](auto&&, auto&&)
		{
			return &systemInput;
		});

		REQUIRE((result && (preprocessor.Process() == "one\ntwo")));
	}

	SECTION("TestProcess_PassSourceWithIncludeGuards_ReturnsProcessedSource")
	{
		std::string inputSource = R"(
			#define FOO
			
			#include <system>

			#ifndef FILE_H
			#define FILE_H

			#ifdef FOO
				#define BAR(x) x
			#endif

			#ifdef FOO2
				#define BAR(x) x,x
			#endif

			#endif
		)";

		std::string systemSource = R"(
			#define FOO3			
			int x = 42;
		)";

		StringInputStream input(inputSource);
		StringInputStream systemInput(systemSource);
		Lexer lexer(input);

		bool result = true;

		Preprocessor preprocessor(lexer, [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		}, [&systemInput](auto&&, auto&&)
		{
			return &systemInput;
		});

		preprocessor.Process();
		REQUIRE(result);
	}

	SECTION("TestProcess_PassSourceWithFunctionMacro_ReturnsProcessedSource")
	{
		std::string inputSource = "#define FOO(X, Y) Foo.getValue(X, Y)\nFOO(42, input.value)";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		bool result = true;

		Preprocessor preprocessor(lexer, [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		});

		std::cout << preprocessor.Process() << std::endl;
		REQUIRE(result);
	}

	SECTION("TestProcess_PassFloatingPointValue_ReturnsThisValue")
	{
		std::string inputSource = "1.0001 1.00001f vec4(1.0f, 0.2, 0.223, 1.0001f);";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		bool result = true;

		Preprocessor preprocessor(lexer, [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		});

		auto&& output = preprocessor.Process();
		std::cout << output << std::endl;
		REQUIRE(output == inputSource);
	}

	SECTION("TestProcess_PassFloatingPointValue_ReturnsThisValue2")
	{
		std::string inputSource = "float c = nebula(layer2_coord * 3.0) * 0.35 - 0.05";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		bool result = true;

		Preprocessor preprocessor(lexer, [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		});

		auto&& output = preprocessor.Process();
		std::cout << output << std::endl;
		REQUIRE(output == inputSource);
	}

	SECTION("TestProcess_PassTwoStringsWithConcatOperation_ReturnsSingleString")
	{
		std::string inputSource = "AAA   ## BB";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		bool result = true;

		Preprocessor preprocessor(lexer, [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		});

		std::string str = preprocessor.Process();
		std::cout << str << std::endl;

		REQUIRE((result && (str == "AAABB")));
	}

	SECTION("TestProcess_PassSourceWithFunctionMacro_ReturnsProcessedSource")
	{
		std::string inputSource = "#define FOO(X) \\\nint X; \\\nint X ## _Additional;\nFOO(Test)";
		std::string expectedResult = "int Test;int Test_Additional;";

		StringInputStream input(inputSource);
		Lexer lexer(input);

		bool result = true;

		Preprocessor preprocessor(lexer, [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		});

		REQUIRE((result && (preprocessor.Process() == expectedResult)));
	}

	SECTION("TestProcess_PassNestedFunctionMacroIntoAnotherFunctionMacro_ReturnsProcessedSource")
	{
		std::string inputSource = "#define FOO(X, Y) X(Y)\nFOO(Foo, Test(0, 0))";
		std::string expectedResult = "Foo(Test(0, 0))";

		StringInputStream input(inputSource);
		Lexer lexer(input);

		bool result = true;

		Preprocessor preprocessor(lexer, [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		});

		std::string actualResult = preprocessor.Process();
		std::cout << actualResult << std::endl;

		REQUIRE((result && (actualResult == expectedResult)));
	}

	SECTION("TestProcess_PassEscapeSequenceInsideLiteralString_CorrectlyPreprocessIt")
	{
		std::string inputSource = R"(
		void main() {
			printf("test \n"); 
		})";

		std::string expectedResult = R"(
		void main() {
			printf("test \n"); 
		})";
		
		StringInputStream input(inputSource);
		Lexer lexer(input);

		bool result = true;

		Preprocessor preprocessor(lexer, [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		});

		std::string actualResult = preprocessor.Process();
		std::cout << actualResult << std::endl;

		REQUIRE((result && (actualResult == expectedResult)));
	}

	SECTION("TestProcess_PassTextWithEscapeSequenceWithinCommentary_CommentsAreBypassedWithoutAnyChanges")
	{
		std::string inputSource = R"(
		Line above

		// "\p"
		Line below
		float getNumber() {
			return 1.0;
		})";

		std::string expectedResult = R"(
		Line above

		// "\p"
		Line below
		float getNumber() {
			return 1.0;
		})";

		StringInputStream input(inputSource);
		Lexer lexer(input);

		bool result = true;

		Preprocessor preprocessor(lexer, [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		});

		std::string actualResult = preprocessor.Process();
		REQUIRE((result && (actualResult == expectedResult)));
	}


}