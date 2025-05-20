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
#include "include/i2c.h"

typedef enum{
    FADING,
    ALL_SAME
} LED_MODE;


LED_MODE huidige_mode = FADING;

TaskHandle_t xt_hand;
TaskHandle_t xt_ir;
TaskHandle_t xt_touch;

TaskHandle_t xMasterHandle;
TaskHandle_t xSlaveHandle;

QueueHandle_t main_queue;



void app_main(void)
{
    int count=0;
    main_queue = xQueueCreate(500, sizeof(queue_msg_t));

    config_led();

    xTaskCreate(ir_task, "ir", 10000, NULL, 5, &xt_ir);
    xTaskCreate(touch_task, "touch", 2048, NULL, 5, &xt_touch);
    xTaskCreate(queue_to_led_task, "queue to led", 2048, NULL, 5, &xt_hand);

    extern uint8_t touch_value;
    if (touch_value > 1000){
    delete_slave_bus();
    xTaskCreate(master_task, "Master Task", MASTER_TASK_STACK_SIZE, NULL, 1, &xMasterHandle);
    configASSERT( xMasterHandle );
    printf("master task");
    }
    else{
    delete_master_bus();
    xTaskCreate(slave_task, "Slave Task", SLAVE_TASK_STACK_SIZE, NULL, 1, &xSlaveHandle);
    configASSERT( xSlaveHandle );
    printf("slave task");
    }
}