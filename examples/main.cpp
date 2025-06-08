#include <ios>
#include <iostream>
#include <venus/core/instance.h>
#include <venus/core/time.h>
#include <venus/core/vk.h>

int main() {
  venus::core::SystemTime::init();
  venus::core::vk::init();
  venus::core::Instance instance("hello_vulkan_app", {}, {});
  auto devices = instance.enumerateAvailablePhysicalDevices();
  std::cout << devices.size() << std::endl;
  return 0;
}
