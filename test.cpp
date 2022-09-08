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

int x11_fd;
fd_set in_fds;

struct timeval tv;
XEvent ev;

int tempMotionX = 0, tempMotionY = 0;

void HandleMapRequest(XEvent *event) {
  Window mapWindow = event->xmaprequest.window;
  XSelectInput(d, mapWindow,
               ExposureMask | StructureNotifyMask | EnterWindowMask |
                   LeaveWindowMask);
  XGrabButton(d, AnyButton, AnyModifier, mapWindow, True, PointerMotionMask,
              GrabModeAsync, GrabModeAsync, 0, 0);
  XMapWindow(d, mapWindow);
}

void HandleMotionNotify(XEvent *event) {
  int xd = event->xmotion.x - buttonClick.x;
  int yd = event->xmotion.y - buttonClick.y;

  XWindowAttributes windowAttrib;
  XGetWindowAttributes(d, windowClick, &windowAttrib);
  XMoveResizeWindow(d, windowClick, xd, yd, windowAttrib.height,
                    windowAttrib.width);

  tempMotionX = event->xmotion.x;
  tempMotionY = event->xmotion.y;
}

void HandleCreateNotify(XEvent *event) {
  Window createdWindow = event->xcreatewindow.window;
  XSelectInput(d, createdWindow,
               ExposureMask | StructureNotifyMask | EnterWindowMask |
                   LeaveWindowMask);
  XGrabButton(d, AnyButton, AnyModifier, createdWindow, True, PointerMotionMask,
              GrabModeAsync, GrabModeAsync, 0, 0);
  XSync(d, 0);
}

void HandleButtonPress(XEvent *event) {
  windowClick = event->xbutton.window;
  buttonClick = event->xbutton;
}

void HandleConfigureNotify(XEvent *event) {
  Window configWindow = event->xconfigure.window;
  windowClick =configWindow;
  XSelectInput(d, configWindow,
               ExposureMask | StructureNotifyMask | EnterWindowMask |
                   LeaveWindowMask);
  XGrabButton(d, AnyButton, AnyModifier, configWindow, True, PointerMotionMask,
              GrabModeAsync, GrabModeAsync, 0, 0);
  XSync(d, 0);
}

void HandleExpose(XEvent *event) {
  Window exposeWindow = event->xexpose.window;
  windowClick = exposeWindow;
  XSelectInput(d, exposeWindow,
               ExposureMask | StructureNotifyMask | EnterWindowMask |
                   LeaveWindowMask);
  XGrabButton(d, AnyButton, AnyModifier, exposeWindow, True, PointerMotionMask,
              GrabModeAsync, GrabModeAsync, 0, 0);
  XSync(d, 0);
}

void HandleSelectionNotify(XEvent *event) {
  Window selectWindow = event->xselection.requestor;
  windowClick = selectWindow;
  XSelectInput(d, selectWindow,
               ExposureMask | StructureNotifyMask | EnterWindowMask |
                   LeaveWindowMask);
  XGrabButton(d, AnyButton, AnyModifier, selectWindow, True, PointerMotionMask,
              GrabModeAsync, GrabModeAsync, 0, 0);
  XSync(d, 0);
}

void threadGtk() {
  sleep(5);
  GtkApplication *app;
  app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);

  GtkWidget *window;
  GtkWidget *button;
  GtkWidget *box;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

  gtk_window_set_child(GTK_WINDOW(window), box);

  button = gtk_button_new_with_label("Hello World");

  gtk_box_append(GTK_BOX(box), button);

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
  XEvent event;
  x11_fd = ConnectionNumber(d);
  std::thread _threadGtk(threadGtk);
  while (1) {
    FD_ZERO(&in_fds);
    FD_SET(x11_fd, &in_fds);

    tv.tv_usec = 0;
    tv.tv_sec = 1;

    int num_ready_fds = select(x11_fd + 1, &in_fds, NULL, NULL, &tv);
    if (num_ready_fds > 0) {
      XPeekEvent(d, &event);
      printf("\n\n\n%i", event.type);
      switch (event.type) {
      case (MapRequest):
        HandleMapRequest(&event);
        break;
      case (ButtonPress):
        HandleButtonPress(&event);
        break;
      case (SelectionNotify):
        HandleSelectionNotify(&event);
        break;
      case (MotionNotify):
        HandleMotionNotify(&event);
        break;
      case (Expose):
        HandleExpose(&event);
        break;
      case (ConfigureNotify):
        HandleConfigureNotify(&event);
        break;
      case (CreateNotify):
        printf("\n\n\n\nCREATEEEEEEEEEEEEEEEEEEE\n\n\n\n\n");
        HandleCreateNotify(&event);
        break;
      default:
        break;
      }
    }
  }
  XSync(d, 0);
}

void X() {
  int screen = DefaultScreen(d);
  Window root = DefaultRootWindow(d);
  XSelectInput(d, root,
               PropertyChangeMask | SubstructureRedirectMask |
                   SubstructureNotifyMask);

  //Window w = XCreateSimpleWindow(d, RootWindow(d, screen), 200, 10, 100, 100, 1,
                                 //BlackPixel(d, screen), WhitePixel(d, screen));
  //XGrabButton(d, AnyButton, AnyModifier, w, True, PointerMotionMask,
              //GrabModeAsync, GrabModeAsync, 0, 0);
  //XSelectInput(d, w, ExposureMask | KeyPressMask);
  //XMapWindow(d, w);

  Cursor cursor = XCreateFontCursor(d, XC_left_ptr);
  XSetWindowAttributes wa;
  wa.cursor = cursor;
  XChangeWindowAttributes(d, root, CWCursor, &wa);
  XSync(d, 0);

  std::thread _threadTest(threadTest);
  _threadTest.join();
}

int main(int argc, char **argv) {
  gtk_init();

  gd = gdk_display_get_default();
  d = GDK_DISPLAY_XDISPLAY(gd);

  X();
}
