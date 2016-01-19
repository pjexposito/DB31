// Gráficos de los iconos adaptados de http://icons.primail.ch/index.php?page=iconset_weather
// Código del tiempo adaptado de https://github.com/Niknam/futura-weather-sdk2.0.
// Lo anteriormente mencionado ya no se usa en esta version
// Código del reloj basado en https://github.com/orviwan/91-Dub-v2.0
// Con código de https://ninedof.wordpress.com/2014/05/24/pebble-sdk-2-0-tutorial-9-app-configuration/


// UUID para versión en inglés:  "a8336799-b197-4e3d-8afd-3290317c65b2"



#include "pebble.h"
  
#define KEY_IDIOMA 0
#define KEY_VIBE 1
#define KEY_DATEFORMAT 2
#define KEY_SEGUNDOS 3
#define KEY_HOURLYVIBE 4
#define KEY_BACK 5
#define KEY_CONDICION 6
#define KEY_TEMPERATURA 7
#define KEY_PIDE 8
  
static Window *window;
static Layer *window_layer;
static uint8_t batteryPercent;
static TextLayer *text_layer_hora, *text_layer_segundos, *text_layer_fecha1, *text_layer_fecha2, *text_layer_letras, *text_layer_ano, *text_layer_bateria;


int IDIOMA;  
// IDIOMA = 0, text in english  
// IDIOMA = 1, texto en español
// IDIOMA = 2, texto en frances
  
int TEMPERATURA, CONDICION;
// Se guarda la temperatura y el icono de condición meteorológica

int temporizador_meteo;
// El temporizador para que cada 10 minutos pida los datos del tiempo

int cuenta_atras_meteo;
// Son los 10 segundos que pasan hasta que vuelve a mostrar de nuevo los datos de día y año

static bool DATEFORMAT;
// DATEFORMAT = True, Formato europeo (DD/MM/AAAA)
// DATEFORMAT = False, Formato americano (MM/DD/AAAA)
  

// Vibra al perder la conexión BT. True es si, false es no.
static bool BluetoothVibe;

// Vibrar en el cambio de hora. True es si, false es no.
static bool HourlyVibe;

// True muestra el segundero. Con False, desaparece.
static bool SEGUNDOS;

// 0 muestra el fondo en blanco y negro. 1 se muestra en color. 2, en color sin sombras
static int BACK;

static bool appStarted = false;

static GBitmap *background_image;
static GBitmap *background_image_color;
static GBitmap *background_image_color2;
static GBitmap *background_image_color_ns;

static GBitmap *battery_image;
static GBitmap *time_format_image;
static GBitmap *time_format_image24;

static BitmapLayer *time_format_layer;
static BitmapLayer *background_layer;
static BitmapLayer *meter_bar_layer;
static BitmapLayer *bluetooth_layer;
static BitmapLayer *porcentaje_layer;
static BitmapLayer *battery_image_layer;
static BitmapLayer *battery_layer;

GFont fuente_hora, fuente_segundos, fuente_fecha, fuente_letras, fuente_bateria;


static void carga_preferencias(void)
  { 
    // Carga las preferencias
    IDIOMA = persist_exists(KEY_IDIOMA) ? persist_read_int(KEY_IDIOMA) : 0;
    DATEFORMAT = persist_exists(KEY_DATEFORMAT) ? persist_read_bool(KEY_DATEFORMAT) : 0;
    BluetoothVibe = persist_exists(KEY_VIBE) ? persist_read_bool(KEY_VIBE) : 1;
    SEGUNDOS = persist_exists(KEY_SEGUNDOS) ? persist_read_bool(KEY_SEGUNDOS) : 1;
    HourlyVibe = persist_exists(KEY_HOURLYVIBE) ? persist_read_bool(KEY_HOURLYVIBE) : 0;
    BACK = persist_exists(KEY_BACK) ? persist_read_int(KEY_BACK) : 0;
  
  }

static void handle_tick(struct tm *tick_time, TimeUnits units_changed);

void pide_datos_tiempo()
  {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Pidiendo datos");


  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_int16(iter, KEY_PIDE, 0);    
  app_message_outbox_send();
}

