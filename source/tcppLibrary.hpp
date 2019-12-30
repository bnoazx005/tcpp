/*!
	\file tcppLibrary.hpp
	\date 29.12.2019
	\author Ildar Kasimov

	This file is a single-header library which was written in C++14 standard.
	The main purpose of the library is to implement simple yet flexible C/C++ preprocessor.
	We've hardly tried to stick to C98 preprocessor's specifications, but if you've found 
	some mistakes or inaccuracies, please, make us to know about them. The project has started
	as minimalistic implementation of preprocessor for GLSL and HLSL languages.

	The usage of the library is pretty simple, just copy this file into your enviornment and
	predefine TCPP_IMPLEMENTATION macro before its inclusion like the following
	
	\code 
		#define TCPP_IMPLEMENTATION
		#include "tcppLibrary.hpp"
	\endcode
*/

#pragma once


#include <string>
#include <functional>
#include <vector>
#include <algorithm>
#include <tuple>
#include <unordered_set>


///< Library's configs
#define TCPP_DISABLE_EXCEPTIONS 1


#if TCPP_DISABLE_EXCEPTIONS
	#define TCPP_NOEXCEPT noexcept
#else
	#define TCPP_NOEXCEPT 
#endif

#include <cassert>
#define TCPP_ASSERT(assertion) assert(assertion)


namespace tcpp
{
	/*!
		interface IInputStream

		\brief The interface describes the functionality that all input streams should
		provide
	*/

	class IInputStream
	{
		public:
			IInputStream() TCPP_NOEXCEPT = default;
			virtual ~IInputStream() TCPP_NOEXCEPT = default;

			virtual std::string ReadLine() TCPP_NOEXCEPT = 0;
			virtual bool HasNextLine() const TCPP_NOEXCEPT = 0;
	};


	/*!
		class StringInputStream

		\brief The class is the simplest implementation of the input stream, which
		is a simple string 
	*/

	class StringInputStream : public IInputStream
	{
		public:
			StringInputStream(const std::string& source) TCPP_NOEXCEPT;
			virtual ~StringInputStream() TCPP_NOEXCEPT = default;

			std::string ReadLine() TCPP_NOEXCEPT override;
			bool HasNextLine() const TCPP_NOEXCEPT override;
		private:
			const std::string* mpSourceStr;

			std::string::size_type mPos;
	};


	enum class E_TOKEN_TYPE: unsigned int
	{
		IDENTIFIER,
		DEFINE,
		IF,
		ELSE,
		ELIF,
		UNDEF,
		INCLUDE,
		SPACE,
		BLOB,
		OPEN_BRACKET,
		CLOSE_BRACKET,
		COMMA,
		NEWLINE,
		LESS,
		GREATER, 
		QUOTES,
		KEYWORD,
		END,
		UNKNOWN
	};


	/*!
		struct TToken

		\brief The structure is a type to contain all information about single token
	*/

	typedef struct TToken
	{
		E_TOKEN_TYPE mType;

		std::string mRawView;

		size_t mLineId;
	} TToken, *TTokenPtr;


	/*!
		class Lexer

		\brief The class implements lexer's functionality
	*/

	class Lexer
	{
		public:
			Lexer() TCPP_NOEXCEPT = delete;
			Lexer(IInputStream& inputStream) TCPP_NOEXCEPT;
			~Lexer() TCPP_NOEXCEPT = default;

			TToken GetNextToken() TCPP_NOEXCEPT;

			bool HasNextToken() const TCPP_NOEXCEPT;
		private:
			TToken _scanTokens(std::string& inputLine) const TCPP_NOEXCEPT;

			std::string _removeSingleLineComment(const std::string& line) const TCPP_NOEXCEPT;

			std::string _removeMultiLineComments(const std::string& currInput) TCPP_NOEXCEPT;

			std::string _requestSourceLine() TCPP_NOEXCEPT;
		private:
			static const TToken mEOFToken;

			IInputStream* mpInputStream;

			std::string mCurrLine;

			size_t mCurrLineIndex = 0;
	};


	/*!
		class Preprocessor

		\brief The class implements main functionality of C preprocessor. To preprocess
		some source code string, create an instance of this class and call Process method.
		The usage of the class is the same as iterators.
	*/

	class Preprocessor
	{
		public:
			using TOnErrorCallback = std::function<void()>;
		public:
			Preprocessor() TCPP_NOEXCEPT = delete;
			Preprocessor(const Preprocessor&) TCPP_NOEXCEPT = delete;
			Preprocessor(Lexer& lexer, const TOnErrorCallback& onErrorCallback = {}) TCPP_NOEXCEPT;
			~Preprocessor() TCPP_NOEXCEPT = default;

			std::string Get() TCPP_NOEXCEPT;

			bool HasNext() TCPP_NOEXCEPT;

