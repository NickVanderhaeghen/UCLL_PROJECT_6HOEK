#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "driver/i2c_master.h"
#include "driver/i2c_slave.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include "include/queue.h"

#define DATA_LENGTH 10000
#define SCL 4
#define SDA 5
#define Adress 0x3A

#define MASTER_TASK_STACK_SIZE 4096
#define SLAVE_TASK_STACK_SIZE 4096


void slave_task(void *pvParameters);
void master_task(void *pvParameters);
void delete_slave_bus();
void delete_master_bus();