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

	Because of the initial goal of the project wasn't full featured C preprocessor but its subset
	that's useful for GLSL and HLSL, some points from the specification of the C preprocessor could
	not be applied to this library, at least for now. There is a list of unimplemented features placed
	below

	\todo Implement support of char literals
	\todo Improve existing performance for massive input files
	\todo Add support of integral literals like L, u, etc
	\todo Implement built-in directives like #pragma, #error and others
	\todo Provide support of variadic macros
*/

#pragma once


#include <string>
#include <functional>
#include <vector>
#include <list>
#include <algorithm>
#include <tuple>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <cctype>


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
			StringInputStream() TCPP_NOEXCEPT = delete;
			explicit StringInputStream(const std::string& source) TCPP_NOEXCEPT;
			StringInputStream(const StringInputStream& inputStream) TCPP_NOEXCEPT;
			StringInputStream(StringInputStream&& inputStream) TCPP_NOEXCEPT;
			virtual ~StringInputStream() TCPP_NOEXCEPT = default;

			std::string ReadLine() TCPP_NOEXCEPT override;
			bool HasNextLine() const TCPP_NOEXCEPT override;
		private:
			std::string mSourceStr;
	};


	enum class E_TOKEN_TYPE : unsigned int
	{
		IDENTIFIER,
		DEFINE,
		IF,
		ELSE,
		ELIF,
		UNDEF,
		ENDIF,
		INCLUDE,
		DEFINED,
		IFNDEF,
		IFDEF,
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
		REJECT_MACRO, ///< Special type of a token to provide meta information 
		STRINGIZE_OP,
		CONCAT_OP,
		NUMBER,
		PLUS,
		MINUS,
		SLASH,
		STAR,
		OR,
		AND,
		AMPERSAND,
		VLINE,
		LSHIFT,
		RSHIFT,
		NOT,
		GE,
		LE,
		EQ,
		NE,
		CUSTOM_DIRECTIVE,
		UNKNOWN,
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
		size_t mPos;
	} TToken, *TTokenPtr;


	/*!
		class Lexer

		\brief The class implements lexer's functionality
	*/

	class Lexer
	{
		private:
			using TTokensQueue = std::list<TToken>;
			using TStreamStack = std::stack<IInputStream*>;
			using TDirectivesMap = std::vector<std::tuple<std::string, E_TOKEN_TYPE>>;
			using TDirectiveHandlersArray = std::unordered_set<std::string>;
		public:
			Lexer() TCPP_NOEXCEPT = delete;
			explicit Lexer(IInputStream& inputStream) TCPP_NOEXCEPT;
			~Lexer() TCPP_NOEXCEPT = default;

			bool AddCustomDirective(const std::string& directive) TCPP_NOEXCEPT; 

			TToken GetNextToken() TCPP_NOEXCEPT;

			bool HasNextToken() const TCPP_NOEXCEPT;

			void AppendFront(const std::vector<TToken>& tokens) TCPP_NOEXCEPT;

			void PushStream(IInputStream& stream) TCPP_NOEXCEPT;
			void PopStream() TCPP_NOEXCEPT;

			size_t GetCurrLineIndex() const TCPP_NOEXCEPT;
			size_t GetCurrPos() const TCPP_NOEXCEPT;
		private:
			TToken _scanTokens(std::string& inputLine) TCPP_NOEXCEPT;

			std::string _removeSingleLineComment(const std::string& line) const TCPP_NOEXCEPT;

			std::string _removeMultiLineComments(const std::string& currInput) TCPP_NOEXCEPT;

			std::string _requestSourceLine() TCPP_NOEXCEPT;

			TToken _scanSeparatorTokens(char ch, std::string& inputLine) TCPP_NOEXCEPT;

			IInputStream* _getActiveStream() const TCPP_NOEXCEPT;
		private:
			static const TToken mEOFToken;

			TDirectivesMap mDirectivesTable;

			TTokensQueue mTokensQueue;

			std::string mCurrLine;

			size_t mCurrLineIndex = 0;
			size_t mCurrPos = 0;

			TStreamStack mStreamsContext;
			
			TDirectiveHandlersArray mCustomDirectivesMap;
	};


	/*!
		struct TMacroDesc

		\brief The type describes a single macro definition's description
	*/

	typedef struct TMacroDesc
	{
		std::string mName;

		std::vector<std::string> mArgsNames;

		std::vector<TToken> mValue;
	} TMacroDesc, *TMacroDescPtr;


	enum class E_ERROR_TYPE : unsigned int
	{
		UNEXPECTED_TOKEN,
		UNBALANCED_ENDIF,
		INVALID_MACRO_DEFINITION,
		MACRO_ALREADY_DEFINED,
		INCONSISTENT_MACRO_ARITY,
		UNDEFINED_MACRO,
		INVALID_INCLUDE_DIRECTIVE,
		UNEXPECTED_END_OF_INCLUDE_PATH,
		ANOTHER_ELSE_BLOCK_FOUND,
		ELIF_BLOCK_AFTER_ELSE_FOUND,
		UNDEFINED_DIRECTIVE,
	};


	std::string ErrorTypeToString(const E_ERROR_TYPE& errorType) TCPP_NOEXCEPT;


	/*!
		struct TErrorInfo

		\brief The type contains all information about happened error
	*/

	typedef struct TErrorInfo
	{
		E_ERROR_TYPE mType;
		size_t mLine;
	} TErrorInfo, *TErrorInfoPtr;


	/*!
		class Preprocessor

		\brief The class implements main functionality of C preprocessor. To preprocess
		some source code string, create an instance of this class and call Process method.
		The usage of the class is the same as iterators.
	*/

	class Preprocessor
	{
		public:
			using TOnErrorCallback = std::function<void(const TErrorInfo&)>;
			using TOnIncludeCallback = std::function<IInputStream*(const std::string&, bool)>;
			using TSymTable = std::vector<TMacroDesc>;
			using TContextStack = std::list<std::string>;
			using TDirectiveHandler = std::function<std::string(Preprocessor&, Lexer&, const std::string&)>;
			using TDirectivesMap = std::unordered_map<std::string, TDirectiveHandler>;

			typedef struct TIfStackEntry
			{
				bool mShouldBeSkipped = true;
				bool mHasElseBeenFound = false;

				TIfStackEntry(bool shouldBeSkipped) : mShouldBeSkipped(shouldBeSkipped), mHasElseBeenFound(false) {}
			} TIfStackEntry, *TIfStackEntryPtr;

			using TIfStack = std::stack<TIfStackEntry>;
		public:
			Preprocessor() TCPP_NOEXCEPT = delete;
			Preprocessor(const Preprocessor&) TCPP_NOEXCEPT = delete;
			Preprocessor(Lexer& lexer, const TOnErrorCallback& onErrorCallback = {}, const TOnIncludeCallback& onIncludeCallback = {}) TCPP_NOEXCEPT;
			~Preprocessor() TCPP_NOEXCEPT = default;

			bool AddCustomDirectiveHandler(const std::string& directive, const TDirectiveHandler& handler) TCPP_NOEXCEPT;

			std::string Process() TCPP_NOEXCEPT;

			Preprocessor& operator= (const Preprocessor&) TCPP_NOEXCEPT = delete;

			TSymTable GetSymbolsTable() const TCPP_NOEXCEPT;
		private:
			void _createMacroDefinition() TCPP_NOEXCEPT;
			void _removeMacroDefinition(const std::string& macroName) TCPP_NOEXCEPT;

			std::vector<TToken> _expandMacroDefinition(const TMacroDesc& macroDesc, const TToken& idToken) TCPP_NOEXCEPT;

			void _expect(const E_TOKEN_TYPE& expectedType, const E_TOKEN_TYPE& actualType) const TCPP_NOEXCEPT;

			void _processInclusion() TCPP_NOEXCEPT;

			TIfStackEntry _processIfConditional() TCPP_NOEXCEPT;
			TIfStackEntry _processIfdefConditional() TCPP_NOEXCEPT;
			TIfStackEntry _processIfndefConditional() TCPP_NOEXCEPT;
			void _processElseConditional(TIfStackEntry& currStackEntry) TCPP_NOEXCEPT;
			void _processElifConditional(TIfStackEntry& currStackEntry) TCPP_NOEXCEPT;

			int _evaluateExpression(const std::vector<TToken>& exprTokens) const TCPP_NOEXCEPT;

			bool _shouldTokenBeSkipped() const TCPP_NOEXCEPT;
		private:
			Lexer* mpLexer;
			TOnErrorCallback mOnErrorCallback;
			TOnIncludeCallback mOnIncludeCallback;
			TSymTable mSymTable;
			TContextStack mContextStack;
			TIfStack mConditionalBlocksStack;
			TDirectivesMap mCustomDirectivesHandlersMap;
	};


	///< implementation of the library is placed below