void sacudida (AccelAxisType axis, int32_t direction) {
    cuenta_atras_meteo = 10;
    static char s_temp_text[] = "9999";
  	snprintf(s_temp_text, sizeof(s_temp_text), "%do", TEMPERATURA);
    text_layer_set_text(text_layer_letras, s_temp_text); 
    static char s_icono_text[] = "9";
  	snprintf(s_icono_text, sizeof(s_icono_text), "*");
    text_layer_set_text(text_layer_ano, s_icono_text);
}




static void in_recv_handler(DictionaryIterator *iterator, void *context)
  {
  if (KEY_PIDE==1)
    {
    //Recibe los datos de configuración
    Tuple *key_idioma_tuple = dict_find(iterator, KEY_IDIOMA);
    Tuple *key_vibe_tuple = dict_find(iterator, KEY_VIBE);
    Tuple *key_dateformat_tuple = dict_find(iterator, KEY_DATEFORMAT);
    Tuple *key_segundos_tuple = dict_find(iterator, KEY_SEGUNDOS);
    Tuple *key_hourlyvibe_tuple = dict_find(iterator, KEY_HOURLYVIBE);
    Tuple *key_bw_tuple = dict_find(iterator, KEY_BACK);  

    if(strcmp(key_idioma_tuple->value->cstring, "spanish") == 0)
      persist_write_int(KEY_IDIOMA, 1);
    else if(strcmp(key_idioma_tuple->value->cstring, "english") == 0)
    persist_write_int(KEY_IDIOMA, 0);
  else if(strcmp(key_idioma_tuple->value->cstring, "french") == 0)
    persist_write_int(KEY_IDIOMA, 2);
  else if(strcmp(key_idioma_tuple->value->cstring, "german") == 0)
    persist_write_int(KEY_IDIOMA, 3);
  else if(strcmp(key_idioma_tuple->value->cstring, "italian") == 0)
    persist_write_int(KEY_IDIOMA, 4);
  else if(strcmp(key_idioma_tuple->value->cstring, "portuguese") == 0)
    persist_write_int(KEY_IDIOMA, 5);  
  else if(strcmp(key_idioma_tuple->value->cstring, "dutch") == 0)
    persist_write_int(KEY_IDIOMA, 6);
  else
    persist_write_int(KEY_IDIOMA, 0);
 
  
  if(strcmp(key_dateformat_tuple->value->cstring, "DDMM") == 0)
      persist_write_bool(KEY_DATEFORMAT, 1);
  else if(strcmp(key_dateformat_tuple->value->cstring, "MMDD") == 0)
    persist_write_bool(KEY_DATEFORMAT, 0);  
    
  if(strcmp(key_vibe_tuple->value->cstring, "on") == 0)
      persist_write_bool(KEY_VIBE, 1);
  else if(strcmp(key_vibe_tuple->value->cstring, "off") == 0)
      persist_write_bool(KEY_VIBE, 0); 
    
  if(strcmp(key_segundos_tuple->value->cstring, "on") == 0)
      persist_write_bool(KEY_SEGUNDOS, 1);
  else if(strcmp(key_segundos_tuple->value->cstring, "off") == 0)
      persist_write_bool(KEY_SEGUNDOS, 0);     
    
  if(strcmp(key_hourlyvibe_tuple->value->cstring, "on") == 0)
     persist_write_bool(KEY_HOURLYVIBE, 1);
  else if(strcmp(key_hourlyvibe_tuple->value->cstring, "off") == 0)
     persist_write_bool(KEY_HOURLYVIBE, 0);

  if(strcmp(key_bw_tuple->value->cstring, "bw") == 0)
     persist_write_int(KEY_BACK, 0);
  else if(strcmp(key_bw_tuple->value->cstring, "color") == 0)
     persist_write_int(KEY_BACK, 1);  
  else if(strcmp(key_bw_tuple->value->cstring, "ns") == 0)
     persist_write_int(KEY_BACK, 2);  
  
  // Vuelve a dibujar el reloj tras cerrar las preferencias
  carga_preferencias();
  
    if (BluetoothVibe)
    layer_set_hidden(bitmap_layer_get_layer(meter_bar_layer), false);
  else
    layer_set_hidden(bitmap_layer_get_layer(meter_bar_layer), true);
  
  if(SEGUNDOS)
  {
    tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
    layer_set_bounds(text_layer_get_layer(text_layer_hora), GRect(3, 0, 100, 70));

  }
  else
  {
    tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
    layer_set_bounds(text_layer_get_layer(text_layer_hora), GRect(10, 0, 100, 70));
  }
  layer_set_hidden(text_layer_get_layer(text_layer_segundos), !SEGUNDOS);
  
  
    #ifdef PBL_COLOR 
      if (BACK==0)
        bitmap_layer_set_bitmap(background_layer, background_image);
      else if (BACK==1) 
        if (SEGUNDOS)
          bitmap_layer_set_bitmap(background_layer, background_image_color);
        else
          bitmap_layer_set_bitmap(background_layer, background_image_color2);
      else if (BACK==2) 
        bitmap_layer_set_bitmap(background_layer, background_image_color_ns);  
    #endif



    time_t now = time(NULL);
    struct tm *tick_time = localtime(&now);  
    handle_tick(tick_time, YEAR_UNIT + MONTH_UNIT + DAY_UNIT + HOUR_UNIT + MINUTE_UNIT + SECOND_UNIT);
  }
  else
    {
    Tuple *key_temperatura_tuple = dict_find(iterator, KEY_TEMPERATURA);
    Tuple *key_condicion_tuple = dict_find(iterator, KEY_CONDICION);


    
    TEMPERATURA = key_temperatura_tuple->value->int8; 
    CONDICION = key_condicion_tuple->value->int8;

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Temperatura %d - Condición %d", TEMPERATURA, CONDICION);

  }
}


