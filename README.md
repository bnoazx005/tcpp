# TCPP (Tiny C PreProcessor)

[![Actions Status](https://github.com/dmikushin/tcpp/actions/workflows/integrate.yaml/badge.svg)](https://github.com/dmikushin/tcpp/actions)

TCPP is small single-header library which provides implementation of C preprocessor (almost). Most part of the library is based upon official specifications of the preprocessor https://docs.freebsd.org/info/cpp/cpp.pdf, https://gcc.gnu.org/onlinedocs/cpp/. 

This project was started with the only one need which was to implement preprocessor for GLSL and HLSL languages and use it within custom game engine. That's it. So I've implemented the simplest working tool as I guess. And I really hope that this library will help for someone who shares my ideas and beliefs. I've not chosen Boost.Wave library because of its dependencies. Yeah, Boost looks so modular until you try to integrate the only module into some project. So, that's why this library is designed in single-header way.

## Table of contents

1. ### [Current Features](#current-features)
2. ### [How to Use](#how-to-use)

***

### Current Features:<a name="current-features"></a>

* Conditional preprocessing

* Support of object-like and function-like macros, except variadic ones

* Error handling via the only callback

* Simple API: all the library based on a few classes with simplistic interfaces

* Exceptions free code: we believe that's explicit error handling is much more robust and ease way 

***

### How to Use<a name="how-to-use"></a>

Just copy **tcppLibrary.hpp** into your working directory and export it within some of your source file like the following
```cpp
#define TCPP_IMPLEMENTATION
#include "tcppLibrary.hpp"
```
Be sure, that's **TCPP_IMPLEMENTATION** flag is defined only once in the project.