#if defined(TCPP_IMPLEMENTATION)


	std::string ErrorTypeToString(const E_ERROR_TYPE& errorType) TCPP_NOEXCEPT
	{
		switch (errorType)
		{
			case E_ERROR_TYPE::UNEXPECTED_TOKEN:
				return "Unexpected token";
			case E_ERROR_TYPE::UNBALANCED_ENDIF:
				return "Unbalanced endif";
			case E_ERROR_TYPE::INVALID_MACRO_DEFINITION:
				return "Invalid macro definition";
			case E_ERROR_TYPE::MACRO_ALREADY_DEFINED:
				return "The macro is already defined";
			case E_ERROR_TYPE::INCONSISTENT_MACRO_ARITY:
				return "Inconsistent number of arguments between definition and invocation of the macro";
			case E_ERROR_TYPE::UNDEFINED_MACRO:
				return "Undefined macro";
			case E_ERROR_TYPE::INVALID_INCLUDE_DIRECTIVE:
				return "Invalid #include directive";
			case E_ERROR_TYPE::UNEXPECTED_END_OF_INCLUDE_PATH:
				return "Unexpected end of include path";
			case E_ERROR_TYPE::ANOTHER_ELSE_BLOCK_FOUND:
				return "#else directive should be last one";
			case E_ERROR_TYPE::ELIF_BLOCK_AFTER_ELSE_FOUND:
				return "#elif found after #else block";
			case E_ERROR_TYPE::UNDEFINED_DIRECTIVE:
				return "Undefined directive";
		}

		return "";
	}


	StringInputStream::StringInputStream(const std::string& source) TCPP_NOEXCEPT:
		IInputStream(), mSourceStr(source)
	{
	}

	StringInputStream::StringInputStream(const StringInputStream& inputStream) TCPP_NOEXCEPT:
		mSourceStr(inputStream.mSourceStr)
	{
	}
	
	StringInputStream::StringInputStream(StringInputStream&& inputStream) TCPP_NOEXCEPT:
		mSourceStr(std::move(inputStream.mSourceStr))
	{
	}

	std::string StringInputStream::ReadLine() TCPP_NOEXCEPT
	{
		std::string::size_type pos = mSourceStr.find_first_of('\n');
		pos = (pos == std::string::npos) ? pos : (pos + 1);

		std::string currLine = mSourceStr.substr(0, pos);
		mSourceStr.erase(0, pos);

		return currLine;
	}

	bool StringInputStream::HasNextLine() const TCPP_NOEXCEPT
	{
		return !mSourceStr.empty();
	}


	const TToken Lexer::mEOFToken = { E_TOKEN_TYPE::END };

	Lexer::Lexer(IInputStream& inputStream) TCPP_NOEXCEPT:
		mDirectivesTable
		{
			{ "define", E_TOKEN_TYPE::DEFINE },
			{ "ifdef", E_TOKEN_TYPE::IFDEF },
			{ "ifndef", E_TOKEN_TYPE::IFNDEF },
			{ "if", E_TOKEN_TYPE::IF },
			{ "else", E_TOKEN_TYPE::ELSE },
			{ "elif", E_TOKEN_TYPE::ELIF },
			{ "undef", E_TOKEN_TYPE::UNDEF },
			{ "endif", E_TOKEN_TYPE::ENDIF },
			{ "include", E_TOKEN_TYPE::INCLUDE },
			{ "defined", E_TOKEN_TYPE::DEFINED },
		}, mCurrLine(), mCurrLineIndex(0)
	{
		PushStream(inputStream);
	}

	bool Lexer::AddCustomDirective(const std::string& directive) TCPP_NOEXCEPT
	{
		if (mCustomDirectivesMap.find(directive) != mCustomDirectivesMap.cend())
		{
			return false;
		}

		mCustomDirectivesMap.insert(directive);
		return true;
	}

	TToken Lexer::GetNextToken() TCPP_NOEXCEPT
	{
		if (!mTokensQueue.empty())
		{
			auto currToken = mTokensQueue.front();
			mTokensQueue.pop_front();

			return currToken;
		}

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
		IInputStream* pCurrInputStream = _getActiveStream();
		return (pCurrInputStream ? pCurrInputStream->HasNextLine() : false) || !mCurrLine.empty() || !mTokensQueue.empty();
	}

	void Lexer::AppendFront(const std::vector<TToken>& tokens) TCPP_NOEXCEPT
	{
		mTokensQueue.insert(mTokensQueue.begin(), tokens.begin(), tokens.end());
	}

	void Lexer::PushStream(IInputStream& stream) TCPP_NOEXCEPT
	{
		mStreamsContext.push(&stream);
	}

	void Lexer::PopStream() TCPP_NOEXCEPT
	{
		if (mStreamsContext.empty())
		{
			return;
		}

		mStreamsContext.pop();
	}

	size_t Lexer::GetCurrLineIndex() const TCPP_NOEXCEPT
	{
		return mCurrLineIndex;
	}

	size_t Lexer::GetCurrPos() const TCPP_NOEXCEPT
	{
		return mCurrPos;
	}

	TToken Lexer::_scanTokens(std::string& inputLine) TCPP_NOEXCEPT
	{
		char ch = '\0';

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

		static const std::string separators = ",()<>\"+-*/&|!=";

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
				++mCurrPos;
				return { E_TOKEN_TYPE::NEWLINE, "\n", mCurrLineIndex, mCurrPos };
			}

			if (std::isspace(ch))
			{
				// flush current blob
				if (!currStr.empty())
				{
					return { E_TOKEN_TYPE::BLOB, currStr, mCurrLineIndex, mCurrPos };
				}

				inputLine.erase(0, 1);
				++mCurrPos;
				return { E_TOKEN_TYPE::SPACE, " ", mCurrLineIndex, mCurrPos };
			}

			if (ch == '#') // \note it could be # operator or a directive
			{
				// flush current blob
				if (!currStr.empty())
				{
					return { E_TOKEN_TYPE::BLOB, currStr, mCurrLineIndex, mCurrPos };
				}

				for (const auto& currDirective : mDirectivesTable)
				{
					auto&& currDirectiveStr = std::get<std::string>(currDirective);

					if (inputLine.rfind(currDirectiveStr, 1) == 1)
					{
						inputLine.erase(0, currDirectiveStr.length() + 1);
						mCurrPos += currDirectiveStr.length() + 1;

						return { std::get<E_TOKEN_TYPE>(currDirective), "", mCurrLineIndex, mCurrPos };
					}
				}

				// \note custom directives
				for (const auto& currDirectiveStr : mCustomDirectivesMap)
				{
					if (inputLine.rfind(currDirectiveStr, 1) == 1)
					{
						inputLine.erase(0, currDirectiveStr.length() + 1);
						mCurrPos += currDirectiveStr.length() + 1;

						return { E_TOKEN_TYPE::CUSTOM_DIRECTIVE, currDirectiveStr, mCurrLineIndex, mCurrPos };
					}
				}

				inputLine.erase(0, 1);

				// \note if we've reached this line it's # operator not directive
				if (!inputLine.empty())
				{
					char nextCh = inputLine.front();
					switch (nextCh)
					{
						case '#':	// \note concatenation operator
							inputLine.erase(0, 1);
							++mCurrPos;
							return { E_TOKEN_TYPE::CONCAT_OP, "", mCurrLineIndex, mCurrPos };
						default:
							if (nextCh != ' ') // \note stringification operator
							{
								return { E_TOKEN_TYPE::STRINGIZE_OP, "", mCurrLineIndex, mCurrPos };
							}

							return { E_TOKEN_TYPE::BLOB, "#", mCurrLineIndex, mCurrPos };
					}
				}
			}

			if (std::isdigit(ch))
			{
				// flush current blob
				if (!currStr.empty())
				{
					return { E_TOKEN_TYPE::BLOB, currStr, mCurrLineIndex, mCurrPos };
				}

				std::string number;
				std::string::size_type i = 0;

				if (ch == '0' && !inputLine.empty())
				{
					inputLine.erase(0, 1);
					++mCurrPos;

					number.push_back(ch);

					char nextCh = inputLine.front();
					if (nextCh == 'x' || std::isdigit(nextCh))
					{
						inputLine.erase(0, 1);
						++mCurrPos;

						number.push_back(nextCh);
					}
					else
					{
						return { E_TOKEN_TYPE::NUMBER, number, mCurrLineIndex, mCurrPos };
					}
				}

				uint8_t charsToRemove = 0;

				while ((i < inputLine.length()) && std::isdigit(ch = inputLine[i++]))
				{
					number.push_back(ch);
					++charsToRemove;
				}

				inputLine.erase(0, charsToRemove);
				mCurrPos += charsToRemove;

				return { E_TOKEN_TYPE::NUMBER, number, mCurrLineIndex, mCurrPos };
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
					++mCurrPos;
				} while (!inputLine.empty() && (std::isalnum(ch = inputLine.front()) || (ch == '_')));

				return { (keywordsMap.find(identifier) != keywordsMap.cend()) ? E_TOKEN_TYPE::KEYWORD : E_TOKEN_TYPE::IDENTIFIER, identifier, mCurrLineIndex, mCurrPos };
			}

			inputLine.erase(0, 1);
			++mCurrPos;

			if ((separators.find_first_of(ch) != std::string::npos))
			{
				if (!currStr.empty())
				{
					auto separatingToken = _scanSeparatorTokens(ch, inputLine);
					if (separatingToken.mType != mEOFToken.mType)
					{
						mTokensQueue.push_front(separatingToken);
					}

					return { E_TOKEN_TYPE::BLOB, currStr, mCurrLineIndex, mCurrPos }; // flush current blob
				}

				auto separatingToken = _scanSeparatorTokens(ch, inputLine);
				if (separatingToken.mType != mEOFToken.mType)
				{
					return separatingToken;
				}
			}

			currStr.push_back(ch);
		}

		// flush current blob
		if (!currStr.empty())
		{
			return { E_TOKEN_TYPE::BLOB, currStr, mCurrLineIndex, mCurrPos };
		}

		PopStream();

		//\note try to continue preprocessing if there is at least one input stream
		if (!mStreamsContext.empty())
		{
			return GetNextToken();
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
		IInputStream* pCurrInputStream = _getActiveStream();

		if (!pCurrInputStream->HasNextLine())
		{
			return "";
		}

		std::string sourceLine = _removeSingleLineComment(pCurrInputStream->ReadLine());
		++mCurrLineIndex;

		/// \note join lines that were splitted with backslash sign
		std::string::size_type pos = 0;
		while (((pos = sourceLine.find_first_of('\\')) != std::string::npos))
		{
			if (pCurrInputStream->HasNextLine())
			{
				sourceLine.replace(pos ? (pos - 1) : 0, std::string::npos, _removeSingleLineComment(pCurrInputStream->ReadLine()));
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
				bool shouldReplace = (ch == ' ' || ch == '\t') && isPrevChWhitespace;
				isPrevChWhitespace = (ch == ' ' || ch == '\t');
				return shouldReplace;
			}), sourceLine.end());
		}

		return sourceLine;
	}

	TToken Lexer::_scanSeparatorTokens(char ch, std::string& inputLine) TCPP_NOEXCEPT
	{
		switch (ch)
		{
			case ',':
				return { E_TOKEN_TYPE::COMMA, ",", mCurrLineIndex, mCurrPos };
			case '(':
				return { E_TOKEN_TYPE::OPEN_BRACKET, "(", mCurrLineIndex, mCurrPos };
			case ')':
				return { E_TOKEN_TYPE::CLOSE_BRACKET, ")", mCurrLineIndex, mCurrPos };
			case '<':
				if (!inputLine.empty())
				{
					char nextCh = inputLine.front();
					switch (nextCh)
					{
						case '<':
							inputLine.erase(0, 1);
							++mCurrPos;
							return { E_TOKEN_TYPE::LSHIFT, "<<", mCurrLineIndex, mCurrPos };
						case '=':
							inputLine.erase(0, 1);
							++mCurrPos;
							return { E_TOKEN_TYPE::LE, "<=", mCurrLineIndex, mCurrPos };
					}					
				}

				return { E_TOKEN_TYPE::LESS, "<", mCurrLineIndex, mCurrPos };
			case '>':
				if (!inputLine.empty())
				{
					char nextCh = inputLine.front();
					switch (nextCh)
					{
						case '>':
							inputLine.erase(0, 1);
							++mCurrPos;
							return { E_TOKEN_TYPE::RSHIFT, ">>", mCurrLineIndex, mCurrPos };
						case '=':
							inputLine.erase(0, 1);
							++mCurrPos;
							return { E_TOKEN_TYPE::GE, ">=", mCurrLineIndex, mCurrPos };
					}
				}

				return { E_TOKEN_TYPE::GREATER, ">", mCurrLineIndex, mCurrPos };
			case '\"':
				return { E_TOKEN_TYPE::QUOTES, "\"", mCurrLineIndex, mCurrPos };
			case '+':
				return { E_TOKEN_TYPE::PLUS, "+", mCurrLineIndex, mCurrPos };
			case '-':
				return { E_TOKEN_TYPE::MINUS, "-", mCurrLineIndex, mCurrPos };
			case '*':
				return { E_TOKEN_TYPE::STAR, "*", mCurrLineIndex, mCurrPos };
			case '/':
				return { E_TOKEN_TYPE::SLASH, "/", mCurrLineIndex, mCurrPos };
			case '&':
				if (!inputLine.empty() && inputLine.front() == '&')
				{
					inputLine.erase(0, 1);
					++mCurrPos;
					return { E_TOKEN_TYPE::AND, "&&", mCurrLineIndex, mCurrPos };
				}

				return { E_TOKEN_TYPE::AMPERSAND, "&", mCurrLineIndex, mCurrPos };
			case '|':
				if (!inputLine.empty() && inputLine.front() == '|')
				{
					inputLine.erase(0, 1);
					++mCurrPos;
					return { E_TOKEN_TYPE::OR, "||", mCurrLineIndex, mCurrPos };
				}

				return { E_TOKEN_TYPE::VLINE, "|", mCurrLineIndex, mCurrPos };
			case '!':
				if (!inputLine.empty() && inputLine.front() == '=')
				{
					inputLine.erase(0, 1);
					++mCurrPos;
					return { E_TOKEN_TYPE::NE, "!=", mCurrLineIndex, mCurrPos };
				}

				return { E_TOKEN_TYPE::NOT, "!", mCurrLineIndex, mCurrPos };
			case '=':
				if (!inputLine.empty() && inputLine.front() == '=')
				{
					inputLine.erase(0, 1);
					++mCurrPos;
					return { E_TOKEN_TYPE::EQ, "==", mCurrLineIndex, mCurrPos };
				}

				return { E_TOKEN_TYPE::BLOB, "=", mCurrLineIndex, mCurrPos };
		}

		return mEOFToken;
	}

	IInputStream* Lexer::_getActiveStream() const TCPP_NOEXCEPT
	{
		return mStreamsContext.empty() ? nullptr : mStreamsContext.top();
	}


	Preprocessor::Preprocessor(Lexer& lexer, const TOnErrorCallback& onErrorCallback, const TOnIncludeCallback& onIncludeCallback) TCPP_NOEXCEPT:
		mpLexer(&lexer), mOnErrorCallback(onErrorCallback), mOnIncludeCallback(onIncludeCallback)
	{
		mSymTable.push_back({ "__LINE__" });
	}

	bool Preprocessor::AddCustomDirectiveHandler(const std::string& directive, const TDirectiveHandler& handler) TCPP_NOEXCEPT
	{
		if ((mCustomDirectivesHandlersMap.find(directive) != mCustomDirectivesHandlersMap.cend()) ||
			!mpLexer->AddCustomDirective(directive))
		{
			return false;
		}

		mCustomDirectivesHandlersMap.insert({ directive, handler });

		return true;
	}

	std::string Preprocessor::Process() TCPP_NOEXCEPT
	{
		TCPP_ASSERT(mpLexer);

		std::string processedStr;

		auto appendString = [&processedStr, this](const std::string& str)
		{
			if (_shouldTokenBeSkipped())
			{
				return;
			}

			processedStr.append(str);
		};

		// \note first stage of preprocessing, expand macros and include directives
		while (mpLexer->HasNextToken())
		{
			auto currToken = mpLexer->GetNextToken();

			switch (currToken.mType)
			{
				case E_TOKEN_TYPE::DEFINE:
					_createMacroDefinition();
					break;
				case E_TOKEN_TYPE::UNDEF:
					currToken = mpLexer->GetNextToken();
					_expect(E_TOKEN_TYPE::IDENTIFIER, currToken.mType);
					_removeMacroDefinition(currToken.mRawView);
					break;
				case E_TOKEN_TYPE::IF:
					mConditionalBlocksStack.push(_processIfConditional());
					break;
				case E_TOKEN_TYPE::IFNDEF:
					mConditionalBlocksStack.push(_processIfndefConditional());
					break;
				case E_TOKEN_TYPE::IFDEF:
					mConditionalBlocksStack.push(_processIfdefConditional());
					break;
				case E_TOKEN_TYPE::ELIF:
					_processElifConditional(mConditionalBlocksStack.top());
					break;
				case E_TOKEN_TYPE::ELSE:
					_processElseConditional(mConditionalBlocksStack.top());
					break;
				case E_TOKEN_TYPE::ENDIF:
					if (mConditionalBlocksStack.empty())
					{
						mOnErrorCallback({ E_ERROR_TYPE::UNBALANCED_ENDIF, mpLexer->GetCurrLineIndex() });
					}
					else
					{
						mConditionalBlocksStack.pop();
					}
					break;
				case E_TOKEN_TYPE::INCLUDE:
					_processInclusion();
					break;
				case E_TOKEN_TYPE::IDENTIFIER: // \note try to expand some macro here
					{
						auto iter = std::find_if(mSymTable.cbegin(), mSymTable.cend(), [&currToken](auto&& item)
						{
							return item.mName == currToken.mRawView;
						});

						auto contextIter = std::find_if(mContextStack.cbegin(), mContextStack.cend(), [&currToken](auto&& item)
						{
							return item == currToken.mRawView;
						});

						if (iter != mSymTable.cend() && contextIter == mContextStack.cend())
						{
							mpLexer->AppendFront(_expandMacroDefinition(*iter, currToken));
						}
						else
						{
							appendString(currToken.mRawView);
						}
					}
					break;
				case E_TOKEN_TYPE::REJECT_MACRO:
					mContextStack.erase(std::remove_if(mContextStack.begin(), mContextStack.end(), [&currToken](auto&& item)
					{
						return item == currToken.mRawView;
					}), mContextStack.end());
					break;
				case E_TOKEN_TYPE::CONCAT_OP:
					if (processedStr.back() == ' ') // \note Remove last character in the processed source if it was a whitespace
					{
						processedStr.erase(processedStr.length() - 1);
					}

					while ((currToken = mpLexer->GetNextToken()).mType == E_TOKEN_TYPE::SPACE); // \note skip space tokens

					appendString(currToken.mRawView);
					break;
				case E_TOKEN_TYPE::STRINGIZE_OP:
					appendString((currToken = mpLexer->GetNextToken()).mRawView);
					break;
				case E_TOKEN_TYPE::CUSTOM_DIRECTIVE:
					{
						auto customDirectiveIter = mCustomDirectivesHandlersMap.find(currToken.mRawView);
						if (customDirectiveIter != mCustomDirectivesHandlersMap.cend())
						{
							appendString(customDirectiveIter->second(*this, *mpLexer, processedStr));
						}
						else
						{
							mOnErrorCallback({ E_ERROR_TYPE::UNDEFINED_DIRECTIVE, mpLexer->GetCurrLineIndex() });
						}
					}
					break;
				default:
					appendString(currToken.mRawView);
					break;
			}

			if (!mpLexer->HasNextToken())
			{
				mpLexer->PopStream();
			}
		}

		return processedStr;
	}
	
	Preprocessor::TSymTable Preprocessor::GetSymbolsTable() const TCPP_NOEXCEPT
	{
		return mSymTable;
	}

	void Preprocessor::_createMacroDefinition() TCPP_NOEXCEPT
	{
		TMacroDesc macroDesc;

		auto currToken = mpLexer->GetNextToken();
		_expect(E_TOKEN_TYPE::SPACE, currToken.mType);

		currToken = mpLexer->GetNextToken();
		_expect(E_TOKEN_TYPE::IDENTIFIER, currToken.mType);

		macroDesc.mName = currToken.mRawView;

		auto extractValue = [this](TMacroDesc& desc, Lexer& lexer)
		{
			TToken currToken;

			while ((currToken = mpLexer->GetNextToken()).mType == E_TOKEN_TYPE::SPACE); // \note skip space tokens

			desc.mValue.push_back(currToken);

			while ((currToken = lexer.GetNextToken()).mType != E_TOKEN_TYPE::NEWLINE)
			{
				desc.mValue.push_back(currToken);
			}

			if (desc.mValue.empty())
			{
				desc.mValue.push_back({ E_TOKEN_TYPE::NUMBER, "1", mpLexer->GetCurrLineIndex() });
			}

			_expect(E_TOKEN_TYPE::NEWLINE, currToken.mType);
		};

		currToken = mpLexer->GetNextToken();
		switch (currToken.mType)
		{
			case E_TOKEN_TYPE::SPACE:	// object like macro
				extractValue(macroDesc, *mpLexer);
				break;
			case E_TOKEN_TYPE::NEWLINE:
				macroDesc.mValue.push_back({ E_TOKEN_TYPE::NUMBER, "1", mpLexer->GetCurrLineIndex() });
				break;
			case E_TOKEN_TYPE::OPEN_BRACKET: // function line macro
				{
					// \note parse arguments
					while (true)
					{
						while ((currToken = mpLexer->GetNextToken()).mType == E_TOKEN_TYPE::SPACE); // \note skip space tokens

						_expect(E_TOKEN_TYPE::IDENTIFIER, currToken.mType);
						macroDesc.mArgsNames.push_back(currToken.mRawView);

						while ((currToken = mpLexer->GetNextToken()).mType == E_TOKEN_TYPE::SPACE);
						if (currToken.mType == E_TOKEN_TYPE::CLOSE_BRACKET)
						{
							break;
						}

						_expect(E_TOKEN_TYPE::COMMA, currToken.mType);
					}
					
					// \note parse macro's value
					extractValue(macroDesc, *mpLexer);
				}
				break;
			default:
				mOnErrorCallback({ E_ERROR_TYPE::INVALID_MACRO_DEFINITION, mpLexer->GetCurrLineIndex() });
				break;
		}
		
		if (_shouldTokenBeSkipped())
		{
			return;
		}

		if (std::find_if(mSymTable.cbegin(), mSymTable.cend(), [&macroDesc](auto&& item) { return item.mName == macroDesc.mName; }) != mSymTable.cend())
		{
			mOnErrorCallback({ E_ERROR_TYPE::MACRO_ALREADY_DEFINED, mpLexer->GetCurrLineIndex() });
			return;
		}

		mSymTable.push_back(macroDesc);
	}

	void Preprocessor::_removeMacroDefinition(const std::string& macroName) TCPP_NOEXCEPT
	{
		if (_shouldTokenBeSkipped())
		{
			return;
		}

		auto iter = std::find_if(mSymTable.cbegin(), mSymTable.cend(), [&macroName](auto&& item) { return item.mName == macroName; });
		if (iter == mSymTable.cend())
		{
			mOnErrorCallback({ E_ERROR_TYPE::UNDEFINED_MACRO, mpLexer->GetCurrLineIndex() });
			return;
		}

		mSymTable.erase(iter);

		auto currToken = mpLexer->GetNextToken();
		_expect(E_TOKEN_TYPE::NEWLINE, currToken.mType);
	}

	std::vector<TToken> Preprocessor::_expandMacroDefinition(const TMacroDesc& macroDesc, const TToken& idToken) TCPP_NOEXCEPT
	{
		// \note expand object like macro with simple replacement
		if (macroDesc.mArgsNames.empty())
		{
			static const std::unordered_map<std::string, std::function<TToken()>> systemMacrosTable
			{
				{ "__LINE__", [&idToken]() { return TToken { E_TOKEN_TYPE::BLOB, std::to_string(idToken.mLineId) }; } }
			};

			auto iter = systemMacrosTable.find(macroDesc.mName);
			if (iter != systemMacrosTable.cend())
			{
				return { iter->second() };
			}

			return macroDesc.mValue;
		}

		mContextStack.push_back(macroDesc.mName);

		// \note function like macro's case
		auto currToken = mpLexer->GetNextToken();
		_expect(E_TOKEN_TYPE::OPEN_BRACKET, currToken.mType);

		std::vector<std::vector<TToken>> processingTokens;
		std::vector<TToken> currArgTokens;

		// \note read all arguments values
		while (true)
		{
			currArgTokens.clear();

			while ((currToken = mpLexer->GetNextToken()).mType == E_TOKEN_TYPE::SPACE); // \note skip space tokens
			currArgTokens.push_back({ currToken });

			while ((currToken = mpLexer->GetNextToken()).mType == E_TOKEN_TYPE::SPACE);
			
			while (currToken.mType != E_TOKEN_TYPE::COMMA &&
				   currToken.mType != E_TOKEN_TYPE::NEWLINE &&
				   currToken.mType != E_TOKEN_TYPE::CLOSE_BRACKET)
			{
				currArgTokens.push_back({ currToken });
				currToken = mpLexer->GetNextToken();
			}

			if (currToken.mType != E_TOKEN_TYPE::COMMA && currToken.mType != E_TOKEN_TYPE::CLOSE_BRACKET)
			{
				_expect(E_TOKEN_TYPE::COMMA, currToken.mType);
			}

			processingTokens.push_back(currArgTokens);

			if (currToken.mType == E_TOKEN_TYPE::CLOSE_BRACKET)
			{
				break;
			}
		}

		if (processingTokens.size() != macroDesc.mArgsNames.size())
		{
			mOnErrorCallback({ E_ERROR_TYPE::INCONSISTENT_MACRO_ARITY, mpLexer->GetCurrLineIndex() });
		}

		// \note execute macro's expansion
		std::vector<TToken> replacementList{ macroDesc.mValue.cbegin(), macroDesc.mValue.cend() };
		const auto& argsList = macroDesc.mArgsNames;

		std::string replacementValue;

		for (short currArgIndex = 0; currArgIndex < processingTokens.size(); ++currArgIndex)
		{
			const std::string& currArgName = argsList[currArgIndex];
			auto&& currArgValueTokens = processingTokens[currArgIndex];

			replacementValue.clear();

			for (auto&& currArgToken : currArgValueTokens)
			{
				replacementValue.append(currArgToken.mRawView);
			}

			for (auto& currToken : replacementList)
			{
				if ((currToken.mType != E_TOKEN_TYPE::IDENTIFIER) || (currToken.mRawView != currArgName))
				{
					continue;
				}

				currToken.mRawView = replacementValue;
			}
		}

		replacementList.push_back({ E_TOKEN_TYPE::REJECT_MACRO, macroDesc.mName });
		return replacementList;
	}

	void Preprocessor::_expect(const E_TOKEN_TYPE& expectedType, const E_TOKEN_TYPE& actualType) const TCPP_NOEXCEPT
	{
		if (expectedType == actualType)
		{
			return;
		}

		mOnErrorCallback({ E_ERROR_TYPE::UNEXPECTED_TOKEN, mpLexer->GetCurrLineIndex() });
	}

	void Preprocessor::_processInclusion() TCPP_NOEXCEPT
	{
		TToken currToken;

		while ((currToken = mpLexer->GetNextToken()).mType == E_TOKEN_TYPE::SPACE); // \note skip space tokens
		
		if (currToken.mType != E_TOKEN_TYPE::LESS && currToken.mType != E_TOKEN_TYPE::QUOTES)
		{
			while ((currToken = mpLexer->GetNextToken()).mType == E_TOKEN_TYPE::NEWLINE); // \note skip to end of current line

			mOnErrorCallback({ E_ERROR_TYPE::INVALID_INCLUDE_DIRECTIVE, mpLexer->GetCurrLineIndex() });
			return;
		}

		bool isSystemPathInclusion = currToken.mType == E_TOKEN_TYPE::LESS;
		
		std::string path;

		while (true)
		{
			if ((currToken = mpLexer->GetNextToken()).mType == E_TOKEN_TYPE::QUOTES ||
				currToken.mType == E_TOKEN_TYPE::GREATER)
			{
				break;
			}

			if (currToken.mType == E_TOKEN_TYPE::NEWLINE)
			{
				mOnErrorCallback({ E_ERROR_TYPE::UNEXPECTED_END_OF_INCLUDE_PATH, mpLexer->GetCurrLineIndex() });
				break;
			}

			path.append(currToken.mRawView);
		}

		while ((currToken = mpLexer->GetNextToken()).mType == E_TOKEN_TYPE::SPACE);
		_expect(E_TOKEN_TYPE::NEWLINE, currToken.mType);

		IInputStream* pInputStream = mOnIncludeCallback(path, isSystemPathInclusion);
		if (!pInputStream)
		{
			TCPP_ASSERT(false);
			return;
		}

		mpLexer->PushStream(*pInputStream);
	}

	Preprocessor::TIfStackEntry Preprocessor::_processIfConditional() TCPP_NOEXCEPT
	{
		auto currToken = mpLexer->GetNextToken();
		_expect(E_TOKEN_TYPE::SPACE, currToken.mType);

		std::vector<TToken> expressionTokens;

		while ((currToken = mpLexer->GetNextToken()).mType != E_TOKEN_TYPE::NEWLINE)
		{
			expressionTokens.push_back(currToken);
		}

		_expect(E_TOKEN_TYPE::NEWLINE, currToken.mType);
		
		return TIfStackEntry { static_cast<bool>(!_evaluateExpression(expressionTokens)) };
	}


	Preprocessor::TIfStackEntry Preprocessor::_processIfdefConditional() TCPP_NOEXCEPT
	{
		auto currToken = mpLexer->GetNextToken();
		_expect(E_TOKEN_TYPE::SPACE, currToken.mType);

		currToken = mpLexer->GetNextToken();
		_expect(E_TOKEN_TYPE::IDENTIFIER, currToken.mType);

		std::string macroIdentifier = currToken.mRawView;

		currToken = mpLexer->GetNextToken();
		_expect(E_TOKEN_TYPE::NEWLINE, currToken.mType);

		return TIfStackEntry { std::find_if(mSymTable.cbegin(), mSymTable.cend(), [&macroIdentifier](auto&& item)
								{
									return item.mName == macroIdentifier;
								}) == mSymTable.cend() };
	}

	Preprocessor::TIfStackEntry Preprocessor::_processIfndefConditional() TCPP_NOEXCEPT
	{
		auto currToken = mpLexer->GetNextToken();
		_expect(E_TOKEN_TYPE::SPACE, currToken.mType);

		currToken = mpLexer->GetNextToken();
		_expect(E_TOKEN_TYPE::IDENTIFIER, currToken.mType);

		std::string macroIdentifier = currToken.mRawView;

		currToken = mpLexer->GetNextToken();
		_expect(E_TOKEN_TYPE::NEWLINE, currToken.mType);

		return TIfStackEntry { std::find_if(mSymTable.cbegin(), mSymTable.cend(), [&macroIdentifier](auto&& item)
								{
									return item.mName == macroIdentifier;
								}) != mSymTable.cend() };
	}

	void Preprocessor::_processElseConditional(TIfStackEntry& currStackEntry) TCPP_NOEXCEPT
	{
		if (currStackEntry.mHasElseBeenFound)
		{
			mOnErrorCallback({ E_ERROR_TYPE::ANOTHER_ELSE_BLOCK_FOUND, mpLexer->GetCurrLineIndex() });
			return;
		}

		currStackEntry.mShouldBeSkipped  = !currStackEntry.mShouldBeSkipped;
		currStackEntry.mHasElseBeenFound = true;
	}

	void Preprocessor::_processElifConditional(TIfStackEntry& currStackEntry) TCPP_NOEXCEPT
	{
		if (currStackEntry.mHasElseBeenFound)
		{
			mOnErrorCallback({ E_ERROR_TYPE::ELIF_BLOCK_AFTER_ELSE_FOUND, mpLexer->GetCurrLineIndex() });
			return;
		}

		auto currToken = mpLexer->GetNextToken();
		_expect(E_TOKEN_TYPE::SPACE, currToken.mType);

		std::vector<TToken> expressionTokens;

		while ((currToken = mpLexer->GetNextToken()).mType != E_TOKEN_TYPE::NEWLINE)
		{
			expressionTokens.push_back(currToken);
		}

		_expect(E_TOKEN_TYPE::NEWLINE, currToken.mType);

		currStackEntry.mShouldBeSkipped = static_cast<bool>(!_evaluateExpression(expressionTokens));
	}

	int Preprocessor::_evaluateExpression(const std::vector<TToken>& exprTokens) const TCPP_NOEXCEPT
	{
		std::vector<TToken> tokens{ exprTokens.begin(), exprTokens.end() };
		tokens.push_back({ E_TOKEN_TYPE::END });

		// \note use recursive descent parsing technique to evaluate expression
		auto evalCall = []()
		{
			return 0;
		};

		auto evalPrimary = [this, &tokens, &evalCall]()
		{
			auto currToken = tokens.front();

			switch (currToken.mType)
			{
				case E_TOKEN_TYPE::IDENTIFIER:
					{
						// \note macro call
						if (tokens.size() >= 2 && tokens[1].mType == E_TOKEN_TYPE::OPEN_BRACKET)
						{
							return evalCall();
						}

						tokens.erase(tokens.cbegin());

						// \note simple identifier
						return static_cast<int>(std::find_if(mSymTable.cbegin(), mSymTable.cend(), [&currToken](auto&& item)
						{
							return item.mName == currToken.mRawView;
						}) != mSymTable.cend());
					}
				case E_TOKEN_TYPE::NUMBER:
					tokens.erase(tokens.cbegin());
					return std::stoi(currToken.mRawView);
				default:
					break;
			}
			
			return 0;
		};

		auto evalUnary = [&tokens, &evalPrimary]()
		{
			int result = evalPrimary();

			auto currToken = tokens.front();
			switch (currToken.mType)
			{
				case E_TOKEN_TYPE::MINUS:
					result = -result;
					break;
				case E_TOKEN_TYPE::NOT:
					result = !result;
					break;
				default:
					break;
			}

			return result;
		};

		auto evalMultiplication = [&tokens, &evalUnary]()
		{
			int result = evalUnary();

			TToken currToken;
			while ((currToken = tokens.front()).mType == E_TOKEN_TYPE::STAR || currToken.mType == E_TOKEN_TYPE::SLASH)
			{
				switch (currToken.mType)
				{
					case E_TOKEN_TYPE::STAR:
						result = result * evalUnary();
						break;
					case E_TOKEN_TYPE::SLASH:
						result = result / evalUnary();
						break;
					default:
						break;
				}
			}

			return result;
		};

		auto evalAddition = [&tokens, &evalMultiplication]()
		{
			int result = evalMultiplication();

			TToken currToken;
			while ((currToken = tokens.front()).mType == E_TOKEN_TYPE::PLUS || currToken.mType == E_TOKEN_TYPE::MINUS)
			{
				switch (currToken.mType)
				{
					case E_TOKEN_TYPE::PLUS:
						result = result + evalMultiplication();
						break;
					case E_TOKEN_TYPE::MINUS:
						result = result - evalMultiplication();
						break;
					default:
						break;
				}
			}

			return result;
		};

		auto evalComparison = [&tokens, &evalAddition]()
		{
			int result = evalAddition();

			TToken currToken;
			while ((currToken = tokens.front()).mType == E_TOKEN_TYPE::LESS || 
					currToken.mType == E_TOKEN_TYPE::GREATER || 
					currToken.mType == E_TOKEN_TYPE::LE || 
					currToken.mType == E_TOKEN_TYPE::GE)
			{
				switch (currToken.mType)
				{
					case E_TOKEN_TYPE::LESS:
						result = result < evalAddition();
						break;
					case E_TOKEN_TYPE::GREATER:
						result = result > evalAddition();
						break;
					case E_TOKEN_TYPE::LE:
						result = result <= evalAddition();
						break;
					case E_TOKEN_TYPE::GE:
						result = result >= evalAddition();
						break;
					default:
						break;
				}
			}

			return result;
		};

		auto evalEquality = [&tokens, &evalComparison]()
		{
			int result = evalComparison();

			TToken currToken;
			while ((currToken = tokens.front()).mType == E_TOKEN_TYPE::EQ || currToken.mType == E_TOKEN_TYPE::NE)
			{
				switch (currToken.mType)
				{
					case E_TOKEN_TYPE::EQ:
						result = result == evalComparison();
						break;
					case E_TOKEN_TYPE::NE:
						result = result != evalComparison();
						break;
					default:
						break;
				}
			}

			return result;
		};

		auto evalAndExpr = [&tokens, &evalEquality]()
		{
			int result = evalEquality();

			while (tokens.front().mType == E_TOKEN_TYPE::AND)
			{
				result = result && evalEquality();
			}

			return result;
		};

		auto evalOrExpr = [&tokens, &evalAndExpr]()
		{
			int result = evalAndExpr();
			
			while (tokens.front().mType == E_TOKEN_TYPE::OR)
			{
				result = result || evalAndExpr();
			}

			return result;
		};

		return evalOrExpr();
	}

	bool Preprocessor::_shouldTokenBeSkipped() const TCPP_NOEXCEPT
	{
		return !mConditionalBlocksStack.empty() && mConditionalBlocksStack.top().mShouldBeSkipped;
	}

#endif
}