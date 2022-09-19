#include <memory>
#include <X11/Xlib.h>

class WindowManager {
private:
  Display *display;

public:
  Display* getDisplay() {
      return this->display;
  }

  void setDisplay(Display* display) {
      this->display = display;
  }

  void run();
  void init();
};