			bool MoveNext() TCPP_NOEXCEPT;

			Preprocessor& operator= (const Preprocessor&) TCPP_NOEXCEPT = delete;
		private:
			Lexer* mpLexer;
			TOnErrorCallback mOnErrorCallback;
	};


///< implementation of the library is placed below
#if defined(TCPP_IMPLEMENTATION)

	StringInputStream::StringInputStream(const std::string& source) TCPP_NOEXCEPT:
		IInputStream(), mpSourceStr(&source), mPos(0)
	{
	}

	std::string StringInputStream::ReadLine() TCPP_NOEXCEPT
	{
		std::string::size_type prevPos = mPos ? (mPos + 1) : 0;
		mPos = mpSourceStr->find_first_of('\n', prevPos);

		return mpSourceStr->substr(prevPos, (mPos != std::string::npos) ? (mPos - prevPos + 1) : (mpSourceStr->length() - prevPos + 1));
	}

	bool StringInputStream::HasNextLine() const TCPP_NOEXCEPT
	{
		return (mpSourceStr->find_first_of("\n\r", mPos) != std::string::npos);
	}


	const TToken Lexer::mEOFToken = { E_TOKEN_TYPE::END };

	Lexer::Lexer(IInputStream& inputStream) TCPP_NOEXCEPT:
		mpInputStream(&inputStream), mCurrLine(), mCurrLineIndex(0)
	{
	}

	TToken Lexer::GetNextToken() TCPP_NOEXCEPT
	{
		if (mCurrLine.empty())
		{
			// \note if it's still empty then we've reached the end of the source
			if ((mCurrLine = _requestSourceLine()).empty())
			{
				return mEOFToken;
			}
		}

		mCurrLine = _removeMultiLineComments(mCurrLine);
		return _scanTokens(mCurrLine);
	}

	bool Lexer::HasNextToken() const TCPP_NOEXCEPT
	{
		return mpInputStream->HasNextLine() || !mCurrLine.empty();
	}

	TToken Lexer::_scanTokens(std::string& inputLine) const TCPP_NOEXCEPT
	{
		char ch = '\0';

		static const std::vector<std::tuple<std::string, E_TOKEN_TYPE>> directives
		{
			{ "define", E_TOKEN_TYPE::DEFINE },
			{ "if", E_TOKEN_TYPE::IF },
			{ "else", E_TOKEN_TYPE::ELSE },
			{ "elif", E_TOKEN_TYPE::ELIF },
			{ "undef", E_TOKEN_TYPE::UNDEF },
			{ "include", E_TOKEN_TYPE::INCLUDE },
		};

		static const std::unordered_set<std::string> keywordsMap
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

		static const std::string separators = ",()<>\"";

		std::string currStr = "";

		while (!inputLine.empty())
		{
			ch = inputLine.front();

			if (ch == '\n')
			{
				// flush current blob
				if (!currStr.empty())
				{
					return { E_TOKEN_TYPE::BLOB, currStr, mCurrLineIndex };
				}

				inputLine.erase(0, 1);
				return { E_TOKEN_TYPE::NEWLINE, "\n", mCurrLineIndex };
			}

			if (std::isspace(ch))
			{
				// flush current blob
				if (!currStr.empty())
				{
					return { E_TOKEN_TYPE::BLOB, currStr, mCurrLineIndex };
				}

				inputLine.erase(0, 1);
				return { E_TOKEN_TYPE::SPACE, " ", mCurrLineIndex };
			}

			if (ch == '#') // \note it could be # operator or a directive
			{
				// flush current blob
				if (!currStr.empty())
				{
					return { E_TOKEN_TYPE::BLOB, currStr, mCurrLineIndex };
				}

				for (const auto& currDirective : directives)
				{
					auto&& currDirectiveStr = std::get<std::string>(currDirective);

					if (inputLine.rfind(currDirectiveStr, 1) == 1)
					{
						inputLine.erase(0, currDirectiveStr.length() + 1);
						return { std::get<E_TOKEN_TYPE>(currDirective), "", mCurrLineIndex };
					}
				}

				// \note if we've reached this line it's # operator not directive
				TCPP_ASSERT(false);
			}

			if (std::isalpha(ch)) ///< \note try to parse a keyword
			{
				char savedCh = ch;

				std::string keyword;
				std::string::size_type i = 0;

				do
				{
					keyword.push_back(ch);
				} 
				while ((i < inputLine.length()) && std::isalpha(ch = inputLine[++i]));

				if (keywordsMap.find(keyword) != keywordsMap.cend())
				{
					inputLine.erase(0, keyword.length());
					return { E_TOKEN_TYPE::KEYWORD, keyword, mCurrLineIndex };
				}

				ch = savedCh; // restore previous state
			}
			
			if (ch == '_' || std::isalpha(ch)) ///< \note parse identifier
			{
				// flush current blob
				if (!currStr.empty())
				{
					return { E_TOKEN_TYPE::BLOB, currStr, mCurrLineIndex };
				}

				std::string identifier;

				do
				{
					identifier.push_back(ch);
					inputLine.erase(0, 1);
				}
				while (!inputLine.empty() && (std::isalnum(ch = inputLine.front()) || (ch == '_')));

				return { E_TOKEN_TYPE::IDENTIFIER, identifier, mCurrLineIndex };
			}

			inputLine.erase(0, 1);

			if ((separators.find_first_of(ch) != std::string::npos))
			{
				if (!currStr.empty())
				{
					return { E_TOKEN_TYPE::BLOB, currStr, mCurrLineIndex }; // flush current blob
				}

				switch (ch)
				{
					case ',':
						return { E_TOKEN_TYPE::COMMA, ",", mCurrLineIndex };
					case '(':
						return { E_TOKEN_TYPE::OPEN_BRACKET, "(", mCurrLineIndex };
					case ')':
						return { E_TOKEN_TYPE::CLOSE_BRACKET, ")", mCurrLineIndex };
					case '<':
						return { E_TOKEN_TYPE::LESS, "<", mCurrLineIndex };
					case '>':
						return { E_TOKEN_TYPE::GREATER, ">", mCurrLineIndex };
					case '\"':
						return { E_TOKEN_TYPE::QUOTES, "\"", mCurrLineIndex };
				}
			}

			currStr.push_back(ch);
		}

		// flush current blob
		if (!currStr.empty())
		{
			return { E_TOKEN_TYPE::BLOB, currStr, mCurrLineIndex };
		}

		return mEOFToken;
	}

