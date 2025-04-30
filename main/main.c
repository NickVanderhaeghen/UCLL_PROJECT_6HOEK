#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"
#include <driver/gpio.h>
#include <esp_system.h>

#include "include/queue.h"
#include "include/ir.h"
#include "include/ledsturing.h"
#include "include/touch.h"

typedef enum{
    FADING,
    ALL_SAME
} LED_MODE;


LED_MODE huidige_mode = FADING;

TaskHandle_t xt_hand;
TaskHandle_t xt_ir;
TaskHandle_t xt_touch;

QueueHandle_t main_queue;


void app_main(void)
{
    main_queue = xQueueCreate(500, sizeof(queue_msg_t));

    config_led();

    xTaskCreate(ir_task, "ir", 10000, NULL, 5, &xt_ir);
    xTaskCreate(touch_task, "touch", 2048, NULL, 5, &xt_touch);
    xTaskCreate(queue_to_led_task, "queue to led", 2048, NULL, 5, &xt_hand);


}