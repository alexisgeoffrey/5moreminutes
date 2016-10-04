#include <pebble.h>

#define START_TIME_IN 0
#define END_TIME_IN   1

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_time_remaining_layer;
static Layer *s_progress_layer;
static float prog_difference = 0.5;
static int input_Start;
static int input_End;

static void update_time() {
	// Get a tm structure
	time_t raw_time = time(NULL);
	struct tm *tick_time = localtime(&raw_time);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "tick_time->tm_hour: %d", tick_time->tm_hour);
	
	// Write the current hours and minutes into a buffer
	static char s_buffer[8];
	strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

	struct tm *begin_time = localtime(&raw_time);
	//struct tm *end_time = localtime(&raw_time);

	begin_time->tm_hour = input_Start;
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "begin_time->tm_hour = input_Start: %d", begin_time->tm_hour);
	begin_time->tm_min = 0;
	begin_time->tm_sec = 0;
	
	time_t start_time = mktime(begin_time);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "start_time: %li", start_time);
	
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "begin_time parameters after mktime: tm_hour=%d, tm_min=%d, tm_sec=%d, tm_mday=%d, tm_mon=%d, tm_year=%d",
	//		begin_time->tm_hour, begin_time->tm_min, begin_time->tm_sec, begin_time->tm_mday, begin_time->tm_mon, begin_time->tm_year);
	
	struct tm *end_time = localtime(&raw_time);
	end_time->tm_hour = input_End;
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "end_time->tm_hour = input_End: %d", end_time->tm_hour);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "start_time: %li", start_time);
	end_time->tm_min = 0;
	end_time->tm_sec = 0;
	
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "end_time parameters before mktime: tm_hour=%d, tm_min=%d, tm_sec=%d, tm_mday=%d, tm_mon=%d, tm_year=%d",
	//		begin_time->tm_hour, begin_time->tm_min, begin_time->tm_sec, begin_time->tm_mday, begin_time->tm_mon, begin_time->tm_year);
	
	time_t destination_time = mktime(end_time);
	
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "end_time parameters after mktime: tm_hour=%d, tm_min=%d, tm_sec=%d, tm_mday=%d, tm_mon=%d, tm_year=%d",
	//		end_time->tm_hour, end_time->tm_min, end_time->tm_sec, end_time->tm_mday, end_time->tm_mon, end_time->tm_year);	APP_LOG(APP_LOG_LEVEL_DEBUG, "destination_time: %li", destination_time);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "destination_time: %li", destination_time);
	
	time_t raw_overall_time = destination_time - start_time;
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "raw_overall_time: %li", raw_overall_time);
	time_t raw_final_time = destination_time - raw_time;
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "raw_final_time: %li", raw_final_time);
	int raw_final_seconds = raw_final_time % 60;
	int raw_final_minutes = (raw_final_time / 60) % 60;
	int raw_final_hours = raw_final_time / 3600;
	
	prog_difference = ((float)raw_final_time / raw_overall_time);
	layer_mark_dirty(s_progress_layer);
		
	static char s_final_buffer[] = "00:00:00 remaining";
	
	//strftime(s_final_buffer, sizeof(s_final_buffer), "%H:%M:%S", compared_time);
	snprintf(s_final_buffer, sizeof(s_final_buffer), "%02d:%02d:%02d", raw_final_hours, raw_final_minutes, raw_final_seconds);
	strcat(s_final_buffer, " remaining");
	
	// Display this time on the TextLayer
	//snprintf(s_final_buffer, sizeof(s_final_buffer), "%d %d", (int)(final_input_Start), (int)(final_input_End));
	text_layer_set_text(s_time_layer, s_buffer);
	text_layer_set_text(s_time_remaining_layer, s_final_buffer);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  	Tuple *start_time_t = dict_find(iter, START_TIME_IN);
  	Tuple *end_time_t = dict_find(iter, END_TIME_IN);
	//dict_find(received, KEY_A)->value->uint32

	//APP_LOG(APP_LOG_LEVEL_DEBUG, "input_Start after message received = %d", input_Start);
	input_Start = start_time_t->value->int32;
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "input_Start after storage = %d", input_Start);
    input_End = end_time_t->value->int32;
	
	persist_write_int(START_TIME_IN, input_Start);
	persist_write_int(END_TIME_IN, input_End);
	
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

static void progress_update_proc(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);
	
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
	
	graphics_context_set_fill_color(ctx, GColorBlack);
	if(prog_difference < 0) {prog_difference = .5;}
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "prog_diff before crash: %f", prog_difference);
	int bar_size = bounds.size.w-10 - ((bounds.size.w - 10) * prog_difference);
	graphics_fill_rect(ctx, GRect(5, 5, bar_size, bounds.size.h - 10), 0, GCornerNone);
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
	// Destroy layers
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_time_remaining_layer);
	layer_destroy(s_progress_layer);
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
	
	// Persist data read
	input_Start = 0;
	input_End = 1;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "contained in persist: %d", START_TIME_IN);
	if (persist_exists(START_TIME_IN)) {
  		input_Start = persist_read_int(START_TIME_IN);
	}
	if (persist_exists(END_TIME_IN)) {
		input_End = persist_read_int(END_TIME_IN);
	}
	
	// Make sure the time is displayed from the start
	update_time();
		
	// Register with TickTimerService
	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
	
	app_message_register_inbox_received(inbox_received_handler);
 	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "contained in persist: %s", tomato);
}

static void deinit() {
	tick_timer_service_unsubscribe();
	app_message_deregister_callbacks();
	
	// Destroy Window
	window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}