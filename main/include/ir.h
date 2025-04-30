#include "freertos/FreeRTOS.h"
#include <string.h>
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"

#include "include/ir_nec_encoder.h"
#include "include/queue.h"

#pragma once

/**
 * @brief NEC timing spec
 */
#define NEC_LEADING_CODE_DURATION_0  9000
#define NEC_LEADING_CODE_DURATION_1  4500
#define NEC_PAYLOAD_ZERO_DURATION_0  560
#define NEC_PAYLOAD_ZERO_DURATION_1  560
#define NEC_PAYLOAD_ONE_DURATION_0   560
#define NEC_PAYLOAD_ONE_DURATION_1   1690
#define NEC_REPEAT_CODE_DURATION_0   9000
#define NEC_REPEAT_CODE_DURATION_1   2250






void config_ir();


/**
 * @brief Check whether a duration is within expected range -> hier checken we dus de opgevangen tijden op een logische 0 of 1
 */
static inline bool nec_check_in_range(uint32_t signal_duration, uint32_t spec_duration);


/**
 * @brief Check whether a RMT symbol represents NEC logic zero -> volgens het protocol is het een logische 0
 * als er voor een totale periode van 1125us de eerste 560us hoog zijn en de overige 560us
 */
static bool nec_parse_logic0(rmt_symbol_word_t *rmt_nec_symbols);


/**
 * @brief Check whether a RMT symbol represents NEC logic one -> volgens het protocol is het een logische 1
 * als er voor een totale periode van 2250us de eerste 560us hoog zijn en de overige 1690us
 */
static bool nec_parse_logic1(rmt_symbol_word_t *rmt_nec_symbols);


/**
 * @brief Decode RMT symbols into NEC address and command
 */
static bool nec_parse_frame(rmt_symbol_word_t *rmt_nec_symbols);



/**
 * @brief Check whether the RMT symbols represent NEC repeat code
 */
static bool nec_parse_frame_repeat(rmt_symbol_word_t *rmt_nec_symbols);



/**
 * @brief Decode RMT symbols into NEC scan code and print the result
 */
static void print_nec_frame(rmt_symbol_word_t *rmt_nec_symbols, size_t symbol_num);




/**
 * als de code die we aankrijgen uniek is wordt deze hier omgezet naar een kleur en in de main_queue gezet
 */
bool afhandeling_code_naar_kleur();




static bool example_rmt_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data);


void ir_task(void *pvParameters);

/*
extern const int16_t ir_RED;
extern const int16_t ir_LIGHT_RED;
extern const int16_t ir_GREEN;
extern const int16_t ir_LIGHT_GREEN;
extern const int16_t ir_BLUE;
extern const int16_t ir_LIGHT_BLUE;
extern const int16_t ir_BABY_BLUE;
extern const int16_t ir_BABY_BLUE2;
extern const int16_t ir_WHITE;
extern const int16_t ir_PINK;
extern const int16_t ir_PINK2;
extern const int16_t ir_TURQUOISE;
extern const int16_t ir_EMERALD;
extern const int16_t ir_DARK_GREEN;
extern const int16_t ir_DARK_PURPLE;
extern const int16_t ir_LIGHT_PURPLE;
extern const int16_t ir_FUCHSIA;
extern const int16_t ir_YELLOW;
extern const int16_t ir_ORANGE;
extern const int16_t ir_LIGHT_ORANGE;
*/