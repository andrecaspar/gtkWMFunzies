#include <gtkmm.h>
#include <inttypes.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xcb_keysyms.h>

static xcb_connection_t *dpy;
static xcb_screen_t *scre;
static xcb_drawable_t win;
static uint32_t values[3];

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

static void handleButtonPress(xcb_generic_event_t *ev) {
  xcb_button_press_event_t *e = (xcb_button_press_event_t *)ev;
  win = e->child;
  values[0] = XCB_STACK_MODE_ABOVE;
  xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_STACK_MODE, values);
  values[2] = ((1 == e->detail) ? 1 : ((win != 0) ? 3 : 0));
  xcb_grab_pointer(dpy, 0, scre->root,
                   XCB_EVENT_MASK_BUTTON_RELEASE |
                       XCB_EVENT_MASK_BUTTON_MOTION |
                       XCB_EVENT_MASK_POINTER_MOTION_HINT,
                   XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, scre->root,
                   XCB_NONE, XCB_CURRENT_TIME);
}

static void handleMotionNotify(xcb_generic_event_t *ev) {
  xcb_query_pointer_cookie_t coord = xcb_query_pointer(dpy, scre->root);
  xcb_query_pointer_reply_t *poin = xcb_query_pointer_reply(dpy, coord, 0);
  if ((values[2] == (uint32_t)(1)) && (win != 0)) {
    xcb_get_geometry_cookie_t geom_now = xcb_get_geometry(dpy, win);
    xcb_get_geometry_reply_t *geom =
        xcb_get_geometry_reply(dpy, geom_now, NULL);
    uint16_t geom_x = geom->width + (2);
    uint16_t geom_y = geom->height + (2);
    values[0] = ((poin->root_x + geom_x) > scre->width_in_pixels)
                    ? (scre->width_in_pixels - geom_x)
                    : poin->root_x;
    values[1] = ((poin->root_y + geom_y) > scre->height_in_pixels)
                    ? (scre->height_in_pixels - geom_y)
                    : poin->root_y;
    xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
                         values);
  } else if ((values[2] == (uint32_t)(3)) && (win != 0)) {
    xcb_get_geometry_cookie_t geom_now = xcb_get_geometry(dpy, win);
    xcb_get_geometry_reply_t *geom =
        xcb_get_geometry_reply(dpy, geom_now, NULL);
    if (!((poin->root_x <= geom->x) || (poin->root_y <= geom->y))) {
      values[0] = poin->root_x - geom->x;
      values[1] = poin->root_y - geom->y;
      if ((values[0] >= (uint32_t)(10)) && (values[1] >= (uint32_t)(10))) {
        xcb_configure_window(dpy, win,
                             XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                             values);
      }
    }
  } else {
  }
}

void setup(void) {
  values[0] =
      XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
      XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_PROPERTY_CHANGE;
  xcb_change_window_attributes_checked(dpy, scre->root, XCB_CW_EVENT_MASK,
                                       values);
  xcb_ungrab_key(dpy, XCB_GRAB_ANY, scre->root, XCB_MOD_MASK_ANY);
  xcb_flush(dpy);
  xcb_grab_button(dpy, 0, scre->root,
                  XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
                  XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, scre->root,
                  XCB_NONE, 1, XCB_MOD_MASK_4);
  xcb_grab_button(dpy, 0, scre->root,
                  XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
                  XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, scre->root,
                  XCB_NONE, 3, XCB_MOD_MASK_4);
  xcb_flush(dpy);
}

void xThread(int argc, char *argv[]) {
  dpy = xcb_connect(NULL, 0);
  scre = xcb_setup_roots_iterator(xcb_get_setup(dpy)).data;
  setup();

  xcb_window_t window = xcb_generate_id(dpy);

  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[2] = {
      scre->white_pixel,
      XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS |
          XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
          XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
          XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE};

  xcb_create_window(dpy, 0,                        /* depth               */
                    window, scre->root,            /* parent window       */
                    150, 150,                      /* x, y                */
                    150, 150,                      /* width, height       */
                    10,                            /* border_width        */
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
                    scre->root_visual,             /* visual              */
                    mask, values);                 /* masks */

  xcb_map_window(dpy, window);

  xcb_cursor_context_t *pcursorctx;
  xcb_cursor_context_new(dpy, scre, &pcursorctx);
  xcb_cursor_t cursor = xcb_cursor_load_cursor(pcursorctx, "left_ptr");
  xcb_change_window_attributes(dpy, scre->root, XCB_CW_CURSOR, &cursor);

  xcb_flush(dpy);

  xcb_generic_event_t *event;
  while ((event = xcb_wait_for_event(dpy))) {
    switch (event->response_type) {
    case XCB_BUTTON_PRESS: {
      handleButtonPress(event);
      break;
    }
    case XCB_MOTION_NOTIFY: {
      handleMotionNotify(event);
      break;
    }
    }

    free(event);
  }
}

void gtkThread(int argc, char *argv[]) {
  auto application = Gtk::Application::create("hmm");
  application->make_window_and_run<WindowMain>(argc, argv);
}

int main(int argc, char *argv[]) {
  std::thread threadTest(gtkThread, argc, argv);
  std::thread threadTest2(xThread, argc, argv);
  threadTest.join();
  threadTest2.join();

  return 0;
}
