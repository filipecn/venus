# Numeric

\tableofcontents

Numerical classes provide all sort of number operations and functions. The 
`hermes::Constants`, for example, offer you common constants:
```cpp
#include <hermes/numeric/numeric.h>
// Here are some constants provided by hermes
// and how you can access them
hermes::Constants::pi;
hermes::Constants::pi_over_four;
hermes::Constants::machine_epsilon;
// and more
```

## Numbers
The `hermes::Numbers` namespace, included by `hermes/numeric/numeric.h`,
provides lots of functions to work with
floating-point, integer and binary number representations. Most functions
are template functions. Here is an **incomplete** list of what you can 
find there, for more functions, please check the documentation of
`hermes::Numbers`.
- **Type representation limits**
    ```cpp
    hermes::Numbers::lowest<T>();
    hermes::Numbers::greatest<T>();
    ```
- **Functions**
  ```cpp
  hermes::Numbers::fract(x);
  hermes::Numbers::clamp(x, l, u);
  hermes::Numbers::sqr(x);
  hermes::Numbers::safe_sqrt(x);
  hermes::Numbers::cube(x);
  hermes::Numbers::sign(x);
  hermes::Numbers::max({-1, 2, 4, 10});
  hermes::Numbers::pow<N>(x);
  ```
- **Binary representations**
  ```cpp
  hermes::Numbers::countHexDigits(x);
  hermes::Numbers::floatExponent(x);
  hermes::Numbers::floatSignificand(x);
  hermes::Numbers::floatSignBit(x);
  hermes::Numbers::floatToBits(x);
  hermes::Numbers::bitsToFloat(u);
  hermes::Numbers::interleaveBits(x, y, z);
  ```
- **Rounding and precision**
  ```cpp
  hermes::Numbers::nextFloatUp(x);
  hermes::Numbers::nextFloatDown(x);
  hermes::Numbers::ceilToInt(x);
  hermes::Numbers::floorToInt(x);
  hermes::Numbers::roundToInt(x);
  // all four arithmetic operations with rounding 
  // to next float up to machine precision
  hermes::Numbers::subRoundUp(x, y);
  ...
  hermes::Numbers::divRoundDonw(x, y);
  hermes::Numbers::sqrtRoundDown(x, y);
  hermes::Numbers::gamma(n);
  ```

### Trigonometry
A few trigonometric functions are listed in `hermes::Trigonometry`:
```cpp
#include <hermes/numeric/numeric.h>
// Examples of trigonometric functions
hermes::Trigonometry::radians2degrees(hermes::Constants::two_pi);
hermes::Trigonometry::safe_acos(-2);
```

### Checks
You can make some floating-number comparisons as well using `hermes::Check`:
```cpp
#include <hermes/numeric/numeric.h>
float x, y;
hermes::Check::is_zero(x);
hermes::Check::is_nan(x);
hermes::Check::is_equal(x, y, 1e-8);
hermes::Check::is_between(0.1, x, y);
hermes::Check::is_between_closed(0.1, x, y);
```

## Interpolation
Check `hermes::interpolation` namespace for interpolation functions.
```cpp
#include <hermes/numeric/interpolation.h>

hermes::interpolation::smooth(a, b);
hermes::interpolation::smoothStep(a, b, v);
hermes::interpolation::sharpen(a, b);
hermes::interpolation::linearStep(v, a, b);
hermes::interpolation::mix(v, a, b);
hermes::interpolation::lerp(t, a, b);
// also bilerp and trilerp
hermes::interpolation::nearest(t, a, b);
hermes::monotonicCubicInterpolation(...);
```

## Arithmetic Interval
`hermes::Interval` represents a numeric interval that supports
interval arithmetic. It is very useful when you want to represent
your values considering the uncertainty that comes from the limitations
of the computer representations of floating-point numbers. This way,
`hermes::Interval` represents a closed interval `[low, high]` that
guarantees to enclose your real numerical value, by providing 
all sorts of arithmetic operations of intervals.

```cpp
#include <hermes/numeric/interval.h>

int main() {
  // suppose two intervals
  hermes::Interval<f32> a(-1, 1), (0, 2);
  // you can do common arithmetic (+,-,*,/)
  auto c = a * b; // [-2,2]
  // also
  c.sqr();  // [0, 4]
  c.sqrt(); // [nan, 1.41421]
return 0;
}
```

You can also make some queries:
```cpp
hermes::Interval<i32> a(-1,1);
a.contains(0); // true
a.center();    // 0
a.radius();    // 1
a.width();     // 2
a.isExact();   // false ([1,1] is exact)
```

## EFloat
For certain calculations, you may want to keep track of 
floating-point operation errors accumulated over time. The 
`hermes::EFloat` class uses the `hermes::Interval` to  carry
error bounds during floating-point operations.

```cpp
#include <hermes/numeric/e_float.h>

int main() {
  // EFloats start as exact intervals
  // in this case, [3,3] and [-1,-1]
  hermes::EFloat a(3.f), b(-1.f);
  // suppose you do math with them
  auto c = a * 3 + a * b - 4 * b + a / b;
  // the resulting quantity will contain errors
  // that can be checked this way:
  c.absoluteError(); // 1.14441e-05
  c.upperBound();    // 7.00001
  c.lowerBound();    // 6.99999
  return 0;
}
```