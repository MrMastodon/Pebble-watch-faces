#include "lcars_draw.h"
#include "lcars_theme.h"

void lcars_block(GContext *ctx, GRect rect, int radius, GColor color) {
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_rect(ctx, rect, radius, GCornersAll);
}

void lcars_bar(GContext *ctx, GRect rect, int radius, GColor color) {
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_rect(ctx, rect, radius, GCornersAll);
  graphics_context_set_stroke_color(ctx, C_TEXT);
  graphics_draw_round_rect(ctx, rect, radius);
}

void lcars_elbow_bottom(GContext *ctx, GRect vert, int arm_y, int arm_w, int arm_h,
                        int radius, int inner_radius, GColor color, GColor bg) {
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_rect(ctx, vert, radius, GCornerBottomLeft);
  graphics_fill_rect(ctx, GRect(vert.origin.x, arm_y, arm_w, arm_h),
                     radius, GCornerBottomLeft);

  // Rounding the bottom-left of a background patch pinned to the inner corner
  // is what produces the concave sweep between the two arms.
  graphics_context_set_fill_color(ctx, bg);
  graphics_fill_rect(ctx,
                     GRect(vert.origin.x + vert.size.w, vert.origin.y,
                           arm_w - vert.size.w, arm_y - vert.origin.y),
                     inner_radius, GCornerBottomLeft);
}

void lcars_elbow_top(GContext *ctx, GRect vert, int arm_w, int arm_h,
                     int radius, int inner_radius, GColor color, GColor bg) {
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_rect(ctx, vert, radius, GCornerTopLeft);
  graphics_fill_rect(ctx, GRect(vert.origin.x, vert.origin.y, arm_w, arm_h),
                     radius, GCornerTopLeft);

  graphics_context_set_fill_color(ctx, bg);
  graphics_fill_rect(ctx,
                     GRect(vert.origin.x + vert.size.w, vert.origin.y + arm_h,
                           arm_w - vert.size.w, vert.size.h - arm_h),
                     inner_radius, GCornerTopLeft);
}

void lcars_header(GContext *ctx, GRect box, const char *label, GFont font,
                  bool leading_pill, GColor pill, GColor bar, GColor cap) {
  const int right = box.origin.x + box.size.w;
  const int label_x = box.origin.x + (leading_pill ? HDR_PILL_W + HDR_GAP : 0);

  if (leading_pill) {
    graphics_context_set_fill_color(ctx, pill);
    graphics_fill_rect(ctx, GRect(box.origin.x, box.origin.y, HDR_PILL_W, box.size.h),
                       box.size.h / 2, GCornerTopLeft | GCornerBottomLeft);
  }

  const GSize label_size = graphics_text_layout_get_content_size(
      label, font, GRect(label_x, box.origin.y, right - label_x, box.size.h),
      GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);

  graphics_context_set_text_color(ctx, C_TEXT);
  // Antonio sits low in its line box; lift it so it centres on the bar.
  graphics_draw_text(ctx, label, font,
                     GRect(label_x, box.origin.y - 3, right - label_x, box.size.h + 6),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

  const int cap_x = right - HDR_CAP_W;
  graphics_context_set_fill_color(ctx, cap);
  graphics_fill_rect(ctx, GRect(cap_x, box.origin.y, HDR_CAP_W, box.size.h),
                     box.size.h / 2, GCornerTopRight | GCornerBottomRight);

  int bar_x = label_x + label_size.w + HDR_GAP;
  if (!leading_pill) {
    graphics_context_set_fill_color(ctx, pill);
    graphics_fill_rect(ctx, GRect(bar_x, box.origin.y, HDR_PILL_W, box.size.h),
                       2, GCornersAll);
    bar_x += HDR_PILL_W + HDR_GAP;
  }

  const int bar_w = cap_x - HDR_GAP - bar_x;
  if (bar_w >= 6) {
    lcars_bar(ctx, GRect(bar_x, box.origin.y + 1, bar_w, box.size.h - 2), 3, bar);
  }
}
