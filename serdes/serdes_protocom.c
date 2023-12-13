/**
 * @file serdes_protocom.c
 * @author Matt Richardson (mattrichardson@meta.com)
 * @brief Implementation of communication protocol routines for serialization/deserialization
 * @version 0.1
 * @date 2023-11-16
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <string.h>
#include "serdes_protocom.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** Starting bit for the CRC in the data packet buffer*/
#define CRC_BIT_POSITION 56

/** Each data packet transmitted will start with this 4-bit value */
#define START_PACKET     0xD

/** The size in bits of the start packet indicator above */
#define START_PACKET_SIZE_BITS 4

/**
 * @brief Macro to extract different bits of value based on the number of bits to shift
 *
 */
#define MASK_BITS(value, numBits) ((value) & ((1ULL << (numBits)) - 1))

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief Generate an 8 bit CRC over the 64 bit data
 *
 * @param buffer Data packet over which to run the CRC over
 * @return 8-bit CRC
 */
uint8_t get_crc8(uint64_t buffer);

/**
 * @brief Check CRC generated on slave device
 *
 * @param buffer 64-bit buffer containing data packet
 * @return true CRC generated from buffer matches CRC in buffer
 * @return false CRC generated from buffer does not match CRC in buffer
 */
bool check_crc(uint64_t buffer);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

// Documented in .h
uint64_t serdes_protocom_serialize_data(data_pckt_t data_pckt)
{
    uint8_t start_bit_pos = START_PACKET_SIZE_BITS;

    // Because noise impacts the transmission, I want to change this so that
    // capturing packets will be easier in that noisey environment.
    uint64_t buffer = START_PACKET;

    // Next the source
    buffer |= data_pckt.src << start_bit_pos;
    start_bit_pos += SRC_SIZE_BITS;
    buffer |= data_pckt.data_length << start_bit_pos;
    start_bit_pos += LENGTH_SIZE_BITS;

    buffer |= (uint64_t)(data_pckt.data) << start_bit_pos;
    start_bit_pos += DATA_SIZE_BITS;

    uint64_t crc = (uint64_t)get_crc8(buffer);

    buffer |= crc << start_bit_pos;

    return buffer;
}

// Documented in .h
protocom_status_t serdes_protocom_deserialize_data(uint64_t data, data_pckt_t *data_pckt)
{
    if (MASK_BITS(data, START_PACKET_SIZE_BITS) != START_PACKET)
    {
        return PROTOCOM_BAD_START_BYTE;
    }

    if (!check_crc(data))
    {
        return PROTOCOM_BAD_CRC;
    }

    uint8_t pckt_position = START_PACKET_SIZE_BITS;

    data_pckt->src = MASK_BITS((data >> pckt_position), SRC_SIZE_BITS);
    pckt_position += SRC_SIZE_BITS;
    data_pckt->data_length = MASK_BITS((data >> pckt_position), LENGTH_SIZE_BITS);
    pckt_position += LENGTH_SIZE_BITS;

    data_pckt->data = MASK_BITS((data >> pckt_position), DATA_SIZE_BITS);

    return PROTOCOM_SUCCESS;
}

// Documented above
uint8_t get_crc8(uint64_t buffer)
{
    uint8_t crc = 0x00;

    for (size_t i = 0; i < sizeof(uint64_t); ++i) {
        crc ^= (buffer >> (8 * i)) & 0xFF;

        for (int j = 0; j < 8; ++j) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

// Documented above
bool check_crc(uint64_t buffer)
{
    uint8_t extracted_crc = buffer >> CRC_BIT_POSITION;

    uint64_t extracted_data = buffer & ~(0xFFULL << CRC_BIT_POSITION);

    uint8_t calculated_crc = get_crc8(extracted_data);

    return (calculated_crc == extracted_crc);
}
