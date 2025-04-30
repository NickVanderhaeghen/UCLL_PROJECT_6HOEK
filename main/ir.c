#include "include/ir.h"




typedef struct {
    char naam[20];
    uint16_t waarde;
} Kleur;


// Definieer de lijst met kleuren
const Kleur kleurenlijst[] = {
    {"ROOD", 0xA758},
    {"LICHTROOD", 0xAB54},
    {"GROEN", 0xA659},
    {"LICHTGROEN", 0xAA55},
    {"BLAUW", 0xBA45},
    {"LICHTBLAUW", 0xB649},
    {"BABYBLAUW", 0xE01F},
    {"BABYBLAUW2", 0xE41B},
    {"WIT", 0xBB44},
    {"ROZE", 0xB748},
    {"ROZE2", 0xB34C},
    {"TURQUOISE", 0xAE51},
    {"SMARAGD", 0xE21D},
    {"DONKERGROEN", 0xE619},
    {"DONKERPAARS", 0xB24D},
    {"LICHTPAARS", 0xE11E},
    {"FUCHSIA", 0xE51A},
    {"GEEL", 0xE718},
    {"ORANJE", 0xAF50},
    {"LICHTORANJE", 0xE31C},
    {"ir_RED_UP", 0xEB14},
    {"ir_RED_DOWN", 0xEF10},
    {"ir_GREEN_UP", 0xEA15},
    {"ir_GREEN_DOWN", 0xEE11},
    {"ir_BLUE_UP", 0xE916},
    {"ir_BLUE_DOWN", 0xED12}
};




QueueHandle_t receive_queue;
rmt_channel_handle_t rx_channel = NULL;
rmt_receive_config_t receive_config = {
    .signal_range_min_ns = 1250,     // the shortest duration for NEC signal is 560us, 1250ns < 560us, valid signal won't be treated as noise
    .signal_range_max_ns = 12000000, // the longest duration for NEC signal is 9000us, 12000000ns > 9000us, the receive won't stop early
};
// save the received RMT symbols
rmt_symbol_word_t raw_symbols[64]; // 64 symbols should be sufficient for a standard NEC frame
rmt_rx_done_event_data_t rx_data;







static const char *TAG = "example";

/**
 * @brief Saving NEC decode results
 */
static uint16_t s_nec_code_address;
static uint16_t s_nec_code_command;


void config_ir(){
    rmt_rx_channel_config_t rx_channel_cfg = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = CONFIG_RESOLUTION_IR,
        .mem_block_symbols = 64, // amount of RMT symbols that the channel can store at a time
        .gpio_num = CONFIG_GPIO_IR,
    };
    
    ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_channel_cfg, &rx_channel));

    ESP_LOGI(TAG, "create RMT RX channel");
    
    ESP_LOGI(TAG, "register RX done callback");
    receive_queue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
    assert(receive_queue);
    rmt_rx_event_callbacks_t cbs = {
        .on_recv_done = example_rmt_rx_done_callback,
    };
    ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rx_channel, &cbs, receive_queue));


    // the following timing requirement is based on NEC protocol


    ESP_LOGI(TAG, "install IR NEC encoder");
    ir_nec_encoder_config_t nec_encoder_cfg = {
        .resolution = CONFIG_RESOLUTION_IR,
    };
    rmt_encoder_handle_t nec_encoder = NULL;
    ESP_ERROR_CHECK(rmt_new_ir_nec_encoder(&nec_encoder_cfg, &nec_encoder));

    ESP_LOGI(TAG, "enable RMT TX and RX channels");
    ESP_ERROR_CHECK(rmt_enable(rx_channel));


    // ready to receive
    ESP_ERROR_CHECK(rmt_receive(rx_channel, raw_symbols, sizeof(raw_symbols), &receive_config));
}


/**
 * @brief Check whether a duration is within expected range -> hier checken we dus de opgevangen tijden op een logische 0 of 1
 */
