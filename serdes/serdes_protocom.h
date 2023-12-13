/**
 * @file serdes_protocom.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-11-16
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _SERDES_PROTOCOM_H
#define _SERDES_PROTOCOM_H
#include <stdint.h>
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SRC_SIZE_BITS       4
#define LENGTH_SIZE_BITS    8
#define DATA_SIZE_BITS      40

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * @brief Status for protocom operations
 */
typedef enum protocom_status_e
{
    PROTOCOM_SUCCESS = 0,
    PROTOCOM_BUFFER_FULL,
    PROTOCOM_BAD_START_BYTE,
    PROTOCOM_BAD_CRC,
    PROTOCOM_STATUS_END
} protocom_status_t;

/**
 * @brief Packet target definitions
 *
 * The SRC_NUM should not exceeed 0xF
 */
typedef enum data_src_e {
    SRC_GPIO = 0x0,
    SRC_I2C = 0x1,
    SRC_SPI = 0x2,
    SRC_UART = 0x3,
    SRC_NUM = 0x4
} data_src_t;

/**
 * @brief Structure of a data packet to be passed across the bridge
 */
typedef struct data_pckt_s {
    data_src_t src: SRC_SIZE_BITS; /**< Destination of the data */
    uint64_t data: DATA_SIZE_BITS;  /**< Pointer to hold data */
    uint32_t data_length: LENGTH_SIZE_BITS; /**< Length of data in bytes */
} data_pckt_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief Return serialized data of current data queue element
 *
 * @return 0 if no data is available otherwise serialized data if available
 */
uint64_t serdes_protocom_serialize_data(data_pckt_t data_pckt);

/**
 * @brief Deserialize a stream of bytes into a data packet.
 *
 * Packet to be deserialized will be of the format
 *
 * |       4 bits      | 4 bits |       1 byte       |  40 bits |
 * | high - start byte | Source | length of data (n) |   data   |
 *
 * @param data 64-bit frame to be deserialized
 * @param data_pckt Pointer to container for deserialized data
 * @return PROTOCOM_SUCCESS on success
 * @return PROTOCOM_BAD_START_BYTE start byte does not match expected start byte
 * @return PROTOCOM_BAD_CRC Generated CRC does not match delivered CRC
 */
protocom_status_t serdes_protocom_deserialize_data(uint64_t data, data_pckt_t *data_pckt);

#endif