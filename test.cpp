#include <gtkmm.h>
#include <inttypes.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>

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

static void handleMotionNotify(xcb_generic_event_t *ev) {
  values[0] = 1000;
  values[1] = 200;
  xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
                       values);
  
	xcb_flush(dpy);
}

static void handleButtonPress(xcb_generic_event_t *ev) {
  xcb_button_press_event_t *e = (xcb_button_press_event_t *)ev;
  win = e->child;
  values[0] = XCB_STACK_MODE_ABOVE;
  xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_STACK_MODE, values);
  values[2] = (1 == e->detail) ? 1 : 3;
  xcb_grab_pointer(dpy, 0, scre->root,
                   XCB_EVENT_MASK_BUTTON_RELEASE |
                       XCB_EVENT_MASK_BUTTON_MOTION |
                       XCB_EVENT_MASK_POINTER_MOTION_HINT,
                   XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, scre->root,
                   XCB_NONE, XCB_CURRENT_TIME);
}

void test(int argc, char *argv[]) {
  auto application = Gtk::Application::create("test");
  application->make_window_and_run<WindowMain>(argc, argv);
}

void test2() {
  dpy = xcb_connect(NULL, NULL);

  /* Get the first scre */
  scre = xcb_setup_roots_iterator(xcb_get_setup(dpy)).data;

  /* Create the window */
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
                    0, 0,                          /* x, y                */
                    150, 150,                      /* width, height       */
                    10,                            /* border_width        */
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
                    scre->root_visual,             /* visual              */
                    mask, values);                 /* masks */

  /* Map the window on the scre */
  xcb_map_window(dpy, window);

  xcb_cursor_context_t *pcursorctx;
  xcb_cursor_context_new(dpy, scre, &pcursorctx);
  xcb_cursor_t cursor = xcb_cursor_load_cursor(pcursorctx, "left_ptr");
  xcb_change_window_attributes(dpy, scre->root, XCB_CW_CURSOR, &cursor);

  xcb_flush(dpy);

  xcb_generic_event_t *event;
  while ((event = xcb_wait_for_event(dpy))) {
    switch (event->response_type) {
    case XCB_EXPOSE: {
      xcb_expose_event_t *expose = (xcb_expose_event_t *)event;
      break;
    }
    case XCB_BUTTON_PRESS: {
      handleButtonPress(event);
      break;
    }
    case XCB_BUTTON_RELEASE: {
      xcb_button_release_event_t *br = (xcb_button_release_event_t *)event;
      break;
    }
    case XCB_MOTION_NOTIFY: {
      handleMotionNotify(event);
      break;
    }
    case XCB_ENTER_NOTIFY: {
      xcb_enter_notify_event_t *enter = (xcb_enter_notify_event_t *)event;
      break;
    }
    case XCB_LEAVE_NOTIFY: {
      xcb_leave_notify_event_t *leave = (xcb_leave_notify_event_t *)event;
      break;
    }
    case XCB_KEY_PRESS: {
      xcb_key_press_event_t *kp = (xcb_key_press_event_t *)event;
      break;
    }
    case XCB_KEY_RELEASE: {
      xcb_key_release_event_t *kr = (xcb_key_release_event_t *)event;
      break;
    }
    default:
      break;
    }

    free(event);
  }
}

int main(int argc, char *argv[]) {

  std::thread threadTest(test, argc, argv);
  std::thread threadTest2(test2);

  threadTest.join();
  threadTest2.join();

  return 0;
}
