#include <catch2/catch.hpp>
#include "tcppLibrary.hpp"
#include <vector>
#include <string>

using namespace tcpp;


class MockInputStream final : public IInputStream
{
	public:
		MockInputStream(std::vector<std::string>&& lines) TCPP_NOEXCEPT:
			mLines(std::move(lines)), mCurrLine(0)
		{
		}

		std::string ReadLine() TCPP_NOEXCEPT
		{
			return mLines[mCurrLine++];
		}

		bool HasNextLine() const TCPP_NOEXCEPT
		{
			return mCurrLine + 1 <= mLines.size();
		}
	private:
		std::vector<std::string> mLines;
		size_t mCurrLine;
};


TEST_CASE("Lexer Tests")
{
	SECTION("TestGetNextToken_PassEmptyStream_ReturnsEndToken")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "" }));

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithSplittedLines_ReturnsConcatenatedBlobToken")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "\\ ", " \\" }));

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SPACE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithWhitespacesLines_ReturnsAllSPACEandENDTokens")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "    ", "  \t " }));

		for (size_t i = 0; i < 8; i++)
		{
			REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SPACE);
		}

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithDirectives_ReturnsCorrespondingTokens")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "#define", "#if", "#else", "#elif", "#include", "#endif" }));

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::DEFINE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IF);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::ELSE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::ELIF);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::INCLUDE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::ENDIF);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithIdentifiers_ReturnsIdentifierToken")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "line", "_macro", "lucky_42" }));

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithSeparators_ReturnsTheirTokens")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { ",()<>\"&|+-*/&&||<<>>!<=>===!=" }));

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::COMMA);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::OPEN_BRACKET);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::CLOSE_BRACKET);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::LESS);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::GREATER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::QUOTES);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::AMPERSAND);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::VLINE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::PLUS);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::MINUS);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::STAR);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SLASH);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::AND);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::OR);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::LSHIFT);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::RSHIFT);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NOT);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::LE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::GE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::EQ);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithLineFeeds_ReturnsNewlineToken")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "line\n", "_macro\n", "lucky_42" }));

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NEWLINE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NEWLINE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithKeywords_ReturnsKeywordTokens")
	{
		std::vector<std::string> keywords
		{ 
			"auto", "double", "int", "struct",
			"break", "else", "long", "switch",
			"case", "enum", "register", "typedef",
			"char", "extern", "return", "union",
			"const", "float", "short", "unsigned",
			"continue", "for", "signed", "void",
			"default", "goto", "sizeof", "volatile",
			"do", "if", "static", "while" 
		};

		size_t keywordsCount = keywords.size();

		Lexer lexer(std::make_unique<MockInputStream>(std::move(keywords)));

		for (size_t i = 0; i < keywordsCount; ++i)
		{
			REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::KEYWORD);
		}
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithSimpleMultilineComments_ReturnsSPACEAndENDTokens")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "/*test\n", " this thing skip */ " }));

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::COMMENTARY);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SPACE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithNestedMultilineComments_ReturnsSPACEAndENDTokens")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "/*test\n", " /*\n", " */ /*test*/ this thing skip */ " }));

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::COMMENTARY);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SPACE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithNestedMultilineComments_ReturnsSPACEAndENDTokens")
	{
		// \note without comments the string looks like that "id  id2 "
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "id /*test\n", "\n", "*/ id2", "/*test this thing skip */ " }));

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SPACE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::COMMENTARY);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SPACE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::COMMENTARY);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SPACE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestAppendFront_PassFewTokensToExistingOnes_ReturnsAppendedFirstlyThenRest")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "line", "_macro", "lucky_42" }));

		lexer.AppendFront({ { E_TOKEN_TYPE::BLOB }, { E_TOKEN_TYPE::ELIF } });

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::BLOB);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::ELIF);

		for (short i = 0; i < 3; ++i)
		{
			REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
		}

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestAppendFront_PassFewTokens_ReturnsAllOfThem")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "(2, 3)" }));

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::OPEN_BRACKET);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NUMBER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::COMMA);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SPACE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NUMBER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::CLOSE_BRACKET);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestAppendFront_PassFewTokensToExistingOnes_ReturnsAppendedFirstlyThenRest")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "line\n", "another line\n" }));

		{
			REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
			REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NEWLINE);

			{
				lexer.PushStream(std::make_unique<MockInputStream>(std::vector<std::string> { "(\n", ")\n" }));

				REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::OPEN_BRACKET);
				REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NEWLINE);
				REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::CLOSE_BRACKET);
				REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NEWLINE);

				lexer.PopStream();
			}

			REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
			REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SPACE);
			REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
			REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NEWLINE);

			{
				lexer.PushStream(std::make_unique<MockInputStream>(std::vector<std::string> { "+\n", "#define\n" }));

				REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::PLUS);
				REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NEWLINE);
				REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::DEFINE);
				REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NEWLINE);

				lexer.PopStream();
			}
		}

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithStringificationOperators_ReturnsCorrespondingTokens")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "# ID", "#ID", "##" }));

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::STRINGIZE_OP);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::STRINGIZE_OP);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::CONCAT_OP);

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassNumbersInDifferentRadixes_ReturnsCorrectTokens")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "42", "0x42", "042" }));

		for (short i = 0; i < 3; ++i)
		{
			REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NUMBER);
		}

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithKeywordLikeIdentifier_ReturnsIdentifierToken")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "float4x4" }));

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithFloatingPointNumbers_ReturnsCorrectTokensSequence")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "1.0001 1.00001f" }));

		// \note For now we don't actually recognize floating-point numbers just process them as blobs
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NUMBER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::BLOB);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NUMBER);

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SPACE);

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NUMBER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::BLOB);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NUMBER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER); 

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassTwoStringsWithConcapOp_ReturnsCorrectTokensSequence")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "AAA   ## BB" }));

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);

		for (short i = 0; i < 3; ++i)
		{
			REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SPACE);
		}

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::CONCAT_OP);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SPACE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassSomeCodeThatEndsWithCommentary_ReturnsCorrectTokensSequence")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "A;// comment" }));

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SEMICOLON);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::COMMENTARY);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestPeekNextToken_IterateOverSequenceUsingOffset_CorrectlyProcessStreamAndReturnsTokens")
	{
		Lexer lexer(std::make_unique<MockInputStream>(std::vector<std::string> { "(2, 3)" }));

		REQUIRE(lexer.PeekNextToken(0).mType == E_TOKEN_TYPE::OPEN_BRACKET); // PeekNextToken(0) equals to GetNextToken()
		REQUIRE(lexer.PeekNextToken(1).mType == E_TOKEN_TYPE::NUMBER);
		REQUIRE(lexer.PeekNextToken(2).mType == E_TOKEN_TYPE::COMMA);
		REQUIRE(lexer.PeekNextToken(3).mType == E_TOKEN_TYPE::SPACE);
		REQUIRE(lexer.PeekNextToken(4).mType == E_TOKEN_TYPE::NUMBER);
		REQUIRE(lexer.PeekNextToken(5).mType == E_TOKEN_TYPE::CLOSE_BRACKET);
		REQUIRE(lexer.PeekNextToken(6).mType == E_TOKEN_TYPE::END);

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::NUMBER);
	}
}