#include <catch2/catch.hpp>
#include "tcppLibrary.hpp"
#include <iostream>


using namespace tcpp;


static bool ContainsMacro(const Preprocessor& preprocessor, const std::string& macroIdentifier)
{
	const auto& symTable = preprocessor.GetSymbolsTable();

	auto it = std::find_if(symTable.cbegin(), symTable.cend(), [&macroIdentifier](auto&& symTableEntry)
	{
		return symTableEntry.mName == macroIdentifier;
	});

	return it != symTable.cend();
}


TEST_CASE("Preprocessor Tests")
{
	auto errorCallback = [](const TErrorInfo&)
	{
		REQUIRE(false);
	};

	SECTION("TestProcess_PassSourceWithoutMacros_ReturnsEquaivalentSource")
	{
		std::string inputSource = "void main/* this is a comment*/(/*void*/)\n{\n\treturn/*   */ 42;\n}";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		REQUIRE(!preprocessor.Process().empty());
	}

	SECTION("TestProcess_PassSourceWithSimpleMacro_ReturnsSourceWithExpandedMacro")
	{
		std::string inputSource = "#define VALUE 42\n void main()\n{\n\treturn VALUE;\n}";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		std::cout << preprocessor.Process() << std::endl;
	}

	SECTION("TestProcess_PassSourceWithSimpleMacroWithoutValue_ReturnsSourceWithExpandedMacro")
	{
		std::string inputSource = "#define VALUE\nVALUE";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		REQUIRE(preprocessor.Process() == "1");
	}

	SECTION("TestProcess_PassSourceWithCorrectFuncMacro_ReturnsSourceWithExpandedMacro")
	{
		std::string inputSource = "#define ADD(X, Y) X + Y\n void main()\n{\n\treturn ADD(2, 3);\n}";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		std::cout << preprocessor.Process() << std::endl;
	}

	SECTION("TestProcess_PassSourceWithIncludeDirective_ReturnsSourceStringWithIncludeDirective")
	{
		std::string inputSource = "#include <system>\n#include \"non_system_path\"\n void main()\n{\n\treturn ADD(2, 3);\n}";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		const std::tuple<std::string, bool> expectedPaths[]{ { "system", true }, { "non_system_path", false } };
		short currExpectedPathIndex = 0;

		Preprocessor preprocessor(lexer, { errorCallback, [&inputSource, &currExpectedPathIndex, &expectedPaths](const std::string& path, bool isSystem)
		{
			auto expectedResultPair = expectedPaths[currExpectedPathIndex++];

			REQUIRE(path == std::get<std::string>(expectedResultPair));
			REQUIRE(isSystem == std::get<bool>(expectedResultPair));

			return std::make_unique<StringInputStream>("");
		} });
		preprocessor.Process();
	}

	SECTION("TestProcess_PassSourceWithIncludeDirective_ReturnsSourceStringWithIncludeDirective")
	{
		std::string inputSource = "__LINE__\n__LINE__\n__LINE__";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		REQUIRE(preprocessor.Process() == "1\n2\n3");
	}

	SECTION("TestProcess_PassSourceWithStringizeOperator_ReturnsSourceWithStringifiedToken")
	{
		std::string inputSource = "#define FOO(Name) #Name\n FOO(Text)";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		REQUIRE(preprocessor.Process() == " \"Text\"");
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
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		REQUIRE(preprocessor.Process() == "\n two three");
	}

	SECTION("TestProcess_PassSourceWithConditionalBlocks_ReturnsSourceWithoutIfBlock")
	{
		std::string inputSource = "#if FOO\n // this block will be skiped\n if block\n#else\n else block #endif";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		REQUIRE(preprocessor.Process() == "\n else block ");
	}
	
	SECTION("TestProcess_PassSourceWithConditionalBlocks_ReturnsSourceWithoutElseBlock")
	{
		std::string inputSource = "#if 1\n if block\n#else\n else block #endif";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		REQUIRE(preprocessor.Process() == " if block\n");
	}

	SECTION("TestProcess_PassSourceWithElifBlocks_ReturnsSourceWithElabledElifBlock")
	{
		std::string inputSource = "#if 0\none\n#elif 1\ntwo\n#else\nthree\n#endif";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		REQUIRE(preprocessor.Process() == "two\n");
	}

	SECTION("TestProcess_PassSourceWithFewElifBlocks_ReturnsSourceWithElabledElifBlock")
	{
		std::string inputSource = "#if 0\none\n#elif 0\ntwo\n#elif 1\nthree\n#else\nfour\n#endif";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		REQUIRE(preprocessor.Process() == "three\n");
	}

	SECTION("TestProcess_PassSourceWithInvalidElseBlock_ReturnsError")
	{
		std::string inputSource = "#if 0\none\n#elif 0\ntwo\n#else\nfour\n#elif 1\nthree\n#endif";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = false;

		Preprocessor preprocessor(lexer, { [&result](auto&&)
		{
			result = true;
		} });

		preprocessor.Process();
		REQUIRE(result);
	}

	SECTION("TestProcess_PassSourceWithNestedConditionalBlocks_CorrectlyProcessedNestedBlocks")
	{
		std::string inputSource = "#if 1\none\n#if 0\ntwo\n#endif\nfour\n#elif 0\nthree\n#endif";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		REQUIRE(preprocessor.Process() == "one\n\nfour\n");
	}

	SECTION("TestProcess_PassSourceWithIfdefBlock_CorrectlyProcessesIfdefBlock")
	{
		std::string inputSource = "#ifdef FOO\none\n#endif\ntwo";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		REQUIRE(preprocessor.Process() == "\ntwo");
	}

	SECTION("TestProcess_PassSourceWithIfndefBlock_CorrectlyProcessesIfndefBlock")
	{
		std::string inputSource = "#ifndef FOO\none\n#endif\ntwo";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		REQUIRE(preprocessor.Process() == "one\n\ntwo");
	}

	SECTION("TestProcess_PassNestedActiveIfdefBlockInsideOfAnotherInactiveIfdefBlock_TopBlockShouldBeRejected")
	{
		std::string inputSource = R"(
#define CONDITION_1

#ifdef CONDITION_0
	condition_0,
	#ifdef CONDITION_1
		condition_1
	#endif
#endif
)";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		std::string output = preprocessor.Process();
		REQUIRE((output.find("condition_1") == std::string::npos && output.find("condition_0") == std::string::npos));
	}

	SECTION("TestProcess_PassNestedActiveElseBlockInsideOfAnotherInactiveIfdefBlock_TopBlockShouldBeRejected")
	{
		std::string inputSource = R"(
#ifdef CONDITION_0
	condition_0,
	#ifdef CONDITION_1
		condition_1
	#else
		condition_1_else
	#endif
#endif
)";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		std::string output = preprocessor.Process();
		REQUIRE((
			output.find("condition_1") == std::string::npos &&
			output.find("condition_0") == std::string::npos &&
			output.find("condition_1_else") == std::string::npos));
	}

	SECTION("TestProcess_PassNestedActiveElifBlockInsideOfAnotherInactiveIfdefBlock_TopBlockShouldBeRejected")
	{
		std::string inputSource = R"(
#define CONDITION_2

#ifdef CONDITION_0
	condition_0,
	#ifdef CONDITION_1
		condition_1
	#elif CONDITION_2
		condition_1_else
	#endif
#endif
)";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		std::string output = preprocessor.Process();
		REQUIRE((
			output.find("condition_1") == std::string::npos &&
			output.find("condition_0") == std::string::npos &&
			output.find("condition_1_else") == std::string::npos));
	}

	SECTION("TestProcess_PassSource_ReturnsProcessedSource")
	{
		std::string inputSource = "#define FOO\n#ifdef FOO\none\n#endif";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		Preprocessor preprocessor(lexer, { errorCallback });
		REQUIRE(preprocessor.Process() == "one\n");
	}

	SECTION("TestProcess_PassSourceWithIncludeDirective_ReturnsProcessedSource")
	{
		const std::string input("#include <system>\ntwo");
		const std::string systemInput("one\n");
		Lexer lexer(std::make_unique<StringInputStream>(input));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		}, [&systemInput](auto&&, auto&&)
		{
			return std::make_unique<StringInputStream>(systemInput);
		} });

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
			#ifndef SYSTEM_H
			#define SYSTEM_H

			#define FOO3			
			int x = 42;

			#endif
		)";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		}, [&systemSource](auto&&, auto&&)
		{
			return std::make_unique<StringInputStream>(systemSource);
		} });

		std::string output = preprocessor.Process();
		REQUIRE(result);
	}

	SECTION("TestProcess_PassSourceWithFunctionMacro_ReturnsProcessedSource")
	{
		std::string inputSource = "#define FOO(X, Y) Foo.getValue(X, Y)\nFOO(42, input.value)";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		} });

		std::cout << preprocessor.Process() << std::endl;
		REQUIRE(result);
	}

	SECTION("TestProcess_PassFloatingPointValue_ReturnsThisValue")
	{
		std::string inputSource = "1.0001 1.00001f vec4(1.0f, 0.2, 0.223, 1.0001f);";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		} });

		auto&& output = preprocessor.Process();
		std::cout << output << std::endl;
		REQUIRE(output == inputSource);
	}

	SECTION("TestProcess_PassFloatingPointValue_ReturnsThisValue2")
	{
		std::string inputSource = "float c = nebula(layer2_coord * 3.0) * 0.35 - 0.05";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		} });

		auto&& output = preprocessor.Process();
		std::cout << output << std::endl;
		REQUIRE(output == inputSource);
	}

	SECTION("TestProcess_PassTwoStringsWithConcatOperation_ReturnsSingleString")
	{
		std::string inputSource = "AAA   ## BB";
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		} });

		std::string str = preprocessor.Process();
		std::cout << str << std::endl;

		REQUIRE((result && (str == "AAABB")));
	}

	SECTION("TestProcess_PassSourceWithFunctionMacro_ReturnsProcessedSource")
	{
		std::string inputSource = "#define FOO(X) \\\nint X; \\\nint X ## _Additional;\nFOO(Test)";
		std::string expectedResult = "int Test;int Test_Additional;";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		} });

		REQUIRE((result && (preprocessor.Process() == expectedResult)));
	}

	SECTION("TestProcess_PassNestedFunctionMacroIntoAnotherFunctionMacro_ReturnsProcessedSource")
	{
		std::string inputSource = "#define FOO(X, Y) X(Y)\nFOO(Foo, Test(0, 0))";
		std::string expectedResult = "Foo(Test(0, 0))";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		} });

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
		
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		} });

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

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		} });

		std::string actualResult = preprocessor.Process();
		REQUIRE((result && (actualResult == expectedResult)));
	}

	SECTION("TestProcess_PassDefineThatSeparatedWithSpaces_ReturnsCorrectProcessedSource")
	{
		std::string inputSource = "#   define Foo";
		
		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		} });

		std::string str = preprocessor.Process();

		/// \note symbol table should contain Foo macro
		REQUIRE(ContainsMacro(preprocessor, "Foo"));
		REQUIRE((result && str.empty()));
	}

	SECTION("TestProcess_PassCodeWithCommentary_ReturnsCorrectProcessedSource")
	{
		std::string inputSource = "A;// Commentary";
		std::string expectedResult = "A;// Commentary";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		} });

		std::string str = preprocessor.Process();

		REQUIRE((result && str == expectedResult));
	}

	SECTION("TestProcess_EvaluateExpressionsInDefines_AllExpressionsShouldBeComputedCorrectly")
	{
		std::string inputSource = R"(
			#define A 1
			#define C 0
			#define FOO(X, Y) (X && Y)
			
			#if A && B
				#define PASSED_0
			#else
				#define FAILED_0
			#endif

			#if A || B
				#define PASSED_1
			#else
				#define FAILED_1
			#endif

			#if !A
				#define PASSED_2
			#else
				#define FAILED_2
			#endif

			#if A + B
				#define PASSED_3
			#else
				#define FAILED_3
			#endif

			#if A - B
				#define PASSED_4
			#else
				#define FAILED_4
			#endif

			#if A * B
				#define PASSED_5
			#else
				#define FAILED_5
			#endif

			#if A / B
				#define PASSED_6
			#else
				#define FAILED_6
			#endif

			#if C
				#define PASSED_7
			#else
				#define FAILED_7
			#endif
)";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		} });

		std::string str = preprocessor.Process();

		REQUIRE(!ContainsMacro(preprocessor, "PASSED_0"));
		REQUIRE(ContainsMacro(preprocessor, "FAILED_0"));

		REQUIRE(ContainsMacro(preprocessor, "PASSED_1"));
		REQUIRE(!ContainsMacro(preprocessor, "FAILED_1"));

		REQUIRE(!ContainsMacro(preprocessor, "PASSED_2"));
		REQUIRE(ContainsMacro(preprocessor, "FAILED_2"));

		REQUIRE(ContainsMacro(preprocessor, "PASSED_3"));
		REQUIRE(!ContainsMacro(preprocessor, "FAILED_3"));

		REQUIRE(ContainsMacro(preprocessor, "PASSED_4"));
		REQUIRE(!ContainsMacro(preprocessor, "FAILED_4"));

		REQUIRE(!ContainsMacro(preprocessor, "PASSED_5"));
		REQUIRE(ContainsMacro(preprocessor, "FAILED_5"));

		REQUIRE(!ContainsMacro(preprocessor, "PASSED_6"));
		REQUIRE(ContainsMacro(preprocessor, "FAILED_6"));

		REQUIRE(!ContainsMacro(preprocessor, "PASSED_7"));
		REQUIRE(ContainsMacro(preprocessor, "FAILED_7"));

		REQUIRE(result);
	}

	SECTION("TestProcess_EvaluateMacroFunctionExpressions_MacroFunctionShouldBeExpandedBeforeEvaluation")
	{
		std::string inputSource = R"(
			#define A 1
			#define AND(X, Y) (X && Y)
			
			#if AND(A, 0)
				#define PASSED
			#else
				#define FAILED
			#endif

			#if AND(A, 1)
				#define PASSED_1
			#else
				#define FAILED_1
			#endif
)";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		} });

		std::string str = preprocessor.Process();

		REQUIRE(!ContainsMacro(preprocessor, "PASSED"));
		REQUIRE(ContainsMacro(preprocessor, "FAILED"));

		REQUIRE(ContainsMacro(preprocessor, "PASSED_1"));
		REQUIRE(!ContainsMacro(preprocessor, "FAILED_1"));

		REQUIRE(result);
	}

	SECTION("TestProcess_PassIncludeDirectiveWithoutNewlineEscapeSequence_DirectiveShouldBeProcessedCorrectly")
	{
		std::string inputSource = "#include <iostream>";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		} });

		std::string str = preprocessor.Process();
	}

	SECTION("TestProcess_PassSourceDangerousCommentary_CorrectlyProcessThatCommentary")
	{
		std::string inputSource = R"(
#ifndef FOO_H
#define FOO_H

/*int foo() {
	return 0 ;//* 42; // this //* sequence can be considered as commentary's beginning
}
*/

#endif
)";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		}, [](auto&&, auto&&)
		{
			return nullptr;
		} });

		std::string output = preprocessor.Process();

		REQUIRE((!output.empty() && output.find("#endif") == std::string::npos));
	}

	SECTION("TestProcess_PassSourceWithCommentPreprocessorSkipsThem_TheOutputDoesntContainComments")
	{
		std::string inputSource = R"(
int main(int argc, char** argv) {
	// TEST COMMENT
	return -1;
}
)";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		}, [](auto&&, auto&&)
		{
			return nullptr;
		}, 
		true });

		std::string output = preprocessor.Process();

		REQUIRE((!output.empty() && output.find("COMMENT") == std::string::npos));
	}

	SECTION("TestProcess_PassMacroIntoFuncMacroWithConcatenation_MacroExpansionIsOmitted")
	{
		std::string inputSource = R"(
#define STRCAT(a, b) a ## b
STRCAT(__LINE__, b)
STRCAT(a, __LINE__)
)";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		}, [](auto&&, auto&&)
		{
			return nullptr;
		},
		true });

		std::string output = preprocessor.Process();

		REQUIRE(output == "\n__LINE__b\na__LINE__\n"); // If an argument is stringized or concatenated, the prescan does not occur and macro is not expanded
	}
