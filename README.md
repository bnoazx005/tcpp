![TDEngine2](https://i.imgur.com/Q8QlaxD.png)

TDEngine 2 is a cross-platform game engine.

| Platform (Compiler) | Build status  |
|--|--|
| Win32 (Visual Studio 2015 / 2017) |[![Build status](https://ci.appveyor.com/api/projects/status/dyp34r05yfxii09m?svg=true)](https://ci.appveyor.com/project/bnoazx005/tdengine2) |
| UNIX (g++ / clang) |[![Build Status](https://travis-ci.org/bnoazx005/TDEngine2.svg?branch=master)](https://travis-ci.org/bnoazx005/TDEngine2) |


## Table of contents

1. ### [Current Goals](#current-goals)
2. ### [Current Features](#current-features)
3. ### [How to Build](#how-to-build)

    ### 2.1 [Visual Studio (Win32)](#vs-win32)

    ### 2.2 [Make Utility (UNIX)](#make-unix)

    ### 2.3 [Compilation](#compilation)
    
4. ### [Getting Started](#getting-started)

***

### Current Goals:<a name="current-goals"></a>

* Entity-Component-System architecture;

* Windows (7 and higher), UNIX systems and Android OS support;

* 2D and 3D graphics support via Direct3D 11 and OpenGL 3.3 (with extensions) GAPIs;

* Effective memory management (in future);

* Extensibility, Modularity and plugins support;

* Toolset (including resource editor, scene editor, exporters and etc) (in future).

***

### Current Features:<a name="current-features"></a>

* Entity-Component-System architecture with support of events, reactive systems and etc;

* Windows (7 and higher), UNIX systems support;

* Support of D3D11 and OGL 3.x GAPIs;

* 2D sprites rendering with instancing support;

* 3D static meshes rendering with different configurable materials;

***

### How to Build<a name="how-to-build"></a>

The project actively uses CMake for the building process. So firstly, you should be sure you have 
CMake tools of 2.8 version or higher to continue. At the moment the engine supports Win32 and UNIX 
platforms.

#### Visual Studio (Win32)<a name="vs-win32"></a>

By now the project  was built using Visual Studio 2017, but it will be successfully built with 2015 
version as well. 

To generate the basic Visual Studio's solution of the engine's SDK you should run the following 
commands in a console (the instructions are correct if you are within root directory of the project):
```console
mkdir build
cd build
cmake .. -G "Visual Studio 201X"
```
where X is 5 or 7.

#### Make utility (UNIX)<a name="make-unix"></a>

Under UNIX platform you have an option to build the project using make utility. As described above you
need firstly generate Makefile with CMake. The process looks the same, but another CMake's generator
is used:
```console
$ mkdir build
$ cd build
$ cmake .. -G "Unix Makefiles"
```

Note. There are a few requirements for successfull compilation of the engine under UNIX. The first is
a support of GLX 1.4. It could be done if you have proprietary video drivers installed on your system.
So your video card should support at least OpenGL 3.0. The second is installed X11 and GLEW libraries.

#### Compilation<a name="compilation"></a>

If everything went ok you've got a solution of the entire SDK (including sample project, 
which is called SandboxGame). For now you can compile it either using Visual Studio IDE,
or a console's command:
```console
$ cmake --build .
```
All executables can be found within /bin directory. Note. To speed up building process you can
exclude a part of built-in plugins, which 
aren't needed you.

If you use UNIX based OS you can install all the compiled libraries into /usr/lib/TDEngine2/
directory (call make install within /build directory),
```console
# make install
```
but everything will work well if you'll place plugins' *.so files and TDEngine2.so
at the same directory with an executable program.

***

### Getting Started<a name="getting-started"></a>

(soon)

