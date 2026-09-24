#include "base/base.h"
#include "wm/wm.h"

#include "base/base.c"
#include "wm/wm.c"

internal void
entry_point(int argc, char **argv) {
  String8 window_name = s("misery");
  Vector2 window_size = vec2(800, 600);

  WM_Window window = wm_window_open(window_name, window_size);
  wm_window_first_paint(window);

  for (b32 should_quit = 0; !should_quit;) {
    Temp scratch = scratch_begin(0, 0);
    WM_Event_List events = wm_get_events(scratch.arena, 0);
    for_each_node(WM_Event, event, events.first) {
      switch (event->type) {
        case WM_Event_Type_WINDOW_CLOSE: {
          should_quit = 1;
        } break;
      }
    }
    scratch_end(scratch);
  }

  wm_window_close(window);
}
