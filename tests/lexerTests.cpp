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
		MockInputStream input({ "" });
		Lexer lexer(input);

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithSplittedLines_ReturnsConcatenatedBlobToken")
	{
		MockInputStream input({ "\\ ", " \\" });
		Lexer lexer(input);

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SPACE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithWhitespacesLines_ReturnsSPACEandENDTokens")
	{
		MockInputStream input({ "    ", "  \t " });
		Lexer lexer(input);

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SPACE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::SPACE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithDirectives_ReturnsCorrespondingTokens")
	{
		MockInputStream input({ "#define", "#if", "#else", "#elif", "#include", });
		Lexer lexer(input);

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::DEFINE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IF);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::ELSE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::ELIF);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::INCLUDE);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithIdentifiers_ReturnsIdentifierToken")
	{
		MockInputStream input({ "line", "_macro", "lucky_42" });
		Lexer lexer(input);

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::IDENTIFIER);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithSeparators_ReturnsTheirTokens")
	{
		MockInputStream input({ ",()"});
		Lexer lexer(input);

		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::COMMA);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::OPEN_BRACKET);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::CLOSE_BRACKET);
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestGetNextToken_PassStreamWithLineFeeds_ReturnsNewlineToken")
	{
		MockInputStream input({ "line\n", "_macro\n", "lucky_42" });
		Lexer lexer(input);

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

		MockInputStream input(std::move(keywords));
		Lexer lexer(input);

		for (int i = 0; i < keywordsCount; ++i)
		{
			REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::KEYWORD);
		}
		REQUIRE(lexer.GetNextToken().mType == E_TOKEN_TYPE::END);
	}
}