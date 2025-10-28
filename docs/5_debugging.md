# Debugging

Assertions and checks can be done by using the following macros:
```cpp
#include <hermes/common/debug.h>

// warns if expr is false
HERMES_CHECK_EXP(expr);
// warns with message M if expr is false
HERMES_CHECK_EXP_WITH_LOG(expr, M);
// errors if expr is false
HERMES_ASSERT(expr);
// errors with message M if expr is false
HERMES_ASSERT_WITH_LOG(expr, M);
```
Sometimes you want some piece of code to be compiled only in debug mode,
this macro can be convenient in this situation:
```cpp
HERMES_DEBUG_CODE(CODE_CONTENT)
```
Then use this way:
```cpp
#include <hermes/common/debug.h>

int main() {
  HERMES_DEBUG_CODE(
      int a = 3;
      printf("%d", a);
      )
  return 0;
}
```
both lines will not be included in release.
