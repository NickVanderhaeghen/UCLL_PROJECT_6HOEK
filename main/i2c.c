#include "include/i2c.h"

extern uint8_t buf[DATA_LENGTH];
static IRAM_ATTR bool i2c_slave_rx_done_callback(i2c_slave_dev_handle_t channel, const i2c_slave_rx_done_event_data_t *edata, void *user_data)
    { //make a call back function taht you put into queue to check if all data from master was received
        BaseType_t high_task_wakeup = pdFALSE;
        QueueHandle_t receive_queue = (QueueHandle_t)user_data;
        xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);
        return high_task_wakeup == pdTRUE;
    }

QueueHandle_t s_receive_queue;

i2c_slave_dev_handle_t slave_handle;
i2c_master_bus_handle_t bus_handle;
i2c_master_dev_handle_t dev_handle;

void slave_task(void *pvParameters)
{
    

    i2c_slave_config_t i2c_slv_config = { //config slave
        .addr_bit_len = I2C_ADDR_BIT_LEN_7,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = 0, 
        .send_buf_depth = DATA_LENGTH,
        .sda_io_num = SDA,
        .slave_addr = Adress,                  
        .scl_io_num = SCL,
    };
    uint8_t *data_rd;
    

    i2c_slave_rx_done_event_data_t rx_data; //pointer for storing queue data - if transmit was finished

    data_rd = (uint8_t *) malloc(DATA_LENGTH); //buffer for received data


    memset(data_rd, 0, 500);

    i2c_new_slave_device(&i2c_slv_config, &slave_handle); //adding slave device

    s_receive_queue = xQueueCreate(1, sizeof(i2c_slave_rx_done_event_data_t)); //create queue for sending data about finishing transmit
    i2c_slave_event_callbacks_t cbs = { //call back function that supposed to start on receive done
    .on_recv_done = i2c_slave_rx_done_callback,
    };
    i2c_slave_register_event_callbacks(slave_handle, &cbs, s_receive_queue); //register event

    for(;;){
        i2c_slave_receive(slave_handle, data_rd, DATA_LENGTH); //receive data
        for(int i = 0; i < DATA_LENGTH;i++){
        printf("received data %d \n\r", data_rd[i]);}
        xQueueReceive(s_receive_queue, &rx_data, pdMS_TO_TICKS(10000)); //receive data from queue about state of the transmit of data
        xQueueSend(main_queue,&data_rd,1000); //send the data into main queue for other tasks
        vTaskDelay(10);
    }
}

void master_task(void *pvParameters)
{
    const i2c_master_bus_config_t i2c_mst_config = { //master config
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = 0,
        .scl_io_num = SCL,
        .sda_io_num = SDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    
    
    
    i2c_device_config_t dev_cfg = { //config slave device
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = Adress,
        .scl_speed_hz = 100000,
    };
    

    i2c_new_master_bus(&i2c_mst_config, &bus_handle); 
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle)); // add slave device to master bus
    
    for (;;)
    {
        // Transmit data to the slave
        esp_err_t ret = i2c_master_transmit(dev_handle, main_queue, DATA_LENGTH, 100);
        if (ret == ESP_OK) { // print transmitted data in case of succesfull datatransmit
            printf("Data sent: ");
            for (int i = 0; i < DATA_LENGTH; i++) {
                printf("%02X ", buf[i]);
            }
            printf("\n");
        } else {
            printf("Error sending data\n");
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay to prevent flooding the bus
    }
    esp_err_t i2c_del_master_bus(i2c_master_bus_handle_t bus_handle);
}

void delete_slave_bus()
{
    if (slave_handle)
    {
        printf("Deleting I2C Slave Device...\n");
        i2c_del_slave_device(slave_handle); //delete slave device
        slave_handle = NULL;
    }

    

    printf("I2C Slave Bus deleted successfully!\n");
}

void delete_master_bus()
{
    if (dev_handle)
    {
        printf("Removing I2C Master Device...\n");
        i2c_master_bus_rm_device(dev_handle); // delete slave device from master bus
        dev_handle = NULL;
    }

    if (bus_handle)
    {
        printf("Deleting I2C Master Bus...\n");
        i2c_del_master_bus(bus_handle); // delete master bus itself
        bus_handle = NULL;
    }

    printf("I2C Master Bus deleted successfully!\n");
}