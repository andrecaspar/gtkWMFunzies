#include <gtkmm.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <iostream>

class WindowMain : public Gtk::Window {
public:
  void On_button_clicked() { std::cout << "test" << std::endl; }

  void css() {
    GtkCssProvider *provider;
    GdkDisplay *display;

    provider = gtk_css_provider_new();
    display = gdk_display_get_default();
    gtk_style_context_add_provider_for_display(
        display, GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_css_provider_load_from_file(provider, g_file_new_for_path("style.css"));
    g_object_unref(provider);
  }

  WindowMain() {
    set_decorated(false);
    button1.set_label("button1");
    button1.signal_clicked().connect(
        sigc::mem_fun(*this, &WindowMain::On_button_clicked));
    button1.set_margin(0);

    set_title("Button example");
    set_child(button1);

    css();
  }

private:
  Gtk::Button button1;
};

int main(int argc, char *argv[]) {
  Display *dpy;
  Window root;
  XWindowAttributes attr;
  XButtonEvent start;
  XEvent ev;

  if (!(dpy = XOpenDisplay(0)))
    return 1;

  root = DefaultRootWindow(dpy);

  Cursor cursor = XCreateFontCursor(dpy, XC_left_ptr);
  XDefineCursor(dpy, root, cursor);

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("F1")), Mod1Mask, root,
           True, GrabModeAsync, GrabModeAsync);
  XGrabButton(dpy, 1, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync,
              GrabModeAsync, None, None);
  XGrabButton(dpy, 3, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync,
              GrabModeAsync, None, None);

  for (;;) {
    auto application = Gtk::Application::create("test");
    application->make_window_and_run<WindowMain>(argc, argv);
    XNextEvent(dpy, &ev);
    if (ev.type == KeyPress && ev.xkey.subwindow != None)
      XRaiseWindow(dpy, ev.xkey.subwindow);
    else if (ev.type == ButtonPress && ev.xbutton.subwindow != None) {
      XGrabPointer(dpy, ev.xbutton.subwindow, True,
                   PointerMotionMask | ButtonReleaseMask, GrabModeAsync,
                   GrabModeAsync, None, None, CurrentTime);
      XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
      start = ev.xbutton;
    } else if (ev.type == MotionNotify) {
      int xdiff, ydiff;
      while (XCheckTypedEvent(dpy, MotionNotify, &ev))
      xdiff = ev.xbutton.x_root - start.x_root;
      ydiff = ev.xbutton.y_root - start.y_root;
      XMoveResizeWindow(dpy, ev.xmotion.window,
                        attr.x + (start.button == 1 ? xdiff : 0),
                        attr.y + (start.button == 1 ? ydiff : 0),
                        MAX(1, attr.width + (start.button == 3 ? xdiff : 0)),
                        MAX(1, attr.height + (start.button == 3 ? ydiff : 0)));
    } else if (ev.type == ButtonRelease)
      XUngrabPointer(dpy, CurrentTime);
  }
}
