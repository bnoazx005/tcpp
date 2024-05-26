cmake_minimum_required (VERSION 3.8)

project(tcpp-tests LANGUAGES CXX)

option(IS_TESTING_ENABLED "The option turns on/off tests" ON)

if (NOT DEFINED ${TCPP_TESTS_NAME})
    set(TCPP_TESTS_NAME ${PROJECT_NAME})
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (IS_TESTING_ENABLED)
	enable_testing()
endif ()

# attach tcpp
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../source")

# include Catch2
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Catch2/contrib/")
add_subdirectory(ThirdParty/Catch2)

set(HEADERS )

set(SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/coreTests.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/lexerTests.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/stringInputStreamTests.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

source_group("includes" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

if (MSVC) 	#cl.exe compiler's options

	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /WX /std:c++14 /MDd /W3 /GS /Zc:inline /Od /Zi  /Zc:wchar_t")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze-")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++14 /permissive- /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /O2 /sdl /Zc:inline")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise /D _WINDLL /D _MBCS /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /EHsc /nologo /diagnostics:classic")

endif(MSVC)


if (UNIX)
	message(STATUS "UNIX system has detected...")

	include(CheckCXXCompilerFlag)

	CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX14)

	if(COMPILER_SUPPORTS_CXX14)
		message(STATUS "C++14 is enabled")

		set(CMAKE_CXX_STANDARD 14)
		set(CMAKE_CXX_STANDARD_REQUIRED ON)
	else()
		message(ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
	endif()
endif (UNIX)

add_executable(${TCPP_TESTS_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${TCPP_TESTS_NAME} Catch2::Catch2)

include(CTest)
include(Catch)

catch_discover_tests(${TCPP_TESTS_NAME})
