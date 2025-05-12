#include "include/touch.h"


gptimer_handle_t touch_timer_handle;

int touch_gpio = 12;

void config_touch(){
    gpio_reset_pin(CONFIG_TOUCH_GPIO);
    touch_pad_init();
    touch_pad_config(CONFIG_TOUCH_GPIO); // Standaard waarde ->s3
    touch_pad_set_thresh(CONFIG_TOUCH_GPIO, CONFIG_TOUCH_THRESHOLD);
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
    touch_pad_denoise_t denoise = {
        /* The bits to be cancelled are determined according to the noise level. */
        .grade = TOUCH_PAD_DENOISE_BIT4,
        .cap_level = TOUCH_PAD_DENOISE_CAP_L4,
    };
    touch_pad_denoise_set_config(&denoise);
    touch_pad_denoise_enable();
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_fsm_start();
    while (1) {
        uint32_t touch_value;
        touch_pad_read_raw_data(CONFIG_TOUCH_GPIO, &touch_value); // s3

        printf("Touch waarde: %ld\n", touch_value);

        if (touch_value > CONFIG_TOUCH_THRESHOLD) { // Aangepaste drempelwaarde
            queue_msg_t msg;
            strcpy(msg.kleurnaam, "next_color");
            xQueueSend(main_queue, &msg, 1000);

        } 
        else {

        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}