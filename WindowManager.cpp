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

#include "WindowManager.hpp"

GdkDisplay *gd;
XButtonEvent buttonClick;
Window windowClick;
int screen;

struct timeval tv;
XEvent ev;

int tempMotionX = 0, tempMotionY = 0;

void threadGtk() {
  GtkBuilder *builder = gtk_builder_new_from_file("/home/a/test.xml");

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

void WindowManager::init() {
  setDisplay(XOpenDisplay(NULL));
  XSetWindowAttributes wa;
  screen = DefaultScreen(display);
  wa.cursor = XCreateFontCursor(display, XC_left_ptr);
  wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask |
                  ButtonPressMask | PointerMotionMask | EnterWindowMask |
                  LeaveWindowMask | StructureNotifyMask | PropertyChangeMask;
  XChangeWindowAttributes(display, RootWindow(display, screen),
                          CWEventMask | CWCursor, &wa);
  XSelectInput(display, RootWindow(display, screen), wa.event_mask);

  gtk_init();

  gd = gdk_display_get_default();

  Window w = XCreateSimpleWindow(display, RootWindow(display, screen), 200, 10,
                                 100, 100, 1, BlackPixel(display, screen),
                                 WhitePixel(display, screen));
  XGrabButton(display, AnyButton, Mod4Mask, w, True, PointerMotionMask,
              GrabModeAsync, GrabModeAsync, 0, 0);
  XSelectInput(display, w, ExposureMask | KeyPressMask);
  XMapWindow(display, w);
}

void WindowManager::run() {
  std::thread _threadGtk(threadGtk);
  XEvent ev;
  XSync(getDisplay(), False);
  while (true && !XNextEvent(getDisplay(), &ev))
    switch (ev.type) {
    default:
      break;
    }
}