#if 0
	SECTION("TestProcess_PassMacroIntoFuncMacroWithinAnotherFuncMacro_MacrosExpanded")
	{
		std::string inputSource = R"(
#define STRCAT(a, b) a##b
#define STRCAT1(a, b) STRCAT(a, b)
STRCAT1(__LINE__, b)
)";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		}, [](auto&&, auto&&)
		{
			return nullptr;
		},
		true });

		std::string output = preprocessor.Process();

		REQUIRE(output == "\n__LINE__3\n"); // If an argument is stringized or concatenated, the prescan does not occur and macro is not expanded
	}
#endif
	SECTION("TestProcess_DefineSelfReferencedMacro_MacroIsExpandedOnlyOnce")
	{
		std::string inputSource = R"(
#define FOO 1 + FOO
FOO
)";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		}, [](auto&&, auto&&)
		{
			return nullptr;
		},
		true });

		std::string output = preprocessor.Process();

		REQUIRE(output == "\n1 + FOO\n"); 
	}

	SECTION("TestProcess_FunctionMacroWithoutInvokation_MacroIsNotExpanded")
	{
		std::string inputSource = R"(
#define FOO(X) X
auto foo = FOO;
)";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		}, [](auto&&, auto&&)
		{
			return nullptr;
		},
		true });

		std::string output = preprocessor.Process();

		REQUIRE(output == "\nauto foo = FOO;\n");
	}

	SECTION("TestProcess_PassCommaInBracketsAsFirstArgumentInMacro_WholeBracketsBlockAssumedAsFirstArgument")
	{
		std::string inputSource = R"(
#define FIRST(X, Y) X
FIRST((1, 2) c, 3)
)";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		}, [](auto&&, auto&&)
		{
			return nullptr;
		},
		true });

		std::string output = preprocessor.Process();
		REQUIRE((result && output == "\n(1, 2) c\n"));
	}
