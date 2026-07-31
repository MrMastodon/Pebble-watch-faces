#include <pebble.h>
#include "lcars_theme.h"

static Window *s_window;
static Layer *s_canvas;

static GBitmap *s_background;

static GFont s_font_time;
static GFont s_font_date;
static GFont s_font_value;
static GFont s_font_batt;

static GBitmap *s_icon_cond;   // points at whichever weather icon is current
static GBitmap *s_icon_clear;
static GBitmap *s_icon_clear_n;
static GBitmap *s_icon_partly;
static GBitmap *s_icon_partly_n;
static GBitmap *s_icon_cloud;
static GBitmap *s_icon_fog;
static GBitmap *s_icon_rain;
static GBitmap *s_icon_snow;
static GBitmap *s_icon_storm;

static char s_time_text[8];
static char s_date_text[12];
static char s_batt_text[8];
static char s_cond_text[10];
static char s_temp_text[8];
static char s_hr_text[8];
static char s_steps_text[8];

static int s_cond_code = COND_UNKNOWN;
static bool s_is_day = true;
static bool s_connected;

// Settings, mirrored from the Clay panel and persisted so they survive a
// reload without waiting on the phone.
static int  s_date_format = DATE_DMY;
static bool s_temp_fahrenheit = false;
static int  s_vibe_bt = VIBE_SHORT;
static bool s_vibe_hourly = false;

// Persisted so the last weather reading survives a watchface reload rather
// than blanking out until the phone answers again.
#define PKEY_COND        1
#define PKEY_TEMP        2
#define PKEY_DAY         3
#define PKEY_DATE_FORMAT 4
#define PKEY_TEMP_UNIT   5
#define PKEY_VIBE_BT     6
#define PKEY_VIBE_HOURLY 7

// Antonio sits low in its line box, so every field is nudged up a little.
static void draw_text(GContext *ctx, const char *text, GFont font,
                      int x, int y, int w, int h, GTextAlignment align) {
  graphics_draw_text(ctx, text, font, GRect(x, y - 3, w, h + 6),
                     GTextOverflowModeTrailingEllipsis, align, NULL);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  graphics_draw_bitmap_in_rect(ctx, s_background, GRect(0, 0, SCREEN_W, SCREEN_H));

  graphics_context_set_text_color(ctx, C_TEXT);

  draw_text(ctx, s_time_text, s_font_time, CONT_X, TIME_Y, CONT_W, TIME_H,
            GTextAlignmentCenter);
  draw_text(ctx, s_date_text, s_font_date, CONT_X, DATE_Y, CONT_W, DATE_H,
            GTextAlignmentCenter);
  draw_text(ctx, s_batt_text, s_font_batt, BATT_X, BATT_Y, BATT_W, BATT_H,
            GTextAlignmentCenter);

  if (s_connected) {
    draw_text(ctx, "LINK", s_font_batt, LINK_X, LINK_Y, LINK_W, LINK_H,
              GTextAlignmentCenter);
  }

  if (s_icon_cond) {
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(
        ctx, s_icon_cond, GRect(COND_ICON_X, COND_ICON_Y, ICON_SZ, ICON_SZ));
  }

  draw_text(ctx, s_cond_text, s_font_value, COND_TEXT_X, COND_TEXT_Y,
            COND_TEXT_W, COND_TEXT_H, GTextAlignmentLeft);
  draw_text(ctx, s_hr_text, s_font_value, HR_TEXT_X, HR_TEXT_Y,
            HR_TEXT_W, HR_TEXT_H, GTextAlignmentLeft);
  draw_text(ctx, s_temp_text, s_font_value, TEMP_TEXT_X, TEMP_TEXT_Y,
            TEMP_TEXT_W, TEMP_TEXT_H, GTextAlignmentLeft);
  draw_text(ctx, s_steps_text, s_font_value, STEPS_TEXT_X, STEPS_TEXT_Y,
            STEPS_TEXT_W, STEPS_TEXT_H, GTextAlignmentLeft);
}

