/**
 * @file serdes_memory.h
 * @author Matt Richardson (mattrichardson@meta.com)
 * @brief
 * @version 0.1
 * @date 2023-11-01
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <stdint.h>
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

typedef enum data_src_e {
    SRC_GPIO = 0x0,
    SRC_I2C = 0x1,
    SRC_SPI = 0x2,
    SRC_UART = 0x3,
    SRC_NUM = 0x4
} data_src_t;

typedef struct data_pckt_s {
    data_src_t src;
    uint8_t *data;
    uint32_t data_length;
} data_pckt_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief Insert audio data into the audio buffer.
 *
 * If there is data in the data buffer, then that data will be read out and all
 * of the data will be placed in that buffer.
 *
 * @param buffer Buffer of data to transfer into the audio buffer.  This assumes the
 *               same format for the two buffers.

 */
void serdes_mem_insert_audio_data(uint8_t *buffer, uint32_t length);

/**
 * @brief Insert data into the data buffer
 *
 * @param data Data to be inserted into the buffer.  If the length exceeds 8 bytes
 *             for the data then the data will occupy multiple slots.
 */
void serdes_mem_insert_data_data(data_pckt_t data);

/**
 * @brief Return the current read buffer for transmit.
 *
 * @return A pointer to the current buffer position.
 */
uint8_t* serdes_get_next_tx_buffer();

uint8_t* serdes_get_next_rx_buffer();

/**
 * @brief Retrieve the next buffer position for writing audio data to
 */
uint8_t* serdes_get_next_audio_src_buffer();

/**
 * @brief Clear buffers and reset write and read positions
 */
void serdes_memory_init();

/**
 * @brief Determine whether there is data available for transmit
 *
 * @return True if there is data in the data buffer available for transmit
 */
bool serdes_memory_data_ready();

/**
 * @brief
 *
 * @return true
 * @return false
 */
bool serdes_memory_more_audio_data();


