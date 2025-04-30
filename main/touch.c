#include "include/touch.h"
#include <string.h>




void config_touch(){

    // Init touch sensor
    touch_pad_init();
    //touch_pad_config(TOUCH_PAD_NUM4, 0); // Standaard waarde ->lolin
    touch_pad_config(TOUCH_PAD_NUM5); // Standaard waarde ->s3

}

void touch_task(void *pvParameters){
    config_touch();
    
    while (1) {
        uint16_t touch_value;
        //touch_pad_read(TOUCH_PAD_NUM4, &touch_value); //lolin
        touch_pad_read_raw_data(TOUCH_PAD_NUM5, &touch_value); // s3

        printf("Touch waarde: %d\n", touch_value);

        if (touch_value < 500) { // Aangepaste drempelwaarde
            queue_msg_t msg;
            strcpy(msg.kleurnaam, "next_color");
            xQueueSend(main_queue, &msg, 1000);

        } else {
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}