// ---------------------------------------------------------------------------
// Data
// ---------------------------------------------------------------------------

// Abbreviations are chosen by measured width, not letter count: the readout
// column is 49px at Antonio 22, and W is far wider than the average cap, so
// SHWRS (53px) and SNSHW (54px) overflowed while five-letter CLEAR (46px) is
// fine. tools/measure_text.py checks a candidate before it ships.
//
// The icon is the at-a-glance cue; the text is what actually distinguishes
// drizzle from freezing rain.
static void apply_condition(int code) {
  s_cond_code = code;
  switch (code) {
    case COND_CLEAR:
      s_icon_cond = s_is_day ? s_icon_clear : s_icon_clear_n;
      strcpy(s_cond_text, "CLEAR"); break;
    case COND_PARTLY:
      s_icon_cond = s_is_day ? s_icon_partly : s_icon_partly_n;
      strcpy(s_cond_text, "PTCLD"); break;
    case COND_CLOUD:
      s_icon_cond = s_icon_cloud; strcpy(s_cond_text, "CLDY"); break;
    case COND_FOG:
      s_icon_cond = s_icon_fog;   strcpy(s_cond_text, "FOG");   break;
    case COND_DRIZZLE:
      s_icon_cond = s_icon_rain;  strcpy(s_cond_text, "DRIZL"); break;
    case COND_RAIN:
      s_icon_cond = s_icon_rain;  strcpy(s_cond_text, "RAIN");  break;
    case COND_FZRAIN:
      s_icon_cond = s_icon_rain;  strcpy(s_cond_text, "FZRN");  break;
    case COND_SNOW:
      s_icon_cond = s_icon_snow;  strcpy(s_cond_text, "SNOW");  break;
    case COND_SHOWERS:
      s_icon_cond = s_icon_rain;  strcpy(s_cond_text, "SHWR"); break;
    case COND_SNOWSH:
      s_icon_cond = s_icon_snow;  strcpy(s_cond_text, "SNSH"); break;
    case COND_STORM:
      s_icon_cond = s_icon_storm; strcpy(s_cond_text, "STRM"); break;
    case COND_NO_LOCATION:
      s_icon_cond = NULL;         strcpy(s_cond_text, "GPS?");  break;
    case COND_NO_NET:
      s_icon_cond = NULL;         strcpy(s_cond_text, "NET?");  break;
    default:
      s_icon_cond = NULL;         strcpy(s_cond_text, "--");    break;
  }
}

