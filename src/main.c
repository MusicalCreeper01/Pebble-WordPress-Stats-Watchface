#include <pebble.h>

#define day_views_count 0
#define views_best 1
#define day_views 2


static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer, *s_views_today_layer, *s_best_views_layer;

static GFont s_time_font;
static GFont s_date_font;

static uint8_t *s_wp_views;
static int s_wp_views_length;
static int s_wp_best_views = 100;
static int s_wp_today_views = 100;

static Layer *s_wp_views_layer;

static GBitmap *s_bitmap_bluetooth;
static BitmapLayer *s_bitmap_layer_bluetooth;

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
	Tuple *day_views_count_tuple = dict_find(iter, day_views_count);
	s_wp_today_views = (int) day_views_count_tuple->value->int32;
	
	static char buf[] = "00000000000";
	snprintf(buf, sizeof(buf), "Today %i", s_wp_today_views);
	text_layer_set_text(s_views_today_layer, buf);
	
	
	Tuple *best_views_count_tuple = dict_find(iter, views_best);
	s_wp_best_views = (int) best_views_count_tuple->value->int32;
	
	static char buf2[] = "00000000000";
	snprintf(buf2, sizeof(buf2), "Best %i", s_wp_best_views);
	text_layer_set_text(s_best_views_layer, buf2);

	
	// Expected length of the binary data
	const int length = 32;

	// Does this message contain the data tuple?
	Tuple *data_tuple = dict_find(iter, day_views);
	if(data_tuple) {
		// Read the binary data value
		uint8_t *data = data_tuple->value->data;
		uint8_t byte_zero = data[0];
		
		//APP_LOG(APP_LOG_LEVEL_INFO, "%i\n", byte_zero);
		
		s_wp_views_length = (int) data_tuple->length;
		s_wp_views = malloc(data_tuple->length);
		// Store into an app-defined buffer
		memcpy(s_wp_views, data, length);
	}
	
	/*
	// Expected length of the binary data
	const int length = 120;

	// Does this message contain the data tuple?
	Tuple *data_tuple = dict_find(iter, day_views);
	if(data_tuple) {
		// Read the binary data value
		uint8_t *data = data_tuple->value->data;

		// Store into an app-defined buffer
		memcpy(s_wp_views, data, length);
	}*/
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void bluetooth_callback(bool connected) {
	// Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layer_bluetooth), !connected);

  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}

static void update_time() {
	// Get a tm structure
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);

	// Write the current hours and minutes into a buffer
	static char s_buffer[8];
	strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
			 "%H:%M" : "%I:%M", tick_time);

	// Display this time on the TextLayer
	text_layer_set_text(s_time_layer, s_buffer);

	// Copy date into buffer from tm structure
	static char date_buffer[16];
	strftime(date_buffer, sizeof(date_buffer), "%a %b %d", tick_time);

	// Show the date
	text_layer_set_text(s_date_layer, date_buffer);
}

static void getStatsUpdate(){
	
	
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
	
  // Get wordpress update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void draw_date_and_time(Window *window){
	// Get information about the Window
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	// Create the TextLayer with specific bounds
	s_time_layer = text_layer_create(
		GRect(0, 0, bounds.size.w-5, 32));

	// Improve the layout to be more like a watchface
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorWhite);
	text_layer_set_text(s_time_layer, "00:00");
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);

	// Create GFont
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_DISPLAY_32));

	// Apply to TextLayer
	text_layer_set_font(s_time_layer, s_time_font);

	// Add it as a child layer to the Window's root layer
	layer_add_child(window_layer, text_layer_get_layer(s_time_layer));


	// Create date TextLayer
	s_date_layer = text_layer_create(GRect(0, 35, bounds.size.w-5, 30));
	text_layer_set_text_color(s_date_layer, GColorWhite);
	text_layer_set_background_color(s_date_layer, GColorClear);
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);

	s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_DISPLAY_14));

	text_layer_set_font(s_date_layer, s_date_font);

	// Add to Window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));


	
	// Create today's views TextLayer
	s_views_today_layer = text_layer_create(GRect(0, (bounds.size.h/2)-35, bounds.size.w/2, 30));
	text_layer_set_text_color(s_views_today_layer, GColorWhite);
	text_layer_set_background_color(s_views_today_layer, GColorClear);
	text_layer_set_text_alignment(s_views_today_layer, GTextAlignmentLeft);

	s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_DISPLAY_14));

	text_layer_set_font(s_views_today_layer, s_date_font);

	// Add to Window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_views_today_layer));

	// Create overall best views TextLayer
	s_best_views_layer = text_layer_create(GRect(bounds.size.w/2, (bounds.size.h/2)-35, bounds.size.w/2, 30));
	text_layer_set_text_color(s_best_views_layer, GColorWhite);
	text_layer_set_background_color(s_best_views_layer, GColorClear);
	text_layer_set_text_alignment(s_best_views_layer, GTextAlignmentRight);

	s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_DISPLAY_14));

	text_layer_set_font(s_best_views_layer, s_date_font);

	// Add to Window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_best_views_layer));
	
