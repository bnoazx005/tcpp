#include <catch2/catch.hpp>
#include "tcppLibrary.hpp"
#include <string>

using namespace tcpp;


TEST_CASE("StringInputStream Tests")
{
	SECTION("TestReadLine_PassEmptyString_ReturnsEmptyString")
	{
		StringInputStream stringInputStream("");
		REQUIRE(!stringInputStream.HasNextLine());
	}

	SECTION("TestReadLine_PassTwoLines_ReturnsEachOfThem")
	{
		std::string lines[]
		{
			"line1\n",
			"line2\r\n",
			"line3"
		};

		std::string concatenatedString;

		for (const auto& currLine : lines)
		{
			concatenatedString.append(currLine);
		}

		StringInputStream stringInputStream(concatenatedString);

		short linesCount = 0;

		while (stringInputStream.HasNextLine())
		{
			REQUIRE(stringInputStream.ReadLine() == lines[linesCount++]);
		}
	}

	SECTION("TestReadLine_PassStringWithoutLines_ReturnsThisLine")
	{
		const std::string& expectedLine = "line without string";
		StringInputStream stringInputStream(expectedLine);
		REQUIRE(stringInputStream.ReadLine() == expectedLine);
	}

	SECTION("TestReadLine_PassComplexString_ReturnsAllItsLines")
	{
		std::string lines[]
		{
			"\n",
			"#define FOO\n",
			"\n",
			"#ifndef FILE_H\n",
			"#define FILE_H\n",
			"\n",
			"#ifdef FOO\n",
			"	#define BAR(x) x\n",
			"#endif\n",
			"\n",
			"#ifdef FOO2\n",
			"	#define BAR(x) x,x\n",
			"#endif\n",
			"\n",
			"#endif\n",
		};

		std::string inputSource;

		for (const auto& currLine : lines)
		{
			inputSource.append(currLine);
		}

		StringInputStream stringInputStream(inputSource);

		for (auto& currLine : lines)
		{
			auto readLine = stringInputStream.ReadLine();
			REQUIRE(readLine == currLine);
		}
	}
}