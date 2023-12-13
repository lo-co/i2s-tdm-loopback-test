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

#ifndef SERDES_MEMORY_H
#define SERDES_MEMORY_H
#include <stdint.h>
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

typedef enum memory_status_e {
    MEMORY_STATUS_SUCCESS = 0,
    MEMORY_STATUS_EMPTY,
    MEMORY_STATUS_FULL,
    MEMORY_STATUS_END
} memory_status_t;

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
 * This function is strictly for sending data to the paired device.  There is no
 * corresponding retrieval of data - this is done when the bridge is active.
 * The buffer will be drained as the device transmits
 *
 * @param data Data to be inserted into the buffer.
 * @return MEMORY_STATUS_SUCCESS if space is available
 * @return MEMORY_STATUS_FULL if space is not available
 */
memory_status_t serdes_mem_insert_data_data(uint64_t data);

/**
 * @brief Return the current read buffer for transmit.
 *
 * @return A pointer to the current buffer position.
 */
uint8_t* serdes_get_next_tx_buffer();

/**
 * @brief Get the next buffer to be read
 *
 * @return Pointer to buffer for next read
 */
uint8_t* serdes_get_next_rx_buffer();

/**
 * @brief Get the next audio receive buffer
 *
 * @param buffer Pointer provided for storing buffer address
 *
 * @return MEMORY_STATUS_EMPTY No memory to be read
 * @return MEMORY_STATUS_SUCCESS Buffer successfully retrieved
 */
memory_status_t serdes_mem_get_next_read_buffer(uint8_t **buffer);

/**
 * @brief Retrieve the next buffer position for writing audio data to
 */
uint8_t* serdes_get_next_audio_src_buffer();

/**
 * @brief Clear buffers and reset write and read positions
 */
void serdes_memory_init(bool is_master);

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

#endif
