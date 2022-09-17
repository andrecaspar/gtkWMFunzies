#include <gtk/gtk.h>
#include <gtkmm.h>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <gdk/x11/gdkx.h>

GdkDisplay *gd;
Display *dpy;
XButtonEvent buttonClick;
Window windowClick;
int screen;

struct timeval tv;
XEvent ev;

int tempMotionX = 0, tempMotionY = 0;

void manage(Window w, XWindowAttributes *wa) {
  XGrabButton(dpy, AnyButton, AnyModifier, w, True, PointerMotionMask,
              GrabModeAsync, GrabModeAsync, 0, 0);
  XSelectInput(dpy, w,
               EnterWindowMask | KeyPressMask | FocusChangeMask |
                   PropertyChangeMask | StructureNotifyMask);
  XMapWindow(dpy, w);
}

void HandleMapRequest(XEvent *event) {
  static XWindowAttributes wa;
  XMapRequestEvent *ev = &event->xmaprequest;
  manage(ev->window, &wa);
}

static void client_resize_absolute(Window window, int w, int h) {
  XResizeWindow(dpy, window, MAX(w, 10), MAX(h, 10));
}

static void client_move_absolute(Window window, int x, int y) {
  XMoveWindow(dpy, window, x, y);
}

static void HandleButtonPress(XEvent *e) {
  XButtonPressedEvent *bev = &e->xbutton;
  XEvent ev;
  int x, y, ocx, ocy, nx, ny, nw, nh, di, ocw, och;
  unsigned int dui;
  Window dummy;
  Time current_time, last_motion;

  XQueryPointer(dpy, RootWindow(dpy, screen), &dummy, &dummy, &x, &y, &di, &di,
                &dui);

  XWindowAttributes windowAttribs;
  XGetWindowAttributes(dpy, bev->window, &windowAttribs);

  ocx = windowAttribs.x;
  ocy = windowAttribs.y;
  ocw = windowAttribs.width;
  och = windowAttribs.height;
  last_motion = ev.xmotion.time;
  Cursor move_cursor = XCreateFontCursor(dpy, XC_crosshair);
  if (XGrabPointer(dpy, RootWindow(dpy, screen), False,
                   PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                   GrabModeAsync, GrabModeAsync, None, move_cursor,
                   CurrentTime) != GrabSuccess)
    return;
  do {
    XMaskEvent(dpy,
               PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                   ExposureMask | SubstructureRedirectMask,
               &ev);
    switch (ev.type) {
    case ConfigureRequest:
      // handle
      break;
    case Expose:
      // handle
      break;
    case MapRequest:
      HandleMapRequest(&ev);
      break;
    case MotionNotify:
      current_time = ev.xmotion.time;
      Time diff_time = current_time - last_motion;
      if (diff_time < (Time)1) {
        continue;
      }
      last_motion = current_time;
      if (ev.xbutton.state == (unsigned)(Button1Mask) ||
          ev.xbutton.state == Button1Mask) {
        nx = ocx + (ev.xmotion.x - x);
        ny = ocy + (ev.xmotion.y - y);
        client_move_absolute(bev->window, nx, ny);
      } else if (ev.xbutton.state == (unsigned)(Button1Mask)) {
        nw = ev.xmotion.x - x;
        nh = ev.xmotion.y - y;
        client_resize_absolute(bev->window, ocw + nw, och + nh);
      }
      XFlush(dpy);
      break;
    }
  } while (ev.type != ButtonRelease);
  XUngrabPointer(dpy, CurrentTime);
}

void HandleConfigureRequest(XEvent *e) {
  XConfigureRequestEvent *ev = &e->xconfigurerequest;
  XWindowChanges wc;
  wc.x = ev->x;
  wc.y = ev->y;
  wc.width = ev->width;
  wc.height = ev->height;
  wc.border_width = ev->border_width;
  wc.sibling = ev->above;
  wc.stack_mode = ev->detail;
  XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
  XSync(dpy, False);
}

void threadGtk() {
  GtkApplication *app;
  app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);

  GtkBuilder *builder = gtk_builder_new_from_file("test.xml");

  GObject *window = gtk_builder_get_object(builder, "main_window");
  gtk_widget_show(GTK_WIDGET(window));

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

void run(void) {
  XEvent ev;
  XSync(dpy, False);
  while (true && !XNextEvent(dpy, &ev))
    switch (ev.type) {
    case MapRequest:
      HandleMapRequest(&ev);
      break;
    case ButtonPress:
      HandleButtonPress(&ev);
      break;
    case ConfigureRequest:
      HandleConfigureRequest(&ev);
      break;
    default:
      break;
    }
}

void setup(void) {
  XSetWindowAttributes wa;
  screen = DefaultScreen(dpy);
  wa.cursor = XCreateFontCursor(dpy, XC_left_ptr);
  wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask |
                  ButtonPressMask | PointerMotionMask | EnterWindowMask |
                  LeaveWindowMask | StructureNotifyMask | PropertyChangeMask;
  XChangeWindowAttributes(dpy, RootWindow(dpy, screen), CWEventMask | CWCursor,
                          &wa);
  XSelectInput(dpy, RootWindow(dpy, screen), wa.event_mask);
}

int main(int argc, char **argv) {
  dpy = XOpenDisplay(NULL);
  setup();

  gtk_init();

  gd = gdk_display_get_default();

  std::thread _threadGtk(threadGtk);

  Window w =
      XCreateSimpleWindow(dpy, RootWindow(dpy, screen), 200, 10, 100, 100, 1,
                          BlackPixel(dpy, screen), WhitePixel(dpy, screen));
  XGrabButton(dpy, AnyButton, AnyModifier, w, True, PointerMotionMask,
              GrabModeAsync, GrabModeAsync, 0, 0);
  XSelectInput(dpy, w, ExposureMask | KeyPressMask);
  XMapWindow(dpy, w);

  run();
}
