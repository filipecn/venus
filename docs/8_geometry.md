# Geometry
\tableofcontents

Here you will find all the basic geometric objects that are commonly used 
in computer applications, mainly Computer Graphics related. Most geometry
classes inherit from a base class called `hermes::MathElement`, this way 
you can easily access information like:
```cpp
// suppose elem is a from MathElement
// you can retrieve how many components 
// (a 3d vector has 3, a 4x4 matrix has 16)
elem.componentCount();
// you can get its underlying type
elem.numeric_data;
// And for debugging, you can also get its memory layout
elem.memoryDumpLayout();
```

Hermes defines its basic geometric entities - vectors, matrices, etc - based on their geometric 
definitions and linear algebra. Their arithmetic operators are defined in the same manner. For example,
if you sum a vector to a point, you will get a point. If you subtract a point from another point, 
you will get a vector. Other operations are defined accordingly.

## Vectors
The most basic, and commonly used geometric object is the vector.
In `hermes`, you will find vector representations in `hermes::Vector*` classes.
These classes are implementations for the [2,3,4]-dimensional
versions of vectors, which you can choose by changing the number
in the name. They are all template classes taking the underlying floating-point type as
parameter (ex: `f32` or `f64`).

Here are some convenient typedefs:
```cpp
#include <hermes/geometry/vector.h>
hermes::vec2;    // hermes::Vector2<real_t>
hermes::vec3;    // hermes::Vector3<real_t>
hermes::vec4;    // hermes::Vector4<real_t>
hermes::vec3d;   // hermes::Vector3<f64>
hermes::vec3f;   // hermes::Vector3<f32>
hermes::vec2f;   // hermes::Vector2<f32>
hermes::vec2i;   // hermes::Vector2<Interval<real_t>>
hermes::vec3i;   // hermes::Vector3<Interval<real_t>>
```

Borrowing some definitions from linear algebra, the `hermes::Vector` implements a vector
space. Consider two scalars \f$\alpha\f$ and \f$\beta\f$, and the vectors \f$u\f$, \f$v\f$, and \f$w\f$, then the following holds:

- **Commutativity**
  
  \f$u+v=v+u\f$

- **Associativity** 
  
  \f$(u + v) + w = u + (v + w)\f$ and \f$(\alpha\beta)v = \alpha(\beta v)\f$

- **Null element** 
  
  \f$v + 0 = 0 + v = v\f$

- **Additive Inverse** 
  
  \f$-v + v = 0\f$

- **Distributivity** 
  
  \f$(\alpha + \beta)v = \alpha v + \beta v\f$ and \f$\alpha(u + v) = \alpha u + \alpha v\f$

- **Scalar Identity** 
  
  \f$1 \cdot v = v\f$

The following code snippet lists some common methods provided by the vector class:
```cpp
// suppose you have two vectors (the same should apply fo
hermes::vec3 u, v;
// you can access their components in both ways
v.x; // or v[0]
v.y; // or v[1]
v.z; // or v[2]
// you can check its magnitude
u.length();
// you can normalize
v.normalize();
// swizzling is also supported
v.xy(); // and any other combination
```

Consider again a scalars \f$\alpha\f$, and the vectors \f$u\f$, \f$v\f$, and \f$w\f$. 
The following vector operations are also available (the equations bellow follow index notation):

- **dot product**:
  
  \f[u \cdot v = u_i v_i\f]
  ```cpp
  hermes::dot(u,v);
  ```

- **cross product**:

  \f[u \times b = \epsilon_{ijk}u_iv_j\hat{e}_k\f]
  ```cpp
  hermes::cross(u,v);
  ```

- **triple product**:

  \f[u \cdot (v \times w)\f]
  ```cpp
  hermes::triple(u,v,w);
  ```

- **normalization**:

  \f[||u|| = \sqrt{u \cdot u}\f]
  ```cpp
  hermes::normalize(u);
  ```
  
- **projection**:

  \f[\frac{u \cdot v}{||v||^2} v\f]
  ```cpp
  hermes::project(u,v);
  ```
  
- **rejection**:

  \f[u - \frac{u \cdot v}{||v||^2} v\f]
  ```cpp
  hermes::reject(u,v);
  ```

The following subsections describe special types of vectors.

### Points

Points, also interpreted as position vectors, follow the same name structure of vectors, and you will find their dimensional 
representations in `hermes::Point*` classes. These classes are implementations for the [2,3,4]-dimensional
versions of points, which you can choose by changing the number
in the name. They are also all template classes taking the underlying floating-point type as
parameter (ex: `f32` or `f64`).

Here are some convenient typedefs:
```cpp
#include <hermes/geometry/point.h>
hermes::point2;  // hermes::Point2<real_t>
hermes::point2f; // hermes::Point2<f32>
hermes::point2d; // hermes::Point2<f64>
hermes::point3;  // hermes::Point3<real_t>
hermes::point3f; // hermes::Point3<f32>
hermes::point3d; // hermes::Point3<f64>
hermes::point2i; // hermes::Point2<Interval<real_t>>
hermes::point3i; // hermes::Point3<Interval<real_t>>
```

The arithmetic of points is more restrict. The arithmetic operators will let you
only translate or scale a point:
```cpp
hermes::point3 p;
hermes::vec3 v;
real_t s;
// translation
p += v;
// scale
p *= s;
```

A subtraction of two points `p` and `q` will give you a _distance_ vector `d`:
```cpp
hermes::vec3 d = p - q;
```

The distance between two points can be computed as well:
```cpp
hermes::distance(p,q);
// the squared distance is also available
hermes::distance2(p,q);
```

### Normals

Normal vectors represent geometric normals, and you will find their dimensional
representations in `hermes::Normal*` classes. These classes are implementations for the [2,3]-dimensional
versions of normals, which you can choose by changing the number
in the name. They are also all template classes taking the underlying floating-point type as
parameter (ex: `f32` or `f64`).

Here are some convenient typedefs:
```cpp
#include <hermes/geometry/normal.h>
hermes::normal2;  // hermes::Normal2<real_t>
hermes::normal2f; // hermes::Normal2<f32>
hermes::normal2d; // hermes::Normal2<f64>
hermes::normal3;  // hermes::Normal3<real_t>
hermes::normal3f; // hermes::Normal3<f32>
hermes::normal3d; // hermes::Normal3<f64>
```

Normals are even more restrict regarding their arithmetic operators. Given a vector `v` and normal `n`, 
the main functions are:

- **dot product with vector**:

  \f[v \cdot n = v_i n_i\f]
  ```cpp
  hermes::dot(v,n);
  hermes::dot(n,v);
  ```
  
- **reflection**:

  \f[v - 2 (v \cdot n)n\f]
  ```cpp
  hermes::reflect(v,n);
  ```

- **projection on surface**:

  \f[v - (v \cdot n)n\f]
  ```cpp
  hermes::project(v,n);
  ```

- **face vector forward normal direction**:
  
  \f[ \begin{cases}
  -v, && v \cdot n < 0 \\
  v, && v \cdot n >= 0 \\
  \end{cases}
  \f]
  
  ```cpp
  hermes::project(v,n);
  ```

## Transforms

### Matrices
