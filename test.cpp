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

void threadX() {
  int screen = DefaultScreen(d);
  Window root = DefaultRootWindow(d);
  XSelectInput(d, root,
               PropertyChangeMask | SubstructureRedirectMask |
                   SubstructureNotifyMask);

  Window w = XCreateSimpleWindow(d, RootWindow(d, screen), 200, 10, 100, 100, 1,
                                 BlackPixel(d, screen), WhitePixel(d, screen));
  XMapWindow(d, w);

  Cursor cursor = XCreateFontCursor(d, XC_left_ptr);
  XSetWindowAttributes wa;
  wa.cursor = cursor;
  XChangeWindowAttributes(d, root, CWCursor, &wa);

  std::thread _threadGtk(threadGtk);
  _threadGtk.join();

  while (true) {
    XEvent event;
    XNextEvent(d, &event);
  }
}

int main(int argc, char **argv) {
  gtk_init();

  gd = gdk_display_get_default();
  d = GDK_DISPLAY_XDISPLAY(gd);

  threadX();
}
