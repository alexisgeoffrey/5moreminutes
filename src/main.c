#include <pebble.h>
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_time_remaining_layer;
static Layer *s_progress_layer;
//static int end_hour = 23;
//static int end_minute = 50;
//static Layer *s_progressBG_layer;
float prog_difference;

static void update_time() {
	// Get a tm structure
	time_t raw_time = time(NULL);
	struct tm *tick_time = localtime(&raw_time);
	//struct tm *tick_time_gm = gmtime(&raw_time);

	
	// Write the current hours and minutes into a buffer
	static char s_buffer[20];
	//static char s_buffer[8];
	strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
	
	//time_t destination_time = raw_time + (end_hour*60*60) + (end_minute*60);
	//tick_time_gm->tm_hour = 11;
	
	struct tm *begin_time = tick_time;
	begin_time->tm_hour = 16;
	begin_time->tm_min = 0;
	begin_time->tm_sec = 0;
	time_t start_time = mktime(begin_time);
	
	struct tm *end_time = tick_time;
	end_time->tm_hour = 23;
	end_time->tm_min = 0;
	end_time->tm_sec = 0;
	time_t destination_time = mktime(end_time);
	
	//time_t compare_time = 1453924800;
	time_t raw_overall_time = destination_time - start_time;
	time_t raw_final_time = destination_time - raw_time;
	int raw_final_seconds = raw_final_time % 60;
	int raw_final_minutes = (raw_final_time / 60) % 60;
	int raw_final_hours = raw_final_time / 3600;
	
	//prog_difference = raw_final_time / raw_overall_time;
	prog_difference = .44446;
	
	struct tm *compared_time = gmtime(&raw_final_time);
	
	static char s_final_buffer[] = "00:00:00 remaining";
	
	//strftime(s_final_buffer, sizeof(s_final_buffer), "%H:%M:%S", compared_time);
	snprintf(s_final_buffer, sizeof(s_final_buffer), "%d:%d:%d", raw_final_hours, raw_final_minutes, raw_final_seconds);
	strcat(s_final_buffer, " remaining");
	
	// Display this time on the TextLayer
	//snprintf(s_final_buffer, sizeof(s_buffer), "%d.%03d", (int)(prog_difference), (int)(prog_difference * 1000) % 1000);
	//snprintf(s_final_buffer, sizeof(s_buffer), "%d", (int)(raw_overall_time));
	text_layer_set_text(s_time_layer, s_buffer);
	text_layer_set_text(s_time_remaining_layer, s_final_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
	//update_progress();
}

static void progress_update_proc(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);
	
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
	
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, GRect(5, 5, ((bounds.size.w - 10) * prog_difference), bounds.size.h - 10), 0, GCornerNone);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "prog_difference = %f", prog_difference);
}

static void main_window_load(Window *window) {
	// Get information about the Window
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	
	// Create the TextLayer with specific bounds
	s_time_layer = text_layer_create(
		GRect(0, 44, bounds.size.w, 50));
	
	s_time_remaining_layer = text_layer_create(
		GRect(0, 120, bounds.size.w, 30));
	
	// Create the progress bar layer
	s_progress_layer = layer_create(GRect(0, 100, bounds.size.w, 20));
	layer_set_update_proc(s_progress_layer, progress_update_proc);
	
	// Time Layer properties
	
	//time clock layer
	text_layer_set_background_color(s_time_layer, GColorBlack);
	text_layer_set_text_color(s_time_layer, GColorWhite);
	text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

	//testing time remaining layer
	text_layer_set_background_color(s_time_remaining_layer, GColorBlack);
	text_layer_set_text_color(s_time_remaining_layer, GColorWhite);
	text_layer_set_font(s_time_remaining_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(s_time_remaining_layer, GTextAlignmentLeft);
		
	window_set_background_color(s_main_window, GColorBlack);
	
	// Add it as a child layer to the Window's root layer
	layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_time_remaining_layer));
	layer_add_child(window_layer, s_progress_layer);
}

static void main_window_unload(Window *window) {
	// Destroy TextLayer
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_time_remaining_layer);
}

static void init() {
	// Create main Window element and assign to pointer
	s_main_window = window_create();
	
	// Set handlers to manage the elemnts inside the Window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	
	// Show the Window on the watch, with animated=true
	window_stack_push(s_main_window, true);
	
	// Make sure the time is displayed from the start
	update_time();
		
	// Register with TickTimerService
	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
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