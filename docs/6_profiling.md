# Profiling

Hermes provides a profiling tool to track your code performance,
the hermes::profiler::Profiler singleton class. It works by
registering blocks that represent execution time of code sections,
scopes and functions. Each block receives a name (and a color if you want),
so you can analyze your data later.

You can profile your code by using a set of macros
like this:

```cpp
#include <hermes/common/profiler.h>
// suppose you want to profile the following function
void profiled_function() {
    // register a block taking the function's name as the label
    // the block is automatically finished after leaving this function
    HERMES_PROFILE_FUNCTION()
    // some code
    {
        // register a block with the label "code scope"
        // the block is automatically finished after leaving this function
        HERMES_PROFILE_SCOPE("code scope")
    }
    // you can also initiate and finish a block manually
    HERMES_PROFILE_START_BLOCK("my block")
    // some code
    // finish "my block" (always remember do finish your custom blocks!)
    HERMES_PROFILE_END_BLOCK
}

int main() {
  profiled_function();
  return 0;
}
```
> The profiler uses with a simple stack to manage block creation and completion.
> So remember to finish blocks consistently.

You can access the history of blocks as well:
```cpp
using hermes::profiler;
Profiler::iterateBlocks([](const Profiler::Block &block {
      auto block_desc = Profiler::blockDescriptor(block);
      // block name
      block_desc.name;
      // block start time
      block.begin();
      // block duration
      block.duration();
    }));
```
Sometimes you don't want to store all blocks created since the start of your
program, maybe to save memory. You can limit the profiler to keep only the
last `n` blocks by calling:
```cpp
hermes::profiler::Profiler::setMaxBlockCount(n);
```
You can also enable or disable the profiler in runtime with the following
macros, respectively:
```cpp
HERMES_PROFILE_ENABLE
HERMES_PROFILE_DISABLE
```
Sometimes, it is also useful to set colors for your blocks. The block label
struct holds a field `u32 color;` for that purpose. You can encode your color
in this unsigned integer the way you prefer, but `hermes` provide a namespace
containing `u32` colors for you called hermes::argb_colors. You can
set the block's color with the same profiling macros:

```cpp
HERMES_PROFILE_FUNCTION(hermes::argb_colors::GreenA200);
HERMES_PROFILE_SCOPE("my scoped block", hermes::argb_colors::BlueA200);
HERMES_PROFILE_START_BLOCK("my custom block", hermes::argb_colors::Coral);
```