static inline bool nec_check_in_range(uint32_t signal_duration, uint32_t spec_duration)
 {
     return (signal_duration < (spec_duration + CONFIG_NEC_DECODE_MARGIN)) &&
            (signal_duration > (spec_duration - CONFIG_NEC_DECODE_MARGIN));
 }

/**
 * @brief Check whether a RMT symbol represents NEC logic zero -> volgens het protocol is het een logische 0
 * als er voor een totale periode van 1125us de eerste 560us hoog zijn en de overige 560us
 */
static bool nec_parse_logic0(rmt_symbol_word_t *rmt_nec_symbols)
 {
     return nec_check_in_range(rmt_nec_symbols->duration0, NEC_PAYLOAD_ZERO_DURATION_0) &&
            nec_check_in_range(rmt_nec_symbols->duration1, NEC_PAYLOAD_ZERO_DURATION_1);
 }

/**
 * @brief Check whether a RMT symbol represents NEC logic one -> volgens het protocol is het een logische 1
 * als er voor een totale periode van 2250us de eerste 560us hoog zijn en de overige 1690us
 */
static bool nec_parse_logic1(rmt_symbol_word_t *rmt_nec_symbols)
 {
     return nec_check_in_range(rmt_nec_symbols->duration0, NEC_PAYLOAD_ONE_DURATION_0) &&
            nec_check_in_range(rmt_nec_symbols->duration1, NEC_PAYLOAD_ONE_DURATION_1);
 }

/**
 * @brief Decode RMT symbols into NEC address and command
 */
static bool nec_parse_frame(rmt_symbol_word_t *rmt_nec_symbols)
 {
     rmt_symbol_word_t *cur = rmt_nec_symbols;
     uint16_t address = 0;
     uint16_t command = 0;
     bool valid_leading_code = nec_check_in_range(cur->duration0, NEC_LEADING_CODE_DURATION_0) &&
                               nec_check_in_range(cur->duration1, NEC_LEADING_CODE_DURATION_1);
     if (!valid_leading_code) {
         return false;
     }
     cur++;
     for (int i = 0; i < 16; i++) {
         if (nec_parse_logic1(cur)) {
             address |= 1 << i;
         } else if (nec_parse_logic0(cur)) {
             address &= ~(1 << i);
         } else {
             return false;
         }
         cur++;
     }
     for (int i = 0; i < 16; i++) {
         if (nec_parse_logic1(cur)) {
             command |= 1 << i;
         } else if (nec_parse_logic0(cur)) {
             command &= ~(1 << i);
         } else {
             return false;
         }
         cur++;
     }
     // save address and command
     s_nec_code_address = address;
     s_nec_code_command = command;
     return true;
 }

/**
 * @brief Check whether the RMT symbols represent NEC repeat code
 */
static bool nec_parse_frame_repeat(rmt_symbol_word_t *rmt_nec_symbols)
 {
     return nec_check_in_range(rmt_nec_symbols->duration0, NEC_REPEAT_CODE_DURATION_0) &&
            nec_check_in_range(rmt_nec_symbols->duration1, NEC_REPEAT_CODE_DURATION_1);
 }



/**
 * @brief Decode RMT symbols into NEC scan code and print the result -> hier wordt ook een onderscheid gemaakt of iets al dan niet
 * een herhaling is. We kunnen hier dus zeggen als iets nieuw is en dus geen herhaling van vb een knop die je blijft indrukken,
 * zet dit dan in de main_queue. Adhv een lut kunnn we in ledsturing dan een kleur aan de code toewijzen.
 */
