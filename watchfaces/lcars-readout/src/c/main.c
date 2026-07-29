#include <pebble.h>
#include "lcars_theme.h"
#include "lcars_draw.h"

static Window *s_window;
static Layer *s_canvas;

static GFont s_font_time;
static GFont s_font_date;
static GFont s_font_value;
static GFont s_font_batt;
static GFont s_font_label;

static GBitmap *s_icon_cond;   // points at whichever weather icon is current
static GBitmap *s_icon_clear;
static GBitmap *s_icon_cloud;
static GBitmap *s_icon_rain;
static GBitmap *s_icon_snow;
static GBitmap *s_icon_temp;
static GBitmap *s_icon_heart;
static GBitmap *s_icon_steps;

static char s_time_text[8];
static char s_date_text[12];
static char s_batt_text[8];
static char s_cond_text[10];
static char s_temp_text[8];
static char s_hr_text[8];
static char s_steps_text[8];

static bool s_connected;
static int s_cond_code = COND_UNKNOWN;

// Persisted so the last weather reading survives a watchface reload rather
// than blanking out until the phone answers again.
#define PKEY_COND 1
#define PKEY_TEMP 2

static void draw_value(GContext *ctx, int x, int y, int w, GBitmap *icon,
                       const char *text) {
  if (icon) {
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, icon, GRect(x, y + 3, ICON_SZ, ICON_SZ));
  }
  const int tx = x + ICON_SZ + 4;
  graphics_context_set_text_color(ctx, C_TEXT);
  graphics_draw_text(ctx, text, s_font_value, GRect(tx, y - 3, w - (tx - x), RO_ROW_H + 6),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, C_BG);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);

  // ---- left rail -----------------------------------------------------------
  lcars_block(ctx, GRect(RAIL_X, TOPBLOCK_Y, RAIL_W, TOPBLOCK_H), BLOCK_R, C_BAR);

  lcars_elbow_bottom(ctx, GRect(RAIL_X, ELBOW_A_Y, RAIL_W, ELBOW_A_H),
                     ELBOW_A_ARMY, ELBOW_A_ARMW, ELBOW_A_ARMH,
                     ELBOW_R, INNER_R, C_ELBOW_A, C_BG);

  lcars_elbow_top(ctx, GRect(RAIL_X, ELBOW_B_Y, RAIL_W, ELBOW_B_H),
                  ELBOW_B_ARMW, ELBOW_B_ARMH, ELBOW_R, INNER_R, C_ELBOW_B, C_BG);

  lcars_block(ctx, GRect(RAIL_X, LINK_Y, RAIL_W, LINK_H), BLOCK_R, C_ELBOW_B);
  graphics_context_set_text_color(ctx, C_TEXT);
  graphics_draw_text(ctx, s_connected ? "LINK\nACTIVE" : "LINK\nLOST", s_font_label,
                     GRect(RAIL_X + 2, LINK_Y + 1, RAIL_W - 4, LINK_H),
                     GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

  lcars_block(ctx, GRect(RAIL_X, RED_Y, RAIL_W, RED_H), BLOCK_R, C_ACCENT);

  lcars_block(ctx, GRect(RAIL_X, BATT_Y, RAIL_W, BATT_H), BLOCK_R, C_BATTERY);
  graphics_draw_text(ctx, s_batt_text, s_font_batt,
                     GRect(RAIL_X, BATT_Y + 1, RAIL_W, BATT_H),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  // ---- TIME ---------------------------------------------------------------
  lcars_header(ctx, GRect(CONT_X, TIME_HDR_Y, CONT_W, HDR_H), "TIME", s_font_label,
               true, C_CAP, C_BAR, C_CAP);

  graphics_context_set_text_color(ctx, C_TEXT);
  graphics_draw_text(ctx, s_time_text, s_font_time,
                     GRect(CONT_X, TIME_Y, CONT_W, TIME_H),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  // Rail of segments under the clock, mirroring the elbow arm's height.
  lcars_block(ctx, GRect(RAIL_BAR_X, RAIL_BAR_Y, 12, RAIL_BAR_H), 3, C_ACCENT);
  lcars_bar(ctx, GRect(RAIL_BAR_X + 15, RAIL_BAR_Y, 34, RAIL_BAR_H), 3, C_BAR);
  lcars_bar(ctx, GRect(RAIL_BAR_X + 52, RAIL_BAR_Y, 34, RAIL_BAR_H), 3, C_BAR);
  lcars_block(ctx, GRect(RAIL_BAR_X + 89, RAIL_BAR_Y, 9, RAIL_BAR_H), 3, C_ELBOW_B);

  // ---- STARDATE -----------------------------------------------------------
  lcars_header(ctx, GRect(DATE_HDR_X, DATE_HDR_Y, DATE_HDR_W, HDR_H), "STARDATE",
               s_font_label, false, C_ACCENT, C_BAR, C_BATTERY);

  graphics_context_set_text_color(ctx, C_TEXT);
  graphics_draw_text(ctx, s_date_text, s_font_date,
                     GRect(CONT_X, DATE_Y, CONT_W, DATE_H),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  // ---- readout grid -------------------------------------------------------
  lcars_header(ctx, GRect(RO_L_X, RO_HDR1_Y, RO_L_W, HDR_H), "SENSORS", s_font_label,
               true, C_CAP, C_BAR, C_CAP);
  lcars_header(ctx, GRect(RO_R_X, RO_HDR1_Y, RO_R_W, HDR_H), "VITALS", s_font_label,
               true, C_CAP, C_BAR, C_CAP);

  draw_value(ctx, RO_L_X, RO_ROW1_Y, RO_L_W, s_icon_cond, s_cond_text);
  draw_value(ctx, RO_R_X, RO_ROW1_Y, RO_R_W, s_icon_heart, s_hr_text);

  lcars_header(ctx, GRect(RO_R_X, RO_HDR2_Y, RO_R_W, HDR_H), "TRAVERSAL", s_font_label,
               true, C_CAP, C_BAR, C_CAP);

  draw_value(ctx, RO_L_X, RO_ROW2_Y, RO_L_W, s_icon_temp, s_temp_text);
  draw_value(ctx, RO_R_X, RO_ROW2_Y, RO_R_W, s_icon_steps, s_steps_text);
}

// ---------------------------------------------------------------------------
// Data
// ---------------------------------------------------------------------------

static void apply_condition(int code) {
  s_cond_code = code;
  switch (code) {
    case COND_CLEAR: s_icon_cond = s_icon_clear; strcpy(s_cond_text, "CLEAR"); break;
    case COND_CLOUD: s_icon_cond = s_icon_cloud; strcpy(s_cond_text, "CLOUD"); break;
    case COND_RAIN:  s_icon_cond = s_icon_rain;  strcpy(s_cond_text, "RAIN");  break;
    case COND_SNOW:  s_icon_cond = s_icon_snow;  strcpy(s_cond_text, "SNOW");  break;
    default:         s_icon_cond = NULL;         strcpy(s_cond_text, "--");    break;
  }
}

static void update_time(struct tm *t) {
  strftime(s_time_text, sizeof(s_time_text),
           clock_is_24h_style() ? "%H:%M" : "%I:%M", t);
  strftime(s_date_text, sizeof(s_date_text), "%d.%m.%Y", t);
}

static void update_health(void) {
#if defined(PBL_HEALTH)
  HealthServiceAccessibilityMask steps_ok =
      health_service_metric_accessible(HealthMetricStepCount,
                                       time_start_of_today(), time(NULL));
  if (steps_ok & HealthServiceAccessibilityMaskAvailable) {
    snprintf(s_steps_text, sizeof(s_steps_text), "%d",
             (int)health_service_sum_today(HealthMetricStepCount));
  } else {
    strcpy(s_steps_text, "--");
  }

  HealthValue hr = health_service_peek_current_value(HealthMetricHeartRateBPM);
  if (hr > 0 && hr < 300) {
    snprintf(s_hr_text, sizeof(s_hr_text), "%d", (int)hr);
  } else {
    strcpy(s_hr_text, "--");
  }
#else
  strcpy(s_steps_text, "--");
  strcpy(s_hr_text, "--");
#endif
}

static void update_battery(BatteryChargeState state) {
  snprintf(s_batt_text, sizeof(s_batt_text), "%d%%", state.charge_percent);
  layer_mark_dirty(s_canvas);
}

static void update_connection(bool connected) {
  s_connected = connected;
  layer_mark_dirty(s_canvas);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
  update_health();
  layer_mark_dirty(s_canvas);
}

static void health_handler(HealthEventType event, void *context) {
  update_health();
  layer_mark_dirty(s_canvas);
}

static void inbox_received(DictionaryIterator *iter, void *context) {
  Tuple *cond = dict_find(iter, MESSAGE_KEY_CONDITION);
  Tuple *temp = dict_find(iter, MESSAGE_KEY_TEMPERATURE);

  if (cond) {
    apply_condition((int)cond->value->int32);
    persist_write_int(PKEY_COND, (int)cond->value->int32);
  }
  if (temp) {
    snprintf(s_temp_text, sizeof(s_temp_text), "%d°C", (int)temp->value->int32);
    persist_write_int(PKEY_TEMP, (int)temp->value->int32);
  }
  layer_mark_dirty(s_canvas);
}

// ---------------------------------------------------------------------------

static void window_load(Window *window) {
  s_font_time  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_58));
  s_font_date  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_30));
  s_font_value = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_22));
  s_font_batt  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_16));
  s_font_label = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_14));

  s_icon_clear = gbitmap_create_with_resource(RESOURCE_ID_ICON_CLEAR);
  s_icon_cloud = gbitmap_create_with_resource(RESOURCE_ID_ICON_CLOUD);
  s_icon_rain  = gbitmap_create_with_resource(RESOURCE_ID_ICON_RAIN);
  s_icon_snow  = gbitmap_create_with_resource(RESOURCE_ID_ICON_SNOW);
  s_icon_temp  = gbitmap_create_with_resource(RESOURCE_ID_ICON_TEMP);
  s_icon_heart = gbitmap_create_with_resource(RESOURCE_ID_ICON_HEART);
  s_icon_steps = gbitmap_create_with_resource(RESOURCE_ID_ICON_STEPS);

  // init() may have restored a persisted condition before these existed, so
  // re-resolve the icon pointer now that the bitmaps are loaded.
  apply_condition(s_cond_code);

  s_canvas = layer_create(layer_get_bounds(window_get_root_layer(window)));
  layer_set_update_proc(s_canvas, canvas_update_proc);
  layer_add_child(window_get_root_layer(window), s_canvas);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas);

  gbitmap_destroy(s_icon_clear);
  gbitmap_destroy(s_icon_cloud);
  gbitmap_destroy(s_icon_rain);
  gbitmap_destroy(s_icon_snow);
  gbitmap_destroy(s_icon_temp);
  gbitmap_destroy(s_icon_heart);
  gbitmap_destroy(s_icon_steps);

  fonts_unload_custom_font(s_font_time);
  fonts_unload_custom_font(s_font_date);
  fonts_unload_custom_font(s_font_value);
  fonts_unload_custom_font(s_font_batt);
  fonts_unload_custom_font(s_font_label);
}

static void init(void) {
  strcpy(s_temp_text, "--");
  strcpy(s_hr_text, "--");
  strcpy(s_steps_text, "--");
  apply_condition(COND_UNKNOWN);

  if (persist_exists(PKEY_COND)) {
    apply_condition(persist_read_int(PKEY_COND));
  }
  if (persist_exists(PKEY_TEMP)) {
    snprintf(s_temp_text, sizeof(s_temp_text), "%d°C",
             (int)persist_read_int(PKEY_TEMP));
  }

  s_window = window_create();
  window_set_background_color(s_window, C_BG);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  time_t now = time(NULL);
  update_time(localtime(&now));
  update_health();
  s_connected = connection_service_peek_pebble_app_connection();
  update_battery(battery_state_service_peek());

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(update_battery);
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = update_connection,
  });
#if defined(PBL_HEALTH)
  health_service_events_subscribe(health_handler, NULL);
#endif

  app_message_register_inbox_received(inbox_received);
  app_message_open(128, 32);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
#if defined(PBL_HEALTH)
  health_service_events_unsubscribe();
#endif
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
