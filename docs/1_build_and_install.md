# Building Hermes

\tableofcontents

Hermes is in [github](https://github.com/filipecn/hermes), so you can download directly in the website or you can
just `clone` the repository like this:

```shell
git clone https://github.com/filipecn/hermes.git
```

Regardless the method you just chose, jump into the hermes folder containing all the source code:

```shell
cd hermes
```

The folder should contain something like this

```shell
ls 
CMake CMakeLists.txt docs Doxyfile ext hermes LICENSE README.md tests
```

Now you are all set to build hermes!

## cmake

Since there are no raw makefiles or anything, the easy way to go is with [cmake](https://cmake.org/). `cmake` 
is a tool that generates all the makefiles for you in a very portable manner, so you can just call 
`make` later. Usually, people create a separate directory for all the generated files and binaries:

```shell
mkdir build
cd build
```
> From now on, all the commands must be called from this build folder you just created

then you can invoke `cmake` from the newly created folder:
```shell
cmake ..
```
This will tell `cmake` that the source code is in the parent folder (if you created `build` inside `hermes`) 
and `cmake` will generate all its files, include the makefiles, in the current (`build`) folder. Calling
`cmake` with no configuration options (just the `..` argument) will configure `hermes` on its default setting: 
which is just the hermes library. But of course there are other things you may want to compile along with
the core library.

## Build Options

We set compilation options in the `cmake` command. For example, `hermes` provides unit tests, and `CUDA` support. 
You need to tell `cmake` if you want these extra features. An option can _turned_ `ON` or `OFF`, depending on 
whether it is set or not - so options with value `ON` are options you want. Here is a list of all the 
options you can use when configuring `cmake` for `hermes`:

| option | description | default  |
|--------------|--------|-----|
| BUILD_WITH_CUDA  | build `hermes` with support to CUDA | OFF |
| BUILD_TESTS  | build unit-tests | OFF |
| BUILD_EXAMPLES  | build examples | OFF |
| BUILD_DOCS  | generates documentation | OFF |
| BUILD_ALL  | set all options above to ON | OFF |

You set an option by using the `-D` argument in the command line. Suppose you would like to 
use `CUDA` (BUILD_WITH_CUDA option) and also perform the unit tests (BUILD_TESTS options), 
your `cmake` command then will look like this:

````shell
cmake .. -DBUILD_WITH_CUDA=ON -DBUILD_TESTS=ON
````

## Dependencies
Hermes' core is dependency-free, the library is self-contained. The only (optionally) required 
external libraries are the `CUDA` runtime libs and [catch2](https://github.com/catchorg/Catch2),
when building with options BUILD_WITH_CUDA and BUILD_TESTS, respectively.

Since `hermes` source already include `catch2` headers, you don't need to take any actions about 
it.

`CUDA` is more complicated. You will need to have `CUDA` runtime libraries installed in your machine
and let `cmake` know where `nvcc`  (`CUDA` compiler) is located passing the `CMAKE_CUDA_COMPILER` 
variable. For example, in my machine `nvcc` is located at `/usr/local/cuda-11.0/bin/nvcc`,
so my `cmake` command would look like this:
```shell
cmake .. -DBUILD_WITH_CUDA=ON -DCMAKE_CUDA_COMPILER=/usr/local/cuda-11.0/bin/nvcc
```

### CUDA

Right now, my `cmake` file is very simple regarding `CUDA`, I just call the `enable_language(CUDA)`
function inside it and include CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES (which is defined automatically
by `cmake`).

> I'm currently using Ubuntu 20.04 with no special setting. Hopefully `cmake` will be able to 
> handle `CUDA` in your system as well :)

## Install

You can also install `hermes` headers and library wherever you want. By default, the installation 
folder is the `build` folder from where you ran the `cmake` command. In order to install in 
another location use the INSTALL_PATH variable when running the `cmake` command like this:

```shell
cmake .. -DINSTALL_PATH=/my/install/location
```

## Build

If the `cmake` command runs successfully, then you can proceed to compilation:

```shell
make -j8
```
and install
```shell
make install
```

Hopefully you will not encounter any errors! Now you are free include and link `hermes` to
your project :)

## Summary
So putting all together, we get something like this:
```shell
git clone https://github.com/filipecn/hermes.git
cd hermes
mkdir build
cd build
cmake .. -DBUILD_ALL=ON -DINSTALL_PATH=/my/install/location
make -j8 install
```
---

Check the [next](2_linking.md) page to see how use `hermes` in your project.

## Troubleshooting

_TODO_