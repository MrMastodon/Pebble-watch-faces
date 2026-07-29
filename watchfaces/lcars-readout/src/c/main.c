#include <pebble.h>
#include "lcars_theme.h"

static Window *s_window;

// Everything static — the elbow, the coloured bars, the label captions and the
// light panel backgrounds — is painted here. Dynamic values sit on top in
// transparent TextLayers.
static Layer *s_frame_layer;

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_readout_layer[READOUT_COUNT];

static char s_time_buffer[8];
static char s_date_buffer[16];
static char s_readout_buffer[READOUT_COUNT][24];

// Slot 0 and 1 are placeholders until weather (pkjs) and health land; slot 2 is
// live today. Keeping all three wired up means adding data later is just a
// matter of filling the buffer.
static const char *const s_readout_label[READOUT_COUNT] = {
  "SENSORS",
  "VITALS",
  "SYSTEMS",
};

static const GColor8 *readout_label_color(int i) {
  static GColor8 colors[READOUT_COUNT];
  colors[0] = LCARS_COLOR_LABEL_BAR;
  colors[1] = LCARS_COLOR_LABEL_BAR_2;
  colors[2] = LCARS_COLOR_LABEL_BAR_3;
  return &colors[i];
}

static void draw_elbow(GContext *ctx) {
  graphics_context_set_fill_color(ctx, LCARS_COLOR_ELBOW);

  // Vertical arm and horizontal arm of the top-left elbow.
  graphics_fill_rect(ctx, GRect(0, 0, ELBOW_W, ELBOW_UPPER_H),
                     ELBOW_RADIUS, GCornerTopLeft);
  graphics_fill_rect(ctx, GRect(0, 0, ELBOW_STUB_W, TOP_BAR_H),
                     ELBOW_RADIUS, GCornerTopLeft);

  // Carve the concave inner corner by painting the background back over it.
  graphics_context_set_fill_color(ctx, LCARS_COLOR_BACKGROUND);
  graphics_fill_rect(ctx, GRect(ELBOW_W, TOP_BAR_H,
                                ELBOW_STUB_W - ELBOW_W, TOP_BAR_H),
                     8, GCornerTopLeft);

  // Lower segments of the column.
  graphics_context_set_fill_color(ctx, LCARS_COLOR_ELBOW_LOWER);
  graphics_fill_rect(ctx, GRect(0, ELBOW_LOWER_Y, ELBOW_W, ELBOW_LOWER_H),
                     0, GCornerNone);

  graphics_context_set_fill_color(ctx, LCARS_COLOR_ELBOW_FOOT);
  graphics_fill_rect(ctx, GRect(0, ELBOW_FOOT_Y, ELBOW_W, ELBOW_FOOT_H),
                     ELBOW_RADIUS, GCornerBottomLeft);
}

static void draw_readout_slot(GContext *ctx, int i) {
  const int y = READOUT_Y(i);

  // Coloured caption bar.
  graphics_context_set_fill_color(ctx, *readout_label_color(i));
  graphics_fill_rect(ctx, GRect(CONTENT_X, y, CONTENT_W, READOUT_LABEL_H),
                     PANEL_RADIUS, GCornerTopLeft | GCornerTopRight);

  graphics_context_set_text_color(ctx, LCARS_COLOR_LABEL_TEXT);
  graphics_draw_text(ctx, s_readout_label[i],
                     fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     GRect(CONTENT_X + 6, y - 3, CONTENT_W - 12, READOUT_LABEL_H + 4),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

  // Light data panel underneath.
  graphics_context_set_fill_color(ctx, LCARS_COLOR_PANEL_BG);
  graphics_fill_rect(ctx, GRect(CONTENT_X, y + READOUT_LABEL_H,
                                CONTENT_W, READOUT_H - READOUT_LABEL_H),
                     PANEL_RADIUS, GCornerBottomLeft | GCornerBottomRight);
}

static void frame_layer_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, LCARS_COLOR_BACKGROUND);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);

  draw_elbow(ctx);

  // Date bar, to the right of the elbow's horizontal arm.
  graphics_context_set_fill_color(ctx, LCARS_COLOR_DATE_BAR);
  graphics_fill_rect(ctx, GRect(DATE_BAR_X, 0, DATE_BAR_W, TOP_BAR_H),
                     PANEL_RADIUS, GCornerBottomLeft);

  // Light panel behind the clock.
  graphics_context_set_fill_color(ctx, LCARS_COLOR_PANEL_BG);
  graphics_fill_rect(ctx, GRect(CONTENT_X, TIME_PANEL_Y, CONTENT_W, TIME_PANEL_H),
                     PANEL_RADIUS, GCornersAll);

  for (int i = 0; i < READOUT_COUNT; i++) {
    draw_readout_slot(ctx, i);
  }
}

