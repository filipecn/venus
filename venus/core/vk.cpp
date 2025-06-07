#include <venus/core/debug.h>
#include <venus/core/vk.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#pragma GCC diagnostic pop

#include <cstdlib>

namespace venus::core {

vk &vk::get() {
  static vk s_vk;
  return s_vk;
}

vk::vk() = default;
vk::~vk() = default;

void vk::init() {
  auto &v = get();
  v.buildInstance();
}

void vk::buildInstance() {
  VkResult r;
  uint32_t version;
  void *ptr;

  ptr =
#if defined(_WIN32)
      &vkCreateWin32SurfaceKHR;
#elif defined(__linux__) || defined(__unix__)
      &vkCreateXlibSurfaceKHR;
#elif defined(__APPLE__)
      &vkCreateMacOSSurfaceMVK;
#else
      NULL;
#endif

  VENUS_UNUSED_VARIABLE(ptr);

  r = volkInitialize();
  if (r != VK_SUCCESS) {
    VENUS_ERROR("volkInitialize failed!");
    return;
  }

  version = volkGetInstanceVersion();
  VENUS_INFO("Vulkan version {}.{}.{} initialized.", VK_VERSION_MAJOR(version),
             VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));
}

} // namespace venus::core