#if 0
	SECTION("TestProcess_StringifyOperatorInvokedOnNonParameterToken_ProcessingErrorOccurs")
	{
		std::string inputSource = R"(
#define TEST(X) #value
TEST(3)
)";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		}, [](auto&&, auto&&)
		{
			return nullptr;
		},
		true });

		preprocessor.Process();
		REQUIRE(!result);
	}
#endif
	SECTION("TestProcess_PassEmptyArg_MacroExpanded")
	{
		std::string inputSource = R"(
#define TEST(X) X
TEST( )
)";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		}, [](auto&&, auto&&)
		{
			return nullptr;
		},
		true });

		preprocessor.Process();
		REQUIRE(result);
	}

	SECTION("TestProcess_PassEmptyArgWithoutSpace_ProcessingErrorOccurs")
	{
		std::string inputSource = R"(
#define TEST(X) X
TEST()
)";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		}, [](auto&&, auto&&)
		{
			return nullptr;
		},
		true });

		preprocessor.Process();
		REQUIRE(!result);
	}

	SECTION("TestProcess_PassDefineExpansionInBrackets_MacroCorrectlyExpanded")
	{
		std::string inputSource = "#define COUNT 4\nint array[COUNT];\n";

		Lexer lexer(std::make_unique<StringInputStream>(inputSource));

		bool result = true;

		Preprocessor preprocessor(lexer, { [&result](auto&& arg)
		{
			std::cerr << "Error: " << ErrorTypeToString(arg.mType) << std::endl;
			result = false;
		}, [](auto&&, auto&&)
		{
			return nullptr;
		},
		true });

		std::string output = preprocessor.Process();
		REQUIRE((result && output == "int array[4];\n"));
	}
}