	std::string Lexer::_removeSingleLineComment(const std::string& line) const TCPP_NOEXCEPT
	{
		std::string::size_type pos = line.find("//");
		return (pos == std::string::npos) ? line : line.substr(0, pos);
	}

	std::string Lexer::_removeMultiLineComments(const std::string& currInput) TCPP_NOEXCEPT
	{
		std::string input = currInput;

		// \note here below all states of DFA are placed
		std::function<std::string(std::string&)> enterCommentBlock = [&enterCommentBlock, this](std::string& input)
		{
			input.erase(0, 2); // \note remove /*

			while (input.rfind("*/", 0) != 0 && !input.empty())
			{
				input.erase(0, 1);

				if (input.rfind("/*", 0) == 0)
				{
					input = enterCommentBlock(input);
				}

				if (input.empty())
				{
					input = _requestSourceLine();
				}
			}

			input.erase(0, 2); // \note remove */

			return input;
		};

		std::string::size_type pos = input.find("/*");
		if (pos != std::string::npos)
		{
			std::string restStr = input.substr(pos, std::string::npos);
			return input.substr(0, pos) + enterCommentBlock(restStr);
		}

		return input;
	}

	std::string Lexer::_requestSourceLine() TCPP_NOEXCEPT
	{
		if (!mpInputStream->HasNextLine())
		{
			return "";
		}

		std::string sourceLine = _removeSingleLineComment(mpInputStream->ReadLine());
		++mCurrLineIndex;

		/// \note join lines that were splitted with backslash sign
		std::string::size_type pos = 0;
		while (((pos = sourceLine.find_first_of('\\')) != std::string::npos))
		{
			if (mpInputStream->HasNextLine())
			{
				sourceLine.replace(pos ? (pos - 1) : 0, std::string::npos, _removeSingleLineComment(mpInputStream->ReadLine()));
				++mCurrLineIndex;

				continue;
			}

			sourceLine.erase(sourceLine.begin() + pos, sourceLine.end());
		}

		// remove redundant whitespaces
		{
			bool isPrevChWhitespace = false;
			sourceLine.erase(std::remove_if(sourceLine.begin(), sourceLine.end(), [&isPrevChWhitespace](char ch)
			{
				bool shouldReplace = std::isspace(ch) && isPrevChWhitespace;
				isPrevChWhitespace = std::isspace(ch);
				return shouldReplace;
			}), sourceLine.end());
		}

		return sourceLine;
	}


	Preprocessor::Preprocessor(Lexer& lexer, const std::function<void()>& onErrorCallback) TCPP_NOEXCEPT:
		mpLexer(&lexer), mOnErrorCallback(onErrorCallback)
	{
	}

	std::string Preprocessor::Get() TCPP_NOEXCEPT
	{
		TCPP_ASSERT(mpLexer);
		return mpLexer->GetNextToken().mRawView;
	}

	bool Preprocessor::HasNext() TCPP_NOEXCEPT
	{
		TCPP_ASSERT(mpLexer);
		return mpLexer->HasNextToken();
	}

	bool Preprocessor::MoveNext() TCPP_NOEXCEPT
	{
		return HasNext();
	}

#endif
}