#include <iostream>
#include <venus/core/time.h>
#include <venus/core/vk.h>

int main() {
  venus::core::SystemTime::init();
  venus::core::vk::init();

  return 0;
}
