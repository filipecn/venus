# Common Utilities

\tableofcontents

Welcome, here I will present the very basic tools that `hermes`
provides to help you during development. The classes and
data types listed here are used throughout the entire library and 
I hope you could use them in your code too.

Before anything, I would like to list here the very primitive
types that you will find in `hermes/common/defs.h`:

| name | description |
|--------------|-------|
| `i8`,`i16`,`i32`, and`i64`  | signed integers, respectively `int8_t`, `int16_t`, `int32_t`, and `int64_t`|
| `u8`,`u16`,`u32`, and`u64`  | unsigned integers, respectively `uint8_t`, `uint16_t`, `uint32_t`, and `uint64_t`|
| `byte`  | 8-bit unsigned (`uint8_t`)|
| `f32` and `f64`  | floating-point types, respectively `float` and `double`|

Sometimes you want to somehow, work with the types in
unusual ways. The `hermes::DataTypes` is an auxiliary 
namespace that provides 
some functions that convert between the declared types, and
the enum class `hermes::DataType`, that holds labels for
each of the types above: 
```cpp
#include <hermes/common/defs.h>

int main() {
  // extract class from type
  auto u8_type = hermes::DataTypes::typeFrom<u8>();
  // check type size
  hermes::DataTypes::typeSize(u8_type);
  // get type name (string)
  hermes::DataTypes::typeName(u8_type);
  return 0;
}
```

## Indices and Sizes
A common task is the iteration over multi-dimensional arrays and index ranges.
Hermes offers 2-dimensional and 3-dimensional index and size operations that
may facilitate these tasks with the following types:
```cpp
// integer based indices
hermes::index2; // (i,j)
hermes::index3; // (i,j,k)
// unsigned integer based sizes
hermes::size2; // (width, height)
hermes::size3; // (width, height, depth)
```
You can do all sort of arithmetic operations between them. But you can also
work with range of indices as well:
```cpp
// ranges represent a half-open interval [lower, upper)
hermes::range2; 
hermes::range3; 
```
which can be useful when you want to iterate over such type of indices:
```cpp
#include <hermes/common/index.h>
int main() {
  // let's iterate over all indices in [(0,0), (9,9)]
  hermes::size2 size(10, 10);
  for(auto index : hermes::range2(size)) {
    // access index coordinates as
    index.i;
    index.j;
  }
  return 0;
}
```

## Strings
Some functions of strings can be found in `hermes::Str`. Here are some examples
of what you can do:
```cpp
#include <hermes/common/str.h>

int main() {
  // strip string sides
  hermes::Str::strip(" asd \n", " \n"); // "asd"
  // split into substrings
  hermes::Str::split("1,2,3", ","); // {"1","2","3"}
  // join strings
  hermes::Str::join({"a","b","c"}, ","); // "a,b,c"
  // concatenate
  hermes::Str::concat("a", " ", 2); // "a 2"
  // format
  hermes::Str::format("{} has {} letters", "word", 4); // "word has 4 letters
  // numbers
  hermes::Str::isInteger("-234235"); // true
  hermes::Str::isNumber("-2e5"); // true
  hermes::Str::binaryToHex(10); // "0x1010"
  hermes::Str::addressOf(ptr); // ptr address value "0xFF..."
  // regex match
  hermes::Str::match_r("subsequence123". "\\b(sub)([^ ]*)"); // true
  // regex contains
  hermes::Str::contains_r("subsequence123", "\\b(sub)"); // true
  // and more ...
  return 0;
}
```

An instance of `hermes::Str` is just a `std::string` wrapper with an `<<` operator. 
So you can do stuff like:
```cpp
hermes::Str s;
s = s << "bla";
```

## Files
The `hermes::Filesystem` provides some _shell_-like functions that can help you
with files and directories. A auxiliary class is the `hermes::Path`,
which holds a filesystem path for a file or directory.

With `hermes::Path` in hands, you can ask all sorts of things like file extension,
directory name, absolute path, etc. You can also test if your file or directory
exists and create it if necessary. You can also construct your path like this:
```cpp
#include <hermes/common/filesystem.h>

int main() {
  hermes::Path parent("parent_folder");
  auto child = parent / "child";
  // now child is the path "parent/child"
  // you can create this path if necessary
  if(!child.exists())
    child.mkdir();
  // the same goes with a file
  hermes::Path file("parent_folder/file.txt");
  if(!file.exists())
    file.touch();
  // you could, for example check the file extension
  HERMES_LOG(file.extension());
  // the file name
  HERMES_LOG(file.name());
  // you can also check what type of path you have
  child.isDirectory();
  file.isFile();
  // in the case of the file, you can read 
  auto content = file.read();
  // and write
  file.writeTo("files content");
  return 0;
}
```

Sometimes you want to iterate over directories and files, find or copy. Here are
examples of how you can do:
```cpp
#include <hermes/common/filesystem.h>

int main() {
  // copy files
  hermes::Filesystem::copyFile("source/path", "destination/path");
  // find files recursively using regular expressions
  // also sort the results
  hermes::Filesystem::find("root/path", 
                           "*.cpp", // look for cpp files
                           hermes::find_options::recursive |
                           hermes::find_options::sort);
  // list only files in a directory, recursively
  hermes::FileSystem::ls("root/path", 
                         hermes::ls_options::files |
                         hermes::ls_options::recursive);
  // list a directory, putting directories first
  hermes::FileSystem::ls("root/path", 
                         hermes::ls_options::group_directories_first);
  return 0;
}
```
## Parsing Arguments
Reading command line arguments or parsing command strings can be done
with hermes::ArgParser:
```cpp
#include <hermes/common/arg_parser.h>

int main(int argc, char** argv) {
    hermes::ArgParser parser("my program", "description");
    // define a simple float argument
    parser.addArgument("--float_argument", "description");
    // an required argument
    parser.addArgument("--int_argument", "argument description", true);
    // parse arguments
    parser.parse(argc, argv);
    // access argument value with default value
    parser.get<int>("--int_argument", 0);
    // check if argument was given
    if(parser.check("--float_argument"))
      HERMES_LOG_VARIABLE(parser.get<float>("--float_argument"));
    return 0;
}
```
For the code above, you code pass arguments like this:
```shell
./a.out --int_argument 3 --float_argument 2.0
```
It works by parsing all arguments, in order, pairing tokens separated
by spaces. So you don't need to explicitly put the names of the arguments
if you don't want to:
```shell
./a.out 3 --float_argument 2.0
```
In that case, `3` will be parsed and considered to be the value of your `--int_argument`,
because the parser will **follow the addArgument order**.
