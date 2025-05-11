#include "include/touch.h"


gptimer_handle_t touch_timer_handle;


void config_touch(){

    // Init touch sensor
    touch_pad_init();
    //touch_pad_config(CONFIG_TOUCH_GPIO, 0); // Standaard waarde ->lolin
    touch_pad_config(TOUCH_PAD_NUM5); // Standaard waarde ->s3
}

void config_touch_timer(){
    gptimer_config_t touch_timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000 //1Mhz -> 1 tick = 1us -> 3s = 3.000.000
    };
    gptimer_new_timer(&touch_timer_config, &touch_timer_handle);
}


void touch_timer(){
    
}

void touch_task(void *pvParameters){
    config_touch();
    
    while (1) {
        uint16_t touch_value;
        //touch_pad_read(TOUCH_PAD_NUM4, &touch_value); //lolin
        touch_pad_read_raw_data(TOUCH_PAD_NUM5, &touch_value); // s3

        printf("Touch waarde: %d\n", touch_value);

        if (touch_value < CONFIG_TOUCH_THRESHOLD) { // Aangepaste drempelwaarde
            queue_msg_t msg;
            strcpy(msg.kleurnaam, "next_color");
            xQueueSend(main_queue, &msg, 1000);

        } else {
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}