#include <catch2/catch.hpp>
#include "tcppLibrary.hpp"
#include <string>

using namespace tcpp;


TEST_CASE("TokensOutputStream Tests")
{
	const TTokensSequence tokens
	{
		TToken { E_TOKEN_TYPE::COMMENTARY },
		TToken { E_TOKEN_TYPE::OPEN_BRACKET },
		TToken { E_TOKEN_TYPE::CLOSE_BRACKET },
		TToken { E_TOKEN_TYPE::END },
	};

	TokensOutputStream stream{ tokens };

	SECTION("TestBeginEnd_IterateThroughUsingRangeBasedFor_AllElementsVisited")
	{
		int i = 0;

		for (const TToken& currToken : stream)
		{
			REQUIRE(currToken.mType == tokens[i++].mType);
		}
	}	
	
	SECTION("TestGetNextToken_IterateThroughSequence_AllElementsVisited")
	{
		int i = 0;

		while (stream.HasNextToken())
		{
			REQUIRE(stream.GetNextToken().mType == tokens[i++].mType);
		}
	}

	SECTION("TestGetNextToken_TryToGetNextTokenWhenNoItemsRemain_ReturnsLastElement")
	{
		int i = 0;

		while (stream.HasNextToken())
		{
			REQUIRE(stream.GetNextToken().mType == tokens[i++].mType);
		}

		REQUIRE(!stream.HasNextToken());

		const TToken& outboundsToken = stream.GetNextToken();
		REQUIRE(outboundsToken.mType == E_TOKEN_TYPE::END);
	}

	SECTION("TestPeekNextToken_TryToIterateThroughAllElements_AllElementsVisited")
	{
		size_t i = 0;

		for (const TToken& expectedToken : tokens)
		{
			REQUIRE(expectedToken.mType == stream.PeekNextToken(i++).mType);
		}
	}
}