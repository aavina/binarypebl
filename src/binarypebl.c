#include "pebble.h"

/*
  Watchface that tells the time in a Binary format
  by: Anthony Avina, 2014-2015
*/
  
static Window *window;

static Layer *layer;

//static int PEB_RES_X = 144;
static int PEB_RES_Y = 168;

// Light/Dark theme choices for watchface
enum Theme {
  light,
  dark
};

// Configurable settings
static enum Theme THEME = dark;  // Theme of watchface

// Dependent on configured settings
static int ON_COLOR;  // Color of Bits when on
static int OFF_COLOR; // Color of bits when off
static int OUTER_BOX_SIZE = 14; // Size of Bit box (outside)
static int INNER_BOX_SIZE; // Size of Bit box (inside)
static int BOX_OFFSET;
static int X_OFFSET = 21; // Where to start drawing the Bits - x
static int Y_OFFSET = 40; // Where to start drawing the Bits - y

typedef struct {
  int x; /* x-coord of origin */
  int y; /* y-coord of origin */
  int on; /* 0 = off, anything else = on */
} Bit;

static Bit g_h1[2]; // Bits for hour1 digit
static Bit g_h0[4]; // Bits for hour0 digit
static Bit g_m1[3]; // Bits for minute1 digit
static Bit g_m0[4]; // Bits for minute0 digit

// Bit-masks to find out which bits are set
static const int g_bitmasks[] = {0x01, 0x02, 0x04, 0x08};

// Will draw an 'on' bit, or a square that's filled in
static void draw_bit_on(int x, int y, GContext *ctx) {
  graphics_context_set_fill_color(ctx, ON_COLOR);
  graphics_fill_rect(ctx, GRect(x, y, OUTER_BOX_SIZE, OUTER_BOX_SIZE), 0, GCornerNone);
}

// Will draw an 'off' bit, or a square with a hole
static void draw_bit_off(int x, int y, GContext *ctx) {
  // Draw the entire box
  draw_bit_on(x, y, ctx);
  // Draw the hole in the box
  graphics_context_set_fill_color(ctx, OFF_COLOR);
  graphics_fill_rect(ctx, GRect(x+BOX_OFFSET, y+BOX_OFFSET, INNER_BOX_SIZE, INNER_BOX_SIZE), 0, GCornerNone);
}

// Draws a bit
static void draw_bit(Bit b, GContext *ctx) {
  if(b.on == 0)
    draw_bit_off(b.x, b.y, ctx);
  else
    draw_bit_on(b.x, b.y, ctx);
}

// Finds out which bits are on/off and sets them to respective value
static void flip_bits_array(Bit *arr, int size, char digit) {
  int i;
  
  for(i=0; i < size; ++i) {
    if(digit & g_bitmasks[i])
      arr[i].on = 1;
    else
      arr[i].on = 0;
  }
}

// Updates the on/off status of bits
static void update_bits() {
  // Get a tm structure and time digits
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  int m0_digit = tick_time->tm_min % 10;
  int m1_digit = (tick_time->tm_min - m0_digit) / 10;
  int h0_digit = tick_time->tm_hour % 10;
  int h1_digit = (tick_time->tm_hour - h0_digit) / 10;

  flip_bits_array(g_h1, 2, h1_digit);
  flip_bits_array(g_h0, 4, h0_digit);
  flip_bits_array(g_m1, 3, m1_digit);
  flip_bits_array(g_m0, 4, m0_digit);
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

// Sets up positioning and states of all bits of given array
static void setup_bin_arr(Bit* arr, int size, int x, int y_offset) {
  int i;
  int y_add = OUTER_BOX_SIZE*2;
  
  for(i=0; i < size; ++i) {
    arr[i].x = x;
    arr[i].y = PEB_RES_Y-y_offset;
    arr[i].on = 0;
    y_offset += y_add;
  }
}

// Calculates positioning for bits and calls helper on each set
static void setup_binary_arrays() {
  int x_add = OUTER_BOX_SIZE*2;
  setup_bin_arr(g_h1, 2, X_OFFSET, Y_OFFSET);
  setup_bin_arr(g_h0, 4, X_OFFSET+x_add, Y_OFFSET);
  setup_bin_arr(g_m1, 3, X_OFFSET+(x_add*2), Y_OFFSET);
  setup_bin_arr(g_m0, 4, X_OFFSET+(x_add*3), Y_OFFSET);
}

// Handles the minute ticks
static void tick_handler(struct tm *tick_time, TimeUnits units_changes) {
  update_bits();
  layer_mark_dirty(layer);
}

void init() {
  // Initialize some constants
  INNER_BOX_SIZE = OUTER_BOX_SIZE - 4;
  BOX_OFFSET = (OUTER_BOX_SIZE - INNER_BOX_SIZE) / 2;
  
  if(THEME == dark) {
    ON_COLOR = GColorWhite;
    OFF_COLOR = GColorBlack;
  }else {
    ON_COLOR = GColorBlack;
    OFF_COLOR = GColorWhite;
  }
  
  // Setup Binary arrays
  setup_binary_arrays();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

int main(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  if(THEME == light)
    window_set_background_color(window, GColorWhite);
  else
    window_set_background_color(window, GColorBlack);

  // Init the layer for display the image
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  layer = layer_create(bounds);
  layer_set_update_proc(layer, layer_update_callback);
  layer_add_child(window_layer, layer);

  init();
  
  update_bits();
  
  app_event_loop();

  // Unsubscribe from tick-timer
  tick_timer_service_unsubscribe();

  window_destroy(window);
  layer_destroy(layer);
}
