#include <stdint.h>

#pragma once


typedef struct{
    char kleurnaam[20];
}queue_msg_t;


// Externe declaratie van de queue, zodat andere bestanden er toegang toe hebben
extern QueueHandle_t main_queue;