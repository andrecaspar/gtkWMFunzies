#include <gtk/gtk.h>
#include <gtkmm.h>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <gdk/x11/gdkx.h>

GdkDisplay *gd;
Display *d;
XButtonEvent buttonClick;
Window windowClick;

fd_set in_fds;

struct timeval tv;
XEvent ev;

int tempMotionX = 0, tempMotionY = 0;

void HandleMapRequest(XEvent *event) {
  Window mapWindow = event->xmaprequest.window;
  XMapWindow(d, mapWindow);
  XSelectInput(d, mapWindow,
               ExposureMask | StructureNotifyMask | EnterWindowMask |
                   LeaveWindowMask);
  XGrabButton(d, AnyButton, Mod1Mask, mapWindow, True, PointerMotionMask,
              GrabModeAsync, GrabModeAsync, 0, 0);
}

void HandleMotionNotify(XEvent *event) {
  int xd = event->xmotion.x - buttonClick.x;
  int yd = event->xmotion.y - buttonClick.y;

  XWindowAttributes windowAttrib;
  XGetWindowAttributes(d, windowClick, &windowAttrib);
  XMoveResizeWindow(
      d, windowClick, windowAttrib.x + (buttonClick.button == 1 ? xd : 0),
      windowAttrib.y + (buttonClick.button == 1 ? yd : 0),
      MAX(windowAttrib.width +
              (buttonClick.button == 3 ? event->xmotion.x - tempMotionX : 0),
          30),
      MAX(windowAttrib.height +
              (buttonClick.button == 3 ? event->xmotion.y - tempMotionY : 0),
          30));

  tempMotionX = event->xmotion.x;
  tempMotionY = event->xmotion.y;
}

void HandleButtonPress(XEvent *event) {
  windowClick = event->xbutton.window;
  buttonClick = event->xbutton;
}

void threadGtk() {
  GtkApplication *app;
  app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  GtkWidget *window = gtk_application_window_new(app);

  GtkWidget *button = gtk_button_new_with_label("Button");
  gtk_window_set_child(GTK_WINDOW(window), button);

  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
  gtk_widget_show(window);

  GtkCssProvider *provider;
  provider = gtk_css_provider_new();
  gtk_style_context_add_provider_for_display(
      gd, GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  gtk_css_provider_load_from_file(provider, g_file_new_for_path("style.css"));
  g_object_unref(provider);

  while (g_main_context_pending(NULL) || true) {
    g_main_context_iteration(NULL, true);
  }
}

void threadTest() {
    int x11_fd = ConnectionNumber(d);
    XEvent event;
    while(1) {
        FD_ZERO(&in_fds);
        FD_SET(x11_fd, &in_fds);

        tv.tv_usec = 0;
        tv.tv_sec = 1;

        int num_ready_fds = select(x11_fd + 1, &in_fds, NULL, NULL, &tv);
        if (num_ready_fds > 0) {
            XPeekEvent(d, &event);
            printf("\n %i ", event.type);
            XFlush(d);
        }
    }
}

int main(int argc, char **argv) {
  gtk_init();

  gd = gdk_display_get_default();
  d = GDK_DISPLAY_XDISPLAY(gd);

  std::thread _threadGtk(threadGtk);

  int screen = DefaultScreen(d);
  Window root = DefaultRootWindow(d);
  XSelectInput(d, root,
               PropertyChangeMask | SubstructureRedirectMask |
                   SubstructureNotifyMask);

  Window w = XCreateSimpleWindow(d, RootWindow(d, screen), 200, 10, 100, 100, 1,
                                 BlackPixel(d, screen), WhitePixel(d, screen));
  XSelectInput(d, w, ExposureMask | KeyPressMask);
  XMapWindow(d, w);

  Cursor cursor = XCreateFontCursor(d, XC_left_ptr);
  XSetWindowAttributes wa;
  wa.cursor = cursor;
  XChangeWindowAttributes(d, root, CWCursor, &wa);

  std::thread _threadTest(threadTest);
  _threadTest.join();
}