static void update_time(struct tm *t) {
  strftime(s_time_text, sizeof(s_time_text),
           clock_is_24h_style() ? "%H:%M" : "%I:%M", t);
  const char *fmt = "%d.%m.%Y";
  if (s_date_format == DATE_MDY) fmt = "%m.%d.%Y";
  else if (s_date_format == DATE_ISO) fmt = "%Y-%m-%d";
  strftime(s_date_text, sizeof(s_date_text), fmt, t);
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

// One place that decides whether the watch is allowed to buzz, so both the
// connection alert and the hourly chime inherit the Quiet Time rule the SDK
// asks watchfaces to honour.
//
// 150ms rather than the 60ms first shipped. The Time 2 uses a linear resonant
// actuator, which reaches amplitude far faster than a rotating-mass motor, so
// a pulse that short is not impossible in principle — but it leaves only a few
// tens of ms at full amplitude, which is easy to miss on a loose strap. The
// panel exposes a longer option rather than guessing a single right answer.
static void buzz(int strength) {
  if (strength == VIBE_OFF || quiet_time_is_active()) return;

  if (strength == VIBE_LONG) {
    vibes_short_pulse();     // the OS's own calibrated short buzz
    return;
  }
  static const uint32_t pulse[] = { 150 };
  VibePattern pat = { .durations = pulse, .num_segments = ARRAY_LENGTH(pulse) };
  vibes_enqueue_custom_pattern(pat);
}

static void update_connection(bool connected) {
  bool was_connected = s_connected;
  s_connected = connected;

  // Only on the way down, and never for the initial state — s_connected is
  // seeded in init() before subscribing, so loading the watchface while the
  // phone is already out of range stays silent.
  if (was_connected && !connected) {
    buzz(s_vibe_bt);
  }

  if (s_canvas) layer_mark_dirty(s_canvas);
}

static void update_battery(BatteryChargeState state) {
  snprintf(s_batt_text, sizeof(s_batt_text), "%d%%", state.charge_percent);
  if (s_canvas) layer_mark_dirty(s_canvas);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
  update_health();

  // MINUTE_UNIT is already subscribed for the clock, so the hourly chime is
  // just a check rather than a second subscription.
  if (s_vibe_hourly && tick_time->tm_min == 0) {
    buzz(VIBE_SHORT);
  }

  layer_mark_dirty(s_canvas);
}

static void health_handler(HealthEventType event, void *context) {
  update_health();
  layer_mark_dirty(s_canvas);
}

static void inbox_received(DictionaryIterator *iter, void *context) {
  Tuple *cond = dict_find(iter, MESSAGE_KEY_CONDITION);
  Tuple *temp = dict_find(iter, MESSAGE_KEY_TEMPERATURE);
  Tuple *day  = dict_find(iter, MESSAGE_KEY_IS_DAY);

  // Settings. Clay sends these as strings for selects and a byte for toggles,
  // so accept whichever arrives rather than assuming one shape.
  Tuple *t;
  if ((t = dict_find(iter, MESSAGE_KEY_DATE_FORMAT))) {
    s_date_format = (t->type == TUPLE_CSTRING) ? atoi(t->value->cstring)
                                               : (int)t->value->int32;
    persist_write_int(PKEY_DATE_FORMAT, s_date_format);
  }
  if ((t = dict_find(iter, MESSAGE_KEY_TEMP_UNIT))) {
    int unit = (t->type == TUPLE_CSTRING) ? atoi(t->value->cstring)
                                          : (int)t->value->int32;
    s_temp_fahrenheit = (unit != 0);
    persist_write_bool(PKEY_TEMP_UNIT, s_temp_fahrenheit);
  }
  if ((t = dict_find(iter, MESSAGE_KEY_VIBE_BT))) {
    s_vibe_bt = (t->type == TUPLE_CSTRING) ? atoi(t->value->cstring)
                                           : (int)t->value->int32;
    persist_write_int(PKEY_VIBE_BT, s_vibe_bt);
  }
  if ((t = dict_find(iter, MESSAGE_KEY_VIBE_HOURLY))) {
    s_vibe_hourly = t->value->int32 != 0;
    persist_write_bool(PKEY_VIBE_HOURLY, s_vibe_hourly);
  }

  // Read the day flag first so apply_condition() picks the right variant.
  if (day) {
    s_is_day = day->value->int32 != 0;
    persist_write_bool(PKEY_DAY, s_is_day);
  }
  if (cond) {
    int code = (int)cond->value->int32;
    apply_condition(code);
    // Only remember real readings — a failure state should not come back after
    // a reload and masquerade as the current weather.
    if (code >= COND_CLEAR && code <= COND_LAST_REAL) {
      persist_write_int(PKEY_COND, code);
    }
  }
  if (temp) {
    snprintf(s_temp_text, sizeof(s_temp_text), "%d°%c", (int)temp->value->int32,
             s_temp_fahrenheit ? 'F' : 'C');
    persist_write_int(PKEY_TEMP, (int)temp->value->int32);
  }

  // The date only redraws on a tick otherwise, so a format change would sit
  // unseen for up to a minute.
  time_t now = time(NULL);
  update_time(localtime(&now));

  layer_mark_dirty(s_canvas);
}

// ---------------------------------------------------------------------------

static void window_load(Window *window) {
  s_background = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);

  s_font_time  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_58));
  s_font_date  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_30));
  s_font_value = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_22));
  s_font_batt  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_16));

  s_icon_clear    = gbitmap_create_with_resource(RESOURCE_ID_ICON_CLEAR);
  s_icon_clear_n  = gbitmap_create_with_resource(RESOURCE_ID_ICON_CLEAR_N);
  s_icon_partly   = gbitmap_create_with_resource(RESOURCE_ID_ICON_PARTLY);
  s_icon_partly_n = gbitmap_create_with_resource(RESOURCE_ID_ICON_PARTLY_N);
  s_icon_cloud    = gbitmap_create_with_resource(RESOURCE_ID_ICON_CLOUD);
  s_icon_fog      = gbitmap_create_with_resource(RESOURCE_ID_ICON_FOG);
  s_icon_rain     = gbitmap_create_with_resource(RESOURCE_ID_ICON_RAIN);
  s_icon_snow     = gbitmap_create_with_resource(RESOURCE_ID_ICON_SNOW);
  s_icon_storm    = gbitmap_create_with_resource(RESOURCE_ID_ICON_STORM);

  // init() may have restored a persisted condition before these existed, so
  // re-resolve the icon pointer now that the bitmaps are loaded.
  apply_condition(s_cond_code);

  s_canvas = layer_create(layer_get_bounds(window_get_root_layer(window)));
  layer_set_update_proc(s_canvas, canvas_update_proc);
  layer_add_child(window_get_root_layer(window), s_canvas);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas);
  s_canvas = NULL;

  gbitmap_destroy(s_background);
  gbitmap_destroy(s_icon_clear);
  gbitmap_destroy(s_icon_clear_n);
  gbitmap_destroy(s_icon_partly);
  gbitmap_destroy(s_icon_partly_n);
  gbitmap_destroy(s_icon_cloud);
  gbitmap_destroy(s_icon_fog);
  gbitmap_destroy(s_icon_rain);
  gbitmap_destroy(s_icon_snow);
  gbitmap_destroy(s_icon_storm);

  fonts_unload_custom_font(s_font_time);
  fonts_unload_custom_font(s_font_date);
  fonts_unload_custom_font(s_font_value);
  fonts_unload_custom_font(s_font_batt);
}

