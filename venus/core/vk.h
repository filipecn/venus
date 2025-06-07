#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <volk.h>
#pragma GCC diagnostic pop

namespace venus::core {

class vk {
public:
  static vk &get();
  static void init();

  ~vk();
  vk &operator=(const vk &) = delete;

private:
  vk();
  void buildInstance();

  VkInstance vk_instance_{VK_NULL_HANDLE};
  VkDebugUtilsMessengerEXT vk_debug_messenger_{VK_NULL_HANDLE};
};

} // namespace venus::core
