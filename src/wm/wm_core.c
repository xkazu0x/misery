////////////////////////////////
// NOTE: Handle Type Helpers

internal WM_Window
wm_window_zero(void) {
  WM_Window result = {0};
  return(result);
}

internal b32
wm_window_match(WM_Window a, WM_Window b) {
  b32 result = memory_match_struct(&a, &b);
  return(result);
}

////////////////////////////////
// NOTE: Event Helpers

internal WM_Event *
wm_event_list_push(Arena *arena, WM_Event_List *list, WM_Event_Type type) {
  WM_Event *event = push_array(arena, WM_Event, 1);
  dll_push_back(list->first, list->last, event);
  list->count += 1;
  event->timestamp_us = get_time_us();
  event->type = type;
  return(event);
}

internal void
wm_eat_event(WM_Event_List *list, WM_Event *event) {
  dll_remove(list->first, list->last, event);
  list->count -= 1;
}
