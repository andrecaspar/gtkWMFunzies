#include <gtkmm.h>
#include <inttypes.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>

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

void test(int argc, char *argv[]) {
  auto application = Gtk::Application::create("test");
  application->make_window_and_run<WindowMain>(argc, argv);
}

void test2() {
  xcb_connection_t *connection = xcb_connect(NULL, NULL);

  /* Get the first screen */
  xcb_screen_t *screen =
      xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

  /* Create the window */
  xcb_window_t window = xcb_generate_id(connection);

  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[2] = {
      screen->white_pixel,
      XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS |
          XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
          XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
          XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE};

  xcb_create_window(connection, 0,                 /* depth               */
                    window, screen->root,          /* parent window       */
                    0, 0,                          /* x, y                */
                    150, 150,                      /* width, height       */
                    10,                            /* border_width        */
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
                    screen->root_visual,           /* visual              */
                    mask, values);                 /* masks */

  /* Map the window on the screen */
  xcb_map_window(connection, window);

  xcb_cursor_context_t *pcursorctx;
  xcb_cursor_context_new(connection, screen, &pcursorctx);
  xcb_cursor_t cursor = xcb_cursor_load_cursor(pcursorctx, "left_ptr");
  xcb_change_window_attributes(connection, screen->root, XCB_CW_CURSOR,
                               &cursor);

  xcb_flush(connection);

  xcb_generic_event_t *event;
  while ((event = xcb_wait_for_event(connection))) {
    switch (event->response_type & ~0x80) {
    case XCB_EXPOSE: {
      xcb_expose_event_t *expose = (xcb_expose_event_t *)event;

      printf("Window %" PRIu32
             " exposed. Region to be redrawn at location (%" PRIu16 ",%" PRIu16
             "), with dimension (%" PRIu16 ",%" PRIu16 ")\n",
             expose->window, expose->x, expose->y, expose->width,
             expose->height);
      break;
    }
    case XCB_BUTTON_PRESS: {
      xcb_button_press_event_t *bp = (xcb_button_press_event_t *)event;

      xcb_configure_window(connection,bp->event, XCB_CONFIG_WINDOW_X, &bp->root_x);

      switch (bp->detail) {
      case 4:
        printf("Wheel Button up in window %" PRIu32 ", at coordinates (%" PRIi16
               ",%" PRIi16 ")\n",
               bp->event, bp->event_x, bp->event_y);
        break;
      case 5:
        printf("Wheel Button down in window %" PRIu32
               ", at coordinates (%" PRIi16 ",%" PRIi16 ")\n",
               bp->event, bp->event_x, bp->event_y);
        break;
      default:
        printf("Button %" PRIu8 " pressed in window %" PRIu32
               ", at coordinates (%" PRIi16 ",%" PRIi16 ")\n",
               bp->detail, bp->event, bp->event_x, bp->event_y);
        break;
      }
      break;
    }
    case XCB_BUTTON_RELEASE: {
      xcb_button_release_event_t *br = (xcb_button_release_event_t *)event;

      printf("Button %" PRIu8 " released in window %" PRIu32
             ", at coordinates (%" PRIi16 ",%" PRIi16 ")\n",
             br->detail, br->event, br->event_x, br->event_y);
      break;
    }
    case XCB_MOTION_NOTIFY: {
      xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t *)event;

      printf("Mouse moved in window %" PRIu32 ", at coordinates (%" PRIi16
             ",%" PRIi16 ")\n",
             motion->event, motion->event_x, motion->event_y);
      break;
    }
    case XCB_ENTER_NOTIFY: {
      xcb_enter_notify_event_t *enter = (xcb_enter_notify_event_t *)event;

      printf("Mouse entered window %" PRIu32 ", at coordinates (%" PRIi16
             ",%" PRIi16 ")\n",
             enter->event, enter->event_x, enter->event_y);
      break;
    }
    case XCB_LEAVE_NOTIFY: {
      xcb_leave_notify_event_t *leave = (xcb_leave_notify_event_t *)event;

      printf("Mouse left window %" PRIu32 ", at coordinates (%" PRIi16
             ",%" PRIi16 ")\n",
             leave->event, leave->event_x, leave->event_y);
      break;
    }
    case XCB_KEY_PRESS: {
      xcb_key_press_event_t *kp = (xcb_key_press_event_t *)event;

      printf("Key pressed in window %" PRIu32 "\n", kp->event);
      break;
    }
    case XCB_KEY_RELEASE: {
      xcb_key_release_event_t *kr = (xcb_key_release_event_t *)event;

      printf("Key released in window %" PRIu32 "\n", kr->event);
      break;
    }
    default:
      /* Unknown event type, ignore it */
      printf("Unknown event: %" PRIu8 "\n", event->response_type);
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