static void update_time(struct tm *tick_time) {
  strftime(s_time_buffer, sizeof(s_time_buffer),
           clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_time_buffer);

  strftime(s_date_buffer, sizeof(s_date_buffer), "%a %d %b", tick_time);
  text_layer_set_text(s_date_layer, s_date_buffer);
}

// Slot 2 shows battery and Bluetooth together, so refresh it from one place.
static void update_systems_readout(void) {
  BatteryChargeState battery = battery_state_service_peek();
  bool connected = connection_service_peek_pebble_app_connection();

  snprintf(s_readout_buffer[2], sizeof(s_readout_buffer[2]), "%d%%   %s",
           battery.charge_percent, connected ? "LINK" : "NO LINK");
  text_layer_set_text(s_readout_layer[2], s_readout_buffer[2]);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

static void battery_handler(BatteryChargeState charge_state) {
  update_systems_readout();
}

static void bluetooth_handler(bool connected) {
  update_systems_readout();
}

static TextLayer *make_panel_text(Layer *parent, GRect frame, const char *font_key,
                                  GTextAlignment align) {
  TextLayer *layer = text_layer_create(frame);
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_text_color(layer, LCARS_COLOR_PANEL_TEXT);
  text_layer_set_font(layer, fonts_get_system_font(font_key));
  text_layer_set_text_alignment(layer, align);
  layer_add_child(parent, text_layer_get_layer(layer));
  return layer;
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  s_frame_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_frame_layer, frame_layer_update_proc);
  layer_add_child(window_layer, s_frame_layer);

  // Bitham 42 sits low in its box, so nudge the frame up to centre it.
  s_time_layer = make_panel_text(window_layer,
                                 GRect(CONTENT_X, TIME_PANEL_Y + 2, CONTENT_W, TIME_PANEL_H),
                                 FONT_KEY_BITHAM_42_BOLD, GTextAlignmentCenter);

  s_date_layer = make_panel_text(window_layer,
                                 GRect(DATE_BAR_X + 4, -1, DATE_BAR_W - 8, TOP_BAR_H + 4),
                                 FONT_KEY_GOTHIC_18_BOLD, GTextAlignmentCenter);

  for (int i = 0; i < READOUT_COUNT; i++) {
    const int data_y = READOUT_Y(i) + READOUT_LABEL_H;
    s_readout_layer[i] = make_panel_text(
        window_layer,
        GRect(CONTENT_X + 6, data_y - 1, CONTENT_W - 12, READOUT_H - READOUT_LABEL_H + 4),
        FONT_KEY_GOTHIC_24_BOLD, GTextAlignmentCenter);
  }

  // Reserved slots read as intentionally empty rather than broken.
  snprintf(s_readout_buffer[0], sizeof(s_readout_buffer[0]), "--");
  text_layer_set_text(s_readout_layer[0], s_readout_buffer[0]);
  snprintf(s_readout_buffer[1], sizeof(s_readout_buffer[1]), "--");
  text_layer_set_text(s_readout_layer[1], s_readout_buffer[1]);

  time_t now = time(NULL);
  update_time(localtime(&now));
  update_systems_readout();
}

static void window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  for (int i = 0; i < READOUT_COUNT; i++) {
    text_layer_destroy(s_readout_layer[i]);
  }
  layer_destroy(s_frame_layer);
}

static void init(void) {
  s_window = window_create();
  window_set_background_color(s_window, LCARS_COLOR_BACKGROUND);
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