void change_battery_icon(bool charging) {
  gbitmap_destroy(battery_image);
  if(charging) {
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGE);
  }
  else {
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  }  
  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  layer_mark_dirty(bitmap_layer_get_layer(battery_image_layer));
}



static void update_battery(BatteryChargeState charge_state) {
  batteryPercent = charge_state.charge_percent;

  if(batteryPercent==100) 
  {
	  change_battery_icon(false);
	  layer_set_hidden(bitmap_layer_get_layer(battery_layer), false);
    layer_set_hidden(text_layer_get_layer(text_layer_bateria),true);
    layer_set_hidden(bitmap_layer_get_layer(porcentaje_layer),true);
    return;
  }

  layer_set_hidden(bitmap_layer_get_layer(battery_layer), charge_state.is_charging);
  change_battery_icon(charge_state.is_charging);  
  layer_set_hidden(text_layer_get_layer(text_layer_bateria),charge_state.is_charging);
  layer_set_hidden(bitmap_layer_get_layer(porcentaje_layer),charge_state.is_charging);

  static char s_time_text[] = "00";
  snprintf(s_time_text, sizeof(s_time_text), "%i", charge_state.charge_percent);
  text_layer_set_text(text_layer_bateria, s_time_text); 
}

static void toggle_bluetooth_icon(bool connected) {
  if(appStarted && !connected && BluetoothVibe) {
    static uint32_t const segments[] = { 200, 100, 100, 100, 500 };
    VibePattern pat = {
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments),
      };
    vibes_enqueue_custom_pattern(pat);
    }
  layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), !connected);
}

void bluetooth_connection_callback(bool connected) {
  toggle_bluetooth_icon(connected);
}

void battery_layer_update_callback(Layer *me, GContext* ctx) {        
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(2, 2, ((batteryPercent/100.0)*11.0), 3), 0, GCornerNone);
}


