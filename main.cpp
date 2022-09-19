#include "WindowManager.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  std::unique_ptr<WindowManager> windowManager = std::make_unique<WindowManager>();
  windowManager->init();
  std::cout << "\n\n\n\naaaaaaaaaaaaaaaaaaaaaaaaa" << "\n\n\n\n" << std::endl;
  std::cout << "\n\n\n\n" << windowManager->getDisplay() << "\n\n\n\n" << std::endl;
  windowManager->run();
  return 0;
}
