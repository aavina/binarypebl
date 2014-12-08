#include "pebble.h"

static Window *window;

static Layer *layer;

static GBitmap *image;

static int PEB_RES_X = 144;
static int PEB_RES_Y = 168;

static int OUTER_BOX_SIZE = 10;
static int INNER_BOX_SIZE = 6;
static int BOX_OFFSET = 2;

typedef struct {
  int x;
  int y;
  int on; /* 0 = off, anything else = on */
} Bit;

Bit g_h1[2];
Bit g_h0[4];
Bit g_m1[3];
Bit g_m0[4];

static void draw_bit_on(int x, int y, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(x, y, OUTER_BOX_SIZE, OUTER_BOX_SIZE), 0, GCornerNone);
}

static void draw_bit_off(int x, int y, GContext *ctx) {
  // Draw the entire box
  draw_bit_on(x, y, ctx);
  // Draw the hole in the box
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(x+BOX_OFFSET, y+BOX_OFFSET, INNER_BOX_SIZE, INNER_BOX_SIZE), 0, GCornerNone);
}

static void draw_bit(Bit b, GContext *ctx) {
  if(b.on == 0)
    draw_bit_off(b.x, b.y, ctx);
  else
    draw_bit_on(b.x, b.y, ctx);
}

static void layer_update_callback(Layer *me, GContext *ctx) {
  int i;
  
  // Hour_1 bits
  for(i=0; i < 2; ++i)
    draw_bit(g_h1[i], ctx);
  // Hour_0 bits
  for(i=0; i < 4; ++i)
    draw_bit(g_h0[i], ctx);
  // Minute_1 bits
  for(i=0; i < 3; ++i)
    draw_bit(g_m1[i], ctx);
  // Minute_0 bits
  for(i=0; i < 4; ++i)
    draw_bit(g_m0[i], ctx);
}

static void setup_bin_arr(Bit* arr, int size, int x, int y_offset) {
  int i;
  
  for(i=0; i < size; ++i) {
    arr[i].x = x;
    arr[i].y = PEB_RES_Y-y_offset;
    arr[i].on = 0;
    y_offset += 20;
  }
}

static void setup_binary_arrays() {
  setup_bin_arr(g_h1, 2, 30, 40);
  setup_bin_arr(g_h0, 4, 50, 40);
  setup_bin_arr(g_m1, 3, 70, 40);
  setup_bin_arr(g_m0, 4, 90, 40);
}

int main(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);

  // Init the layer for display the image
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  layer = layer_create(bounds);
  layer_set_update_proc(layer, layer_update_callback);
  layer_add_child(window_layer, layer);

  // Setup Binary arrays
  setup_binary_arrays();
  
  app_event_loop();

  gbitmap_destroy(image);

  window_destroy(window);
  layer_destroy(layer);
}
