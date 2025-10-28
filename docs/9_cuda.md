# CUDA

Hermes defines some macros to help you distinguish device code from
host code.
```cpp
#define HERMES_HOST_FUNCTION __host__
#define HERMES_DEVICE_CALLABLE __device__ __host__
#define HERMES_DEVICE_FUNCTION __device__
// you can check if there is CUDA support with:
#define HERMES_DEVICE_ENABLED
// Wraps a block of code that gets compiled only when using CUDA
#define HERMES_CUDA_CODE(CODE) {CODE}
```
> When you build `hermes` without `CUDA` support, all macros listed in this section get empty or are simply not defined.

An important tip to make sure a piece of code runs only in device is to use the following
preprocessor condition:
```cpp
#if (defined(__CUDA_ARCH__) && (__CUDA_ARCH__ > 0))
// this code runs only in device
#else
// this code runs only in host 
#endif
```

Here is how you can declare a `CUDA` kernel:
```cpp
// this macro creates a kernel called my_kernel_k
HERMES_CUDA_KERNEL(my_kernel)(int argument) {
  // kernel code   
}
```
> Note that the macro appends `_k` to your kernel's name

Usually your kernel will use thread indices that can be 1-dimensional, 2-dimensional or 3-dimensional
depending on you launch configurations. Here are some macros that create those indices for you:
```cpp
// creates a u32 i containing the thread index
HERMES_CUDA_THREAD_INDEX_I;
// creates a hermes::index2 ij containing the thread index
HERMES_CUDA_THREAD_INDEX_IJ;
// creates a hermes::index3 ijk containing the thread index
HERMES_CUDA_THREAD_INDEX_IK;  
// sometimes you may want the thread to execute only if its index is less then a size:
HERMES_CUDA_THREAD_INDEX_I_LT(BOUNDS);
HERMES_CUDA_THREAD_INDEX_IJ_LT(BOUNDS);
HERMES_CUDA_THREAD_INDEX_IJK_LT(BOUNDS);
// if you want to define the index variable name, use:
HERMES_CUDA_THREAD_INDEX_LT(I, BOUNDS);
HERMES_CUDA_THREAD_INDEX2_LT(IJ, BOUNDS);
HERMES_CUDA_THREAD_INDEX3_LT(IJK, BOUNDS);
```
For debugging purposes, you may want to quickly make the first thread the only thread to execute,
then you can use:
```cpp
HERMES_CUDA_RETURN_IF_NOT_THREAD_0
```
The most important thing you want to do is to check for errors, `hermes` lets you use:
```cpp
// check CUDA function returns
HERMES_CHECK_CUDA_CALL(err);
// for functions that do not return error codes or for kernel launches
// call it right after:
HERMES_CHECK_LAST_CUDA_CALL 
```
When launching kernels, `hermes::cuda_utils::LaunchInfo` holds launch information such
as number of threads, blocks, shared memory size and stream. It also
redistributes threads for you trying to optimize occupancy. The following macro
can be used to launch kernels:
```cpp
HERMES_CUDA_LAUNCH_AND_SYNC(LAUNCH_INFO, NAME, ...);
```
In this case, LAUNCH_INFO is the constructor parameters, surrounded by `()`, of `hermes::cuda_utils::LaunchInfo`.
Here is a complete example:
```cpp
#include <vector>
#include <hermes/common/cuda_utils.h>

// A kernel that stores in c, the sum of a and b
// All vectors have n elements
HERMES_CUDA_KERNEL(sum)(size_t n, float* a, float* b, float* c) {
  HERMES_CUDA_THREAD_INDEX_I_LT(n);
  c[i] = a[i] + b[i];
}

int main() {
  // lets create 3 arrays of size n
  size_t n = 1000;
  std::vector<float> host_a(n), host_b(n), host_c(n, 0);
  // fill a and b with numbers
  // ...
  // lets now allocate memory in device
  float *device_a, *device_b, *device_c;
  HERMES_CHECK_CUDA_CALL(cudaMalloc(&device_a, n * sizeof(float)));
  HERMES_CHECK_CUDA_CALL(cudaMalloc(&device_b, n * sizeof(float)));
  HERMES_CHECK_CUDA_CALL(cudaMalloc(&device_c, n * sizeof(float)));
  // now lets send the data to device
  HERMES_CHECK_CUDA_CALL(cudaMemcpy(device_a, &host_a[0], n * sizeof(float), cudaMemcpyHostToDevice));
  HERMES_CHECK_CUDA_CALL(cudaMemcpy(device_b, &host_b[0], n * sizeof(float), cudaMemcpyHostToDevice));
  HERMES_CHECK_CUDA_CALL(cudaMemcpy(device_c, &host_c[0], n * sizeof(float), cudaMemcpyHostToDevice));
  // and finally call the kernel
  HERMES_CUDA_LAUNCH_AND_SYNC((n), sum_k, n, device_a, device_b, device_c);
  return 0;
}
```
> Please check [storage classes](10_storage.md) for device arrays.