static void print_nec_frame(rmt_symbol_word_t *rmt_nec_symbols, size_t symbol_num)
{

    printf("---NEC frame end: ");
    // decode RMT symbols
    switch (symbol_num) {
    case 34: // NEC normal frame
        if (nec_parse_frame(rmt_nec_symbols)) {
            printf("Address=%04X, Command=%04X\r\n\r\n", s_nec_code_address, s_nec_code_command);
            bool test = afhandeling_code_naar_kleur();
            if (test){printf("kleur is gevonden in de lijst en de kleurnaam zit in de queue\n\r");}
            else {printf("kleur is niet gevonden in de lijst en onbekend zit in de queue\n\r");}
        }
        break;
    case 2: // NEC repeat frame
        if (nec_parse_frame_repeat(rmt_nec_symbols)) {
            printf("Address=%04X, Command=%04X, repeat\r\n\r\n", s_nec_code_address, s_nec_code_command);
        }
        break;
    default:
        printf("Unknown NEC frame\r\n\r\n");
        break;
    }
}



static bool example_rmt_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data)
{
    BaseType_t high_task_wakeup = pdFALSE;
    QueueHandle_t receive_queue = (QueueHandle_t)user_data;
    // send the received RMT symbols to the parser task
    xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);
    return high_task_wakeup == pdTRUE;
}


/**
 * als de code die we aankrijgen uniek is wordt deze hier omgezet naar een kleur en in de main_queue gezet
 */
bool afhandeling_code_naar_kleur(){
    Kleur kleurtest;
    for (int i = 0; i < sizeof(kleurenlijst); i++) {
        if (kleurenlijst[i].waarde == s_nec_code_command) {
            printf("GEVONDEN! kleur is %s\n\r", kleurenlijst[i].naam);
            queue_msg_t msg;
            strcpy(msg.kleurnaam, kleurenlijst[i].naam);
            xQueueSend(main_queue, &msg, 1000);
            return true;
        }
    }
    xQueueSend(main_queue, "Onbekend",10000);
    return false; // Als de kleur niet wordt gevonden    
}





void ir_task(void *pvParameters)
{
    config_ir();
    
    while (1) {
        // wait for RX done signal
        if (xQueueReceive(receive_queue, &rx_data, pdMS_TO_TICKS(1000)) == pdPASS) {
            // parse the receive symbols and print the result
            print_nec_frame(rx_data.received_symbols, rx_data.num_symbols);
            // start receive again
            ESP_ERROR_CHECK(rmt_receive(rx_channel, raw_symbols, sizeof(raw_symbols), &receive_config));
        } else {
            // timeout, transmit predefined IR NEC packets
            const ir_nec_scan_code_t scan_code = {
                .address = 0x0440,
                .command = 0x3003,
            };
        }
    }
}




/*
const int16_t ir_RED = 0xA758;
const int16_t ir_LIGHT_RED = 0xAB54;
const int16_t ir_GREEN = 0xA659;
const int16_t ir_LIGHT_GREEN = 0xAA55;
const int16_t ir_BLUE = 0xBA45;
const int16_t ir_LIGHT_BLUE = 0xB649;
const int16_t ir_BABY_BLUE = 0xE01F;
const int16_t ir_BABY_BLUE2 = 0xE41B;
const int16_t ir_WHITE = 0xBB44;
const int16_t ir_PINK = 0xB748;
const int16_t ir_PINK2 = 0xB34C;
const int16_t ir_TURQUOISE = 0xAE51;
const int16_t ir_EMERALD = 0xE21D;
const int16_t ir_DARK_GREEN = 0xE619;
const int16_t ir_DARK_PURPLE = 0xB24D;
const int16_t ir_LIGHT_PURPLE = 0xE11E;
const int16_t ir_FUCHSIA = 0xE51A;
const int16_t ir_YELLOW = 0xE718;
const int16_t ir_ORANGE = 0xAF50;
const int16_t ir_LIGHT_ORANGE = 0xE31C;

const int16_t ir_RED_UP = 0xEB14;
const int16_t ir_RED_DOWN = 0xEF10;
const int16_t ir_GREEN_UP = 0xEA15;
const int16_t ir_GREEN_DOWN = 0xEE11;
const int16_t ir_BLUE_UP = 0xE916;
const int16_t ir_BLUE_DOWN = 0xED12;
*/