static void init(void) {
  strcpy(s_temp_text, "--");
  strcpy(s_hr_text, "--");
  strcpy(s_steps_text, "--");
  apply_condition(COND_UNKNOWN);

  if (persist_exists(PKEY_DATE_FORMAT)) s_date_format = persist_read_int(PKEY_DATE_FORMAT);
  if (persist_exists(PKEY_TEMP_UNIT))   s_temp_fahrenheit = persist_read_bool(PKEY_TEMP_UNIT);
  if (persist_exists(PKEY_VIBE_BT))     s_vibe_bt = persist_read_int(PKEY_VIBE_BT);
  if (persist_exists(PKEY_VIBE_HOURLY)) s_vibe_hourly = persist_read_bool(PKEY_VIBE_HOURLY);

  if (persist_exists(PKEY_DAY)) {
    s_is_day = persist_read_bool(PKEY_DAY);
  }
  if (persist_exists(PKEY_COND)) {
    apply_condition(persist_read_int(PKEY_COND));
  }
  if (persist_exists(PKEY_TEMP)) {
    snprintf(s_temp_text, sizeof(s_temp_text), "%d°%c",
             (int)persist_read_int(PKEY_TEMP), s_temp_fahrenheit ? 'F' : 'C');
  }

  s_window = window_create();
  window_set_background_color(s_window, GColorWhite);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  time_t now = time(NULL);
  update_time(localtime(&now));
  update_health();
  update_battery(battery_state_service_peek());

  // Seed before subscribing: update_connection() vibrates on a true->false
  // edge, and without this the first callback would look like a fresh drop.
  s_connected = connection_service_peek_pebble_app_connection();

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
