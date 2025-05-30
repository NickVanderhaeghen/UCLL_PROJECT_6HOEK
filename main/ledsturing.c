#include "include/ledsturing.h"

const RGBColor kleurenlijst_rgb[] = {
    {"OFF",  0, 0,   0},
    {"ROOD",  255, 0,   0},
    {"GROEN",  0, 255,   0},
    {"BLAUW",  0,   0, 255},
    {"WIT", 255, 255, 255},
    {"GEEL", 255, 255,   0},
    {"PAARS", 128,   0, 128},
    {"CYAAN",  0, 255, 255},
    {"ORANJE", 255, 102,   0},
    {"ROZE", 255, 153, 153},  //GECHECKD

    // Extra kleuren ->10
    {"TURQUOISE",  64, 224, 208},
    {"LICHTBLAUW", 173, 216, 230},
    {"DONKERBLAUW",  0,   0, 139},
    {"GRIJS", 128, 128, 128},
    {"LICHTGRIJS", 211, 211, 211},
    {"BRUIN", 139,  69,  19},
    {"GOUDBRUIN", 218, 165,  32},
    {"INDIGO",  75,   0, 130},
    {"MUNT", 152, 255, 152},
    {"ZALM", 255, 204, 204},  //GECHECKD

    // Extra kleurn (11 kleuren) ->11
    {"LICHTROOD", 155, 0, 0},   //GECHECKD
    {"LICHTGROEN", 0, 230, 0},   //GECHECKD
    {"BABYBLAUW", 51, 214, 255},   //GECHECKD
    {"BABYBLAUW2", 51, 214, 255},  //GECHECKD
    {"ROZE2", 255, 153, 153},  //GECHECKD
    {"SMARAGD", 80, 200, 120},
    {"DONKERGROEN", 0, 100, 0},
    {"DONKERPAARS", 75, 0, 130},
    {"LICHTPAARS", 155, 0, 155},
    {"FUCHSIA", 255, 0, 255},
    {"LICHTORANJE", 255, 200, 0}
};

static led_strip_handle_t led_strip;
RGBColor huidige_kleur = {0};
uint8_t huidige_kleur_index = 0;
RGBColor displayColor = {0};

float dim_brighten_factor = 0.7;

void config_led(){
    led_strip_config_t strip_config = {
        .strip_gpio_num = CONFIG_GPIO_LED,
        .max_leds = CONFIG_LEDS_COUNT
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    led_strip_clear(led_strip);
}

void all_same(const RGBColor* color){
    for(int i= 0; i < CONFIG_LEDS_COUNT; i++){
        set_color(color, i);
    }
    led_strip_refresh(led_strip);
}

void blink(){
    
    for(int i= 0; i <= 6; i++){
        for(int j= 0; j < CONFIG_LEDS_COUNT; j++){
            if(i%2 == 0){
                set_color(&huidige_kleur, i);
            }
            else{
                set_color("OFF", i);
            }
        }
        led_strip_refresh(led_strip);
        vTaskDelay(500);
    }
}

void rgb_fading(){
    static int r = CONFIG_FADING_RGB_VALUE_r;
    static int g = CONFIG_FADING_RGB_VALUE_g;
    static int b = CONFIG_FADING_RGB_VALUE_b;

    while (1){
        if(r > 0 && b == 0){
            r--;
            g++;
        }
        if(g > 0 && r == 0){
            g--;
            b++;
        }
        if(b > 0 && g == 0){
            r++;
            b--;
        }
        for(int i= 0; i < CONFIG_LEDS_COUNT; i++){
            led_strip_set_pixel(led_strip, i, r, g, b);
        }
        led_strip_refresh(led_strip);
        
        vTaskDelay(CONFIG_FADING_SPEED / portTICK_PERIOD_MS);
    }
}

void next_color(){
    RGBColor huidige_kleur;
    printf("volgende kleur is %s", kleurenlijst_rgb[huidige_kleur_index+1].kleurnaam);
    if((huidige_kleur_index + 1) < 30){huidige_kleur_index += 1;}
    else{huidige_kleur_index = 0;}
    huidige_kleur = kleurenlijst_rgb[huidige_kleur_index];
    all_same(&huidige_kleur);
}


void helderheid_omhoog(){    
    if((displayColor.r / dim_brighten_factor) <= 255 && (displayColor.g / dim_brighten_factor) <= 255 && (displayColor.b / dim_brighten_factor) <= 255){


        huidige_kleur.r = displayColor.r / dim_brighten_factor;
        huidige_kleur.g = displayColor.g / dim_brighten_factor;
        huidige_kleur.b = displayColor.b / dim_brighten_factor;

    }
    displayColor = huidige_kleur;
    
    all_same(&displayColor);
}

void helderheid_omlaag(){
    if((displayColor.r * dim_brighten_factor) > 9){
        huidige_kleur.r = displayColor.r * dim_brighten_factor;
    }
    if((displayColor.g * dim_brighten_factor) > 9){
        huidige_kleur.g = displayColor.g * dim_brighten_factor;
    }
    if((displayColor.b * dim_brighten_factor) > 9){
        huidige_kleur.b = displayColor.b * dim_brighten_factor;
    }
    displayColor = huidige_kleur;
    
    all_same(&displayColor);
}


void set_color(const RGBColor* color, int index){    
    led_strip_set_pixel(led_strip, index, color -> r, color -> g, color -> b);
}


void queue_to_led_task(void *pvParameters){
    queue_msg_t received_v;

    while (1) {
        if (xQueueReceive(main_queue, &received_v, portMAX_DELAY) == pdPASS) {
            if(strcmp(received_v.kleurnaam, "next_color") == 0){
                next_color();
            }
            else if(strcmp(received_v.kleurnaam, "HELDERHEID_OMHOOG") == 0){                
                helderheid_omhoog();
            }
            else if(strcmp(received_v.kleurnaam, "HELDERHEID_OMLAAG") == 0){                
                helderheid_omlaag();
            }
            else if(strcmp(received_v.kleurnaam, "FADING") == 0){                
                rgb_fading();
            }
            for (int i = 0; i < 30; i++) {
                if (strcmp(received_v.kleurnaam, kleurenlijst_rgb[i].kleurnaam) == 0) {
                    huidige_kleur_index = i;
                    huidige_kleur = kleurenlijst_rgb[i];
                    displayColor.r = kleurenlijst_rgb[i].r;
                    displayColor.g = kleurenlijst_rgb[i].g;
                    displayColor.b = kleurenlijst_rgb[i].b;
                    all_same(&displayColor);
                }
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

