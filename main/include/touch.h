#pragma once

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/touch_sensor.h"
#include "driver/touch_pad.h"
#include "include/queue.h"


void config_touch();

void touch_task(void *pvParameters);