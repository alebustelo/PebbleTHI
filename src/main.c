#include <pebble.h>
  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_layer;

static GFont s_time_font;
static GFont s_date_font;
static GFont s_battery_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00 xx";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) { //true
    //Use 24h hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof("00:00 xx"), "%I:%M %p", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void update_date(){
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_date = localtime(&temp);
  
  // Create a long-lived buffer
  static char buffer[] = "00/00/0000";
  
  // Write the current date to the buffer
  strftime(buffer, sizeof("00/00/0000"), "%m/%d/%Y", tick_date);
  
  // Display this date on the TextLayer
  text_layer_set_text(s_date_layer, buffer);
}

static void battery_handler(BatteryChargeState new_state) {
  // Write to buffer and display
  static char s_battery_buffer[5];
  if (new_state.is_charging) {
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "CHRG");
  } else {
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", new_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, s_battery_buffer);
}

static void main_window_load(Window *window) {
  //Create GBitmap, then set to created BitmapLayer
  //s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_THI_IMAGE_BACKGROUND);
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_THI_IMAGE_BACKGROUND_LOWER);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  // Create time TextLayer
  int xPos = 5; //5
  //int yPos = 80; //80
  int yPos = 100;
  s_time_layer = text_layer_create(GRect (xPos, yPos, 144-xPos, 168-yPos)); //(5, 52, 139, 50));  
  text_layer_set_background_color(s_time_layer, GColorClear); //clear
  text_layer_set_text_color(s_time_layer, GColorWhite); //white
  text_layer_set_text(s_time_layer, "00:00 xx");
  
  // Create date TextLayer
  int xDPos = 10;
  //int yDPos = 110;
  int yDPos = 130;
  s_date_layer = text_layer_create(GRect (xDPos, yDPos, 144-xDPos, 168-yDPos));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text(s_date_layer, "00/00/0000");
  //text_layer_set_text(s_date_layer, "Toha Heavy Industries");
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  // END
  
  // Battery Layer
  int xBPos = 90;
  int yBPos = 10;
  s_battery_layer = text_layer_create(GRect(xBPos, yBPos, 144-xBPos, 168-yBPos));
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  s_battery_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_12));
  text_layer_set_font(s_battery_layer, s_battery_font);
  text_layer_set_text(s_battery_layer, "OFFL");
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  
  battery_handler(battery_state_service_peek());
  // END
  
  //Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_28)); //28

  //Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  // Make sure the time is displayed from the start
  update_time();
  update_date();
}

static void main_window_unload(Window *window) {
  //Unload GFont
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  fonts_unload_custom_font(s_battery_font);
  
  //Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  //Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_battery_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  update_date();
}
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
