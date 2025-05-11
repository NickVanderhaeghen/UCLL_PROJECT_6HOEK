#include <esp_log.h>
#include <led_strip.h>
#include <string.h>
#include "include/kleuren.h"
#include "include/queue.h"



void config_led();
//config de ledstrip

void all_same(const RGBColor* color);
//zet alle leds op 1 kleur

void blink();

void rgb_fading();
//zet alle leds op 1 kleur en begind te faden(zachtjes van kleur veranderen)

void next_color();
//deze gaat de kleur naar een andere kleur veranderen

void set_color(const RGBColor* color, int index);
//deze functie wordt door de vorige functies gebruikt.
//zet een led op een bepaalde index op de meegegeven kleur
//KLEUREN STAAN IN kleuren.h


void queue_to_led_task(void *pvParameters);