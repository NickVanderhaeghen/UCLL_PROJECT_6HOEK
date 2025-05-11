#pragma once

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/touch_sensor.h"
#include "driver/touch_pad.h"
#include "include/queue.h"
#include "driver/gptimer.h"
#include <string.h>


void config_touch();

void config_touch_timer();

void touch_timer();



void touch_task(void *pvParameters);