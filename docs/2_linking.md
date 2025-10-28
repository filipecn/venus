# Linking Hermes

\tableofcontents

Once `hermes` is built and/or installed ([instructions](1_build_and_install.md))
you are free to use it in your project.

Linking `hermes` is quite straightforward since it just contains a single static library file and
all the headers are in a single folder. However, if you compiled it with `CUDA` you will need 
to take extra care when linking it. 

Before talking about `CUDA`, lets see what you need to do if you compiled `hermes` **without** 
`CUDA`. You want to link your code against `hermes` library, which in `UNIX` systems will
probably a file called `libhermes.a`.

If you ran `make install`, then all the files will be in the installation folder, which by default
is the folder you used to build the library itself. Both headers and the library file will be 
separate, respectively, in two folders like this:
```shell
include/
 - hermes/
   - # all headers and subdirectories of headers
lib/
 - libhermes.a
```

Once you now exactly the location of these folders, you can easily include and link them 
in your project. For example, lets say hermes is installed under `/my/location` directory.
So there you find both folders:
```shell
/my/location/include # containing hermes headers
/my/location/lib     # containing libhermes.a
```

Suppose you want compile the following program saved in a file `main.cpp`:
```cpp
#include <hermes/common/debug.h>
int main() {
  HERMES_LOG("hello hermes!")
  return 0;
}
```

In order to compile and run this file using `gcc` you can do something like this:
```shell
# compilation command
g++ -I/my/location/include -l/my/location/lib/libhermes.a main.cpp 
# run
./a.out
hello hermes!
```

## cmake 
If you use `cmake` in your project, then it might be useful to have 
a `hermes.cmake` (listed later) that conveniently sets variables - HERMES_LIBRARIES and
HERMES_INCLUDES - pointing to respective `hermes` locations. This method
requires you to do the following modifications to your `CMakeLists.txt`:
```cmake
include(ExternalProject)
# let cmake know where to find hermes.cmake file
# for example, a folder called ext
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext")
# add hermes.cmake to your project
include(hermes)
# set hermes as a dependency to your target
add_dependencies(my_target hermes)
# and finally, linking to your target this way
target_link_libraries(my_target PUBLIC ${HERMES_LIBRARIES})
# and including the headers like this
target_include_directories(my_target PUBLIC ${HERMES_INCLUDES})
```

I usually use the following `hermes.cmake` file:
```cmake
# allow cmake to handle external projects (hermes in this case)
include(ExternalProject)
# config hermes package
ExternalProject_Add(
        hermes PREFIX hermes
        URL "https://github.com/filipecn/hermes/archive/refs/heads/main.zip"
        CMAKE_ARGS
        "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DINSTALL_PATH=install"
)
# retrieve hermes local install
ExternalProject_Get_Property(hermes INSTALL_DIR)
set(INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/hermes)
# set variable containing hermes include directory
set(HERMES_INCLUDES ${INSTALL_DIR}/install/include)
# set variable containing hermes library
set(HERMES_LIBRARIES ${INSTALL_DIR}/install/lib/${CMAKE_STATIC_LIBRARY_PREFIX}hermes${CMAKE_STATIC_LIBRARY_SUFFIX})
# output both variables
set(HERMES_INCLUDES ${HERMES_INCLUDES} CACHE STRING "")
set(HERMES_LIBRARIES ${HERMES_LIBRARIES} CACHE STRING "")
```

Notice that inside `CMAKE_ARGS` section, in `ExternalProject_Add` function, you can pass the
options you want from `hermes`. We will need to do this for `CUDA` for example.

## CUDA
For any option other than BUILD_WITH_CUDA the process is exactly the same as above. Just
pass the `cmake` arguments the way you want and be happy. In the case of `CUDA`, there will be two main 
changes in the `hermes.cmake`:
```cmake
# Add the CUDA option in CMAKE_ARGS section
ExternalProject_Add(
        ...
        CMAKE_ARGS
        ...
        "-DBUILD_WITH_CUDA=ON" 
        ...
)
# And add CUDA toolkit headers location to HERMES_INCLUDES variables
set(HERMES_INCLUDES
        ...
        ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
        )
```
> Note: CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES should be automatically defined by cmake when
> you set your project language to CUDA as well

There are several ways to compile `CUDA` code with `cmake`, here a possible configuration:

```cmake
target_compile_definitions(my_target PUBLIC
        -DENABLE_CUDA=1
        -D__CUDA_INCLUDE_COMPILER_INTERNAL_HEADERS__=1)
target_include_directories(helios PUBLIC ${HERMES_INCLUDES})
target_compile_options(my_target PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:
        --generate-line-info
        --use_fast_math
        -Xcompiler -pg
        --relocatable-device-code=true
        >)
set_target_properties(helios PROPERTIES
        LINKER_LANGUAGE CUDA
        CMAKE_CUDA_SEPARABLE_COMPILATION ON
        CUDA_RESOLVE_DEVICE_SYMBOLS OFF
        POSITION_INDEPENDENT_CODE ON
        CUDA_STANDARD 17
        CUDA_STANDARD_REQUIRED ON
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        )
```
---
Alright! If you are still there and could compile your code along with `hermes`, then it is time to use! 
Please check the [introduction](0_getting_started.md) page to get yourself started!

## Troubleshooting

_TODO_