static void update_days(struct tm *tick_time) {
  const char *dias_es[] = {"DOM", "LUN", "MAR", "MIE", "JUE", "VIE", "SAB"};
  const char *dias_en[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
  const char *dias_fr[] = {"DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM"};
  const char *dias_de[] = {"SON", "MON", "DIE", "MIT", "DON", "FRE", "SAM"};
  const char *dias_it[] = {"DOM", "LUN", "MAR", "MER", "GIO", "VEN", "SAB"};
  const char *dias_pt[] = {"DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SAB"};
  const char *dias_nl[] = {"ZON", "MAA", "DIN", "WOE", "DON", "VRI", "ZAT"};

  switch (IDIOMA)
  {
  case 0:
    text_layer_set_text(text_layer_letras, dias_en[tick_time->tm_wday]); 
    break;
  case 1:
    text_layer_set_text(text_layer_letras, dias_es[tick_time->tm_wday]); 
    break;
  case 2:
    text_layer_set_text(text_layer_letras, dias_fr[tick_time->tm_wday]); 
    break;    
  case 3:
    text_layer_set_text(text_layer_letras, dias_de[tick_time->tm_wday]); 
    break;   
  case 4:
    text_layer_set_text(text_layer_letras, dias_it[tick_time->tm_wday]); 
    break;   
  case 5:
    text_layer_set_text(text_layer_letras, dias_pt[tick_time->tm_wday]); 
    break;  
  case 6:
    text_layer_set_text(text_layer_letras, dias_nl[tick_time->tm_wday]); 
    break;     
  default:
    text_layer_set_text(text_layer_letras, dias_en[tick_time->tm_wday]); 
    break;    
  }

  if (DATEFORMAT)
    {
  	  static char s_time_text[] = "00";
  	  strftime(s_time_text, sizeof(s_time_text), "%d", tick_time);
  	  text_layer_set_text(text_layer_fecha1, s_time_text); 
    }
  else
    {
  	  static char s_time_text[] = "00";
  	  strftime(s_time_text, sizeof(s_time_text), "%d", tick_time);
  	  text_layer_set_text(text_layer_fecha2, s_time_text);  
    }  
}

static void update_months(struct tm *tick_time) {
  if (DATEFORMAT)
  {	  
	  static char s_time_text[] = "00";
	  strftime(s_time_text, sizeof(s_time_text), "%m", tick_time);
	  text_layer_set_text(text_layer_fecha2, s_time_text);
  }
  else
  {
	  static char s_time_text[] = "00";
	  strftime(s_time_text, sizeof(s_time_text), "%m", tick_time);
	  text_layer_set_text(text_layer_fecha1, s_time_text); 
  }  
    
}

static void update_years(struct tm *tick_time) {
  static char s_time_text[] = "00";
  strftime(s_time_text, sizeof(s_time_text), "%y", tick_time);
  text_layer_set_text(text_layer_ano, s_time_text);
}

static void update_hours(struct tm *tick_time) {
  if(appStarted && HourlyVibe) {
    vibes_short_pulse();
  }
  if (!clock_is_24h_style()) {
    if (tick_time->tm_hour >= 12) {
          bitmap_layer_set_bitmap(time_format_layer, time_format_image); 

      layer_set_hidden(bitmap_layer_get_layer(time_format_layer), false);
    } 
    else {
      layer_set_hidden(bitmap_layer_get_layer(time_format_layer), true);
    }
}
}

static void update_minutes(struct tm *tick_time) {
    static char s_time_text[] = "00:00";
    if (clock_is_24h_style()) 
	    strftime(s_time_text, sizeof(s_time_text), "%R:%M", tick_time);
     else 
	     strftime(s_time_text, sizeof(s_time_text), "%l:%M", tick_time);      

    text_layer_set_text(text_layer_hora, s_time_text);
    temporizador_meteo++;
    if (temporizador_meteo==10){
      pide_datos_tiempo();
      temporizador_meteo = 0;
    }
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Hora es %s", s_time_text);

}

static void update_seconds(struct tm *tick_time) {
  if (cuenta_atras_meteo>0)
    cuenta_atras_meteo--;
  
  if (cuenta_atras_meteo==0)
    {
     update_days (tick_time);
     update_years (tick_time);
     cuenta_atras_meteo = -1;
    }
  
  if (SEGUNDOS)
  {
	  static char s_time_text[] = "00";
	  strftime(s_time_text, sizeof(s_time_text), "%S", tick_time);
	  text_layer_set_text(text_layer_segundos, s_time_text);
  }

}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  if (units_changed & YEAR_UNIT) update_years(tick_time);
  if (units_changed & MONTH_UNIT) update_months(tick_time);
  if (units_changed & DAY_UNIT) update_days(tick_time);
  if (units_changed & HOUR_UNIT) update_hours(tick_time);
  if (units_changed & MINUTE_UNIT) update_minutes(tick_time);
  if (units_changed & SECOND_UNIT) update_seconds(tick_time);
}


static TextLayer* crea_capa_texto(GRect location, GColor colour, GColor background, GFont fuente, GTextAlignment alignment)
{
	TextLayer *layer = text_layer_create(location);
	text_layer_set_text_color(layer, colour);
	text_layer_set_background_color(layer, background);
	text_layer_set_font(layer, fuente);
	text_layer_set_text_alignment(layer, alignment);
	return layer;
}