//s_best_views_layer
}

static void stats_wordpress_proc(Layer *layer, GContext *ctx){
	if(s_wp_views){
		GRect bounds = layer_get_bounds(layer);

		int length = s_wp_views_length; //sizeof(s_wp_views) / sizeof(s_wp_views[0]);
		if(length > 0){
			int width = bounds.size.w / length;
			int pad = (bounds.size.w % length)/2;
			
			// Draw the background
			graphics_context_set_fill_color(ctx, GColorWhite);
			graphics_fill_rect(ctx, GRect(bounds.origin.x+pad, bounds.origin.y, bounds.size.w-(pad*2), bounds.size.h), 0, GCornerNone);
			
			//APP_LOG(APP_LOG_LEVEL_INFO, "screen width: %i, bar width: %i",  bounds.size.w, width);
			int i;
			for(i = 0; i < length; i++){
				int viewCount = s_wp_views[i];

				//int height = viewCount;
				int height = (viewCount * bounds.size.h / s_wp_best_views);

				//APP_LOG(APP_LOG_LEVEL_INFO, "%i\n", viewCount);

				graphics_context_set_fill_color(ctx, GColorBlack);
				graphics_fill_rect(ctx, GRect(pad+(width*i), bounds.size.h-height, width, height), 0, GCornerNone);
			}
		}

		layer_mark_dirty(s_wp_views_layer);
	}
}

static void main_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	
	draw_date_and_time(window);
	
	// Create battery meter Layer
	s_wp_views_layer = layer_create(GRect(0, bounds.size.h/2, bounds.size.w, bounds.size.h/2));
	layer_set_update_proc(s_wp_views_layer, stats_wordpress_proc);

	// Add to Window
	layer_add_child(window_get_root_layer(window), s_wp_views_layer);
	
	
	// Create the Bluetooth icon GBitmap
	s_bitmap_bluetooth = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);

	// Create the BitmapLayer to display the GBitmap
	s_bitmap_layer_bluetooth = bitmap_layer_create(GRect(0, 0, 28, 28));
	bitmap_layer_set_bitmap(s_bitmap_layer_bluetooth, s_bitmap_bluetooth);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bitmap_layer_bluetooth));	
}

static void main_window_unload(Window *window) {
	// Destroy TextLayer
	text_layer_destroy(s_time_layer);

	// Unload GFont
	fonts_unload_custom_font(s_time_font);

	fonts_unload_custom_font(s_date_font);

	text_layer_destroy(s_date_layer);
	
	layer_destroy(s_wp_views_layer);
	
	gbitmap_destroy(s_bitmap_bluetooth);
	bitmap_layer_destroy(s_bitmap_layer_bluetooth);
}


static void init() {
	// Create main Window element and assign to pointer
	s_main_window = window_create();

	// Set the background color
	window_set_background_color(s_main_window, GColorBlack);

	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});

	// Show the Window on the watch, with animated=true
	window_stack_push(s_main_window, true);

	// Make sure the time is displayed from the start
	update_time();

	// Register with TickTimerService
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);

	// Open AppMessage
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

	connection_service_subscribe((ConnectionHandlers) {
	  .pebble_app_connection_handler = bluetooth_callback
	});
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