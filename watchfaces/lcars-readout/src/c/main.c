#include <pebble.h>
#include "lcars_theme.h"

static Window *s_window;

// Custom-drawn LCARS frame: elbow column, top date bar, decorative blocks.
// TODO: implement drawing of the elbow (two stacked color blocks with a
// rounded top-left corner) and the top bar in this layer's update_proc,
// using LCARS_COLOR_* from lcars_theme.h and graphics_fill_rect with a
// GCornerMask for the rounded corners.
static Layer *s_frame_layer;

// Light panel showing the time in large, bold, black text.
// TODO: back this with a rounded-rect drawn layer (LCARS_COLOR_PANEL_BG)
// plus a TextLayer for the "HH:MM" string, updated on MINUTE_UNIT ticks.
static TextLayer *s_time_layer;

// Light panel showing the date (weekday + day + month) in the top bar.
// TODO: back with LCARS_COLOR_TOP_BAR label bar + black text.
static TextLayer *s_date_layer;

// Secondary light panels: battery percentage and Bluetooth status.
// TODO: subscribe to battery_state_service and connection_service, and
// render each as a labeled light panel per the layout in PLAN.md.
static TextLayer *s_battery_layer;
static TextLayer *s_bluetooth_layer;

static void frame_layer_update_proc(Layer *layer, GContext *ctx) {
  // TODO: draw the elbow column and top bar per PLAN.md's layout.
  graphics_context_set_fill_color(ctx, LCARS_COLOR_BACKGROUND);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void update_time(struct tm *tick_time) {
  static char s_time_buffer[6];
  strftime(s_time_buffer, sizeof(s_time_buffer),
           clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_time_buffer);

  static char s_date_buffer[16];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a %d %b", tick_time);
  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

static void battery_handler(BatteryChargeState charge_state) {
  static char s_battery_buffer[8];
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge_state.charge_percent);
  text_layer_set_text(s_battery_layer, s_battery_buffer);
}

static void bluetooth_handler(bool connected) {
  text_layer_set_text(s_bluetooth_layer, connected ? "BT" : "--");
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_frame_layer = layer_create(bounds);
  layer_set_update_proc(s_frame_layer, frame_layer_update_proc);
  layer_add_child(window_layer, s_frame_layer);

  // TODO: replace these placeholder frames with the panel positions from
  // PLAN.md (main time panel, battery/Bluetooth panels side by side, date
  // in the top bar), each with LCARS_COLOR_PANEL_BG behind it.
  s_time_layer = text_layer_create(GRect(ELBOW_WIDTH, TOP_BAR_HEIGHT, bounds.size.w - ELBOW_WIDTH, 60));
  text_layer_set_background_color(s_time_layer, LCARS_COLOR_PANEL_BG);
  text_layer_set_text_color(s_time_layer, LCARS_COLOR_PANEL_TEXT);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  s_date_layer = text_layer_create(GRect(ELBOW_WIDTH, 0, bounds.size.w - ELBOW_WIDTH, TOP_BAR_HEIGHT));
  text_layer_set_background_color(s_date_layer, LCARS_COLOR_TOP_BAR);
  text_layer_set_text_color(s_date_layer, LCARS_COLOR_LABEL_TEXT);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  s_battery_layer = text_layer_create(GRect(ELBOW_WIDTH, TOP_BAR_HEIGHT + 60, (bounds.size.w - ELBOW_WIDTH) / 2, 40));
  text_layer_set_background_color(s_battery_layer, LCARS_COLOR_PANEL_BG_ALT);
  text_layer_set_text_color(s_battery_layer, LCARS_COLOR_PANEL_TEXT);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));

  s_bluetooth_layer = text_layer_create(GRect(ELBOW_WIDTH + (bounds.size.w - ELBOW_WIDTH) / 2, TOP_BAR_HEIGHT + 60, (bounds.size.w - ELBOW_WIDTH) / 2, 40));
  text_layer_set_background_color(s_bluetooth_layer, LCARS_COLOR_PANEL_BG_ALT);
  text_layer_set_text_color(s_bluetooth_layer, LCARS_COLOR_PANEL_TEXT);
  text_layer_set_font(s_bluetooth_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_bluetooth_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_bluetooth_layer));

  time_t now = time(NULL);
  update_time(localtime(&now));
  battery_handler(battery_state_service_peek());
  bluetooth_handler(connection_service_peek_pebble_app_connection());
}

static void window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_bluetooth_layer);
  layer_destroy(s_frame_layer);
}

static void init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_handler,
  });
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