static BitmapLayer* crea_capa_grafica(int x, int y, const uint8_t IMAGEN, bool Carga)
{
  GBitmap *layer_image = gbitmap_create_with_resource(IMAGEN);
    GRect bitmap_bounds = gbitmap_get_bounds(layer_image);

  GRect frame = GRect(x, y, bitmap_bounds.size.w, bitmap_bounds.size.h);
  BitmapLayer *layer = bitmap_layer_create(frame);
  
  if (Carga)
    bitmap_layer_set_bitmap(layer, layer_image);
	return layer;
}

static void init(void) {
  
  carga_preferencias();
  
  TEMPERATURA = 0;
  CONDICION = 0;
  
  window = window_create();
  if (window == NULL) {
      return;
  }
  window_stack_push(window, true);
  window_layer = window_get_root_layer(window);
  
  fuente_hora = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUENTE_HORA_64));
  fuente_segundos = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUENTE_SEGUNDOS_32));
  fuente_fecha = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUENTE_FECHA_32));
  fuente_letras = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUENTE_LETRAS_24));
  fuente_bateria = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUENTE_BATERIA_8));
  
	
  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  background_image_color = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_COLOR);
  background_image_color2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_COLOR2);

  background_image_color_ns = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_COLOR_NS);
  background_layer = bitmap_layer_create(layer_get_frame(window_layer));
  #ifdef PBL_COLOR 
    if (BACK==0)
      bitmap_layer_set_bitmap(background_layer, background_image);
    else if (BACK==1) 
      if (SEGUNDOS)
        bitmap_layer_set_bitmap(background_layer, background_image_color);
      else
        bitmap_layer_set_bitmap(background_layer, background_image_color2);
  else if (BACK==2) 
      bitmap_layer_set_bitmap(background_layer, background_image_color_ns);  
  #else
    bitmap_layer_set_bitmap(background_layer, background_image);  
  #endif


  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
  
  porcentaje_layer = crea_capa_grafica(27, 41, RESOURCE_ID_IMAGE_TINY_PERCENT, 1);
  layer_add_child(window_layer, bitmap_layer_get_layer(porcentaje_layer));
  
  meter_bar_layer = crea_capa_grafica(13, 70, RESOURCE_ID_IMAGE_METER_BAR, 1);
  layer_add_child(window_layer, bitmap_layer_get_layer(meter_bar_layer));
  if (BluetoothVibe == 1)
    layer_add_child(window_layer, bitmap_layer_get_layer(meter_bar_layer)); 
  
  bluetooth_layer = crea_capa_grafica(29, 70, RESOURCE_ID_IMAGE_BLUETOOTH, 1);
  layer_add_child(window_layer, bitmap_layer_get_layer(bluetooth_layer));

  
  time_format_layer = bitmap_layer_create(GRect(13,83,19,8));
  time_format_image24 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_24_HOUR_MODE);
  time_format_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PM_MODE);
  if (clock_is_24h_style()) {
    time_format_image24 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_24_HOUR_MODE);
    bitmap_layer_set_bitmap(time_format_layer, time_format_image24); 
  }
  layer_add_child(window_layer, bitmap_layer_get_layer(time_format_layer));
  
  
  battery_image_layer = crea_capa_grafica(13, 51, RESOURCE_ID_IMAGE_BATTERY, 1);
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_image_layer));
  
  battery_layer = crea_capa_grafica(13, 51, RESOURCE_ID_IMAGE_BATTERY ,0);
  layer_set_update_proc(bitmap_layer_get_layer(battery_layer), battery_layer_update_callback);
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_layer));
  
  // EMPIEZAN LOS CARACTERES
  
  text_layer_hora = crea_capa_texto(GRect(0, 69, 130, 70), GColorBlack, GColorClear, fuente_hora, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(text_layer_hora));

  text_layer_segundos = crea_capa_texto(GRect(107, 106, 40, 40), GColorBlack, GColorClear, fuente_segundos, GTextAlignmentLeft);
	layer_add_child(window_layer, text_layer_get_layer(text_layer_segundos));

  text_layer_fecha1 = crea_capa_texto(GRect(72, 62, 40, 40), GColorBlack, GColorClear, fuente_fecha, GTextAlignmentLeft);
	layer_add_child(window_layer, text_layer_get_layer(text_layer_fecha1));

  text_layer_fecha2 = crea_capa_texto(GRect(110, 62, 40, 40), GColorBlack, GColorClear, fuente_fecha, GTextAlignmentLeft);
	layer_add_child(window_layer, text_layer_get_layer(text_layer_fecha2));

  text_layer_letras = crea_capa_texto(GRect(41, 33, 70, 40), GColorBlack, GColorClear, fuente_letras, GTextAlignmentLeft);
	layer_add_child(window_layer, text_layer_get_layer(text_layer_letras));

  
  text_layer_ano = crea_capa_texto(GRect(102, 33, 70, 40), GColorBlack, GColorClear, fuente_letras, GTextAlignmentLeft);
	layer_add_child(window_layer, text_layer_get_layer(text_layer_ano));

  text_layer_bateria = crea_capa_texto(GRect(10, 41, 15, 15), GColorBlack, GColorClear, fuente_bateria, GTextAlignmentRight);
	layer_add_child(window_layer, text_layer_get_layer(text_layer_bateria));
  
  // AQUI ACABAN LOS CARACTERES
 
  toggle_bluetooth_icon(bluetooth_connection_service_peek());
  update_battery(battery_state_service_peek());

  appStarted = true;
  
  // Se evita la pantalla blanca al iniciar.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);  
  handle_tick(tick_time, YEAR_UNIT + MONTH_UNIT + DAY_UNIT + HOUR_UNIT + MINUTE_UNIT + SECOND_UNIT);

  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
  
  
  if(SEGUNDOS)
  {
    tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
    layer_set_bounds(text_layer_get_layer(text_layer_hora), GRect(3, 0, 100, 70));
  }
  else
  {
    tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
    layer_set_bounds(text_layer_get_layer(text_layer_hora), GRect(10, 0, 100, 70));
  }
  layer_set_hidden(text_layer_get_layer(text_layer_segundos), !SEGUNDOS);

  bluetooth_connection_service_subscribe(bluetooth_connection_callback);
  battery_state_service_subscribe(&update_battery);
  //app_message_register_inbox_received((AppMessageInboxReceived) in_recv_handler);
  //app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  app_message_register_inbox_received(in_recv_handler);
  app_message_open(64, 64);
  accel_tap_service_subscribe (sacudida);

}


static void deinit(void) {

  accel_tap_service_unsubscribe ();
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  app_message_deregister_callbacks();

  layer_remove_from_parent(bitmap_layer_get_layer(background_layer));
  bitmap_layer_destroy(background_layer);
  gbitmap_destroy(background_image);
  background_image = NULL;
  
  layer_remove_from_parent(bitmap_layer_get_layer(meter_bar_layer));
  bitmap_layer_destroy(meter_bar_layer);
	
  layer_remove_from_parent(bitmap_layer_get_layer(bluetooth_layer));
  bitmap_layer_destroy(bluetooth_layer);
	
  layer_remove_from_parent(bitmap_layer_get_layer(battery_layer));
  bitmap_layer_destroy(battery_layer);
  gbitmap_destroy(battery_image);
  battery_image = NULL;
	
  layer_remove_from_parent(bitmap_layer_get_layer(battery_image_layer));
  bitmap_layer_destroy(battery_image_layer);

  layer_remove_from_parent(bitmap_layer_get_layer(time_format_layer));
  bitmap_layer_destroy(time_format_layer);
  gbitmap_destroy(time_format_image);
  time_format_image = NULL;
	
  text_layer_destroy(text_layer_hora);
  text_layer_destroy(text_layer_segundos);
  text_layer_destroy(text_layer_fecha1);
  text_layer_destroy(text_layer_letras);
  text_layer_destroy(text_layer_ano);
  text_layer_destroy(text_layer_fecha2);
  text_layer_destroy(text_layer_bateria);
	
  
  fonts_unload_custom_font(fuente_hora);
  fonts_unload_custom_font(fuente_segundos);
  fonts_unload_custom_font(fuente_fecha);
  fonts_unload_custom_font(fuente_letras);
  fonts_unload_custom_font(fuente_bateria);
  
  layer_remove_from_parent(window_layer);
  layer_destroy(window_layer);
	
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}