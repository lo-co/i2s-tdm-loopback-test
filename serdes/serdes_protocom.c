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
/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/
/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

// Documented in .h
uint64_t serdes_protocom_serialize_data(data_pckt_t data_pckt)
{
    uint8_t buffer[8] = {0};
    uint8_t *buffer_ptr = buffer;

    // Insert the start byte
    *(buffer_ptr++) = 0x1;

    // Next the source
    *(buffer_ptr++) = (uint8_t)data_pckt.src;

    // Next the length
    *(uint32_t *)buffer_ptr = data_pckt.data_length;
    buffer_ptr += 4;

    // Finally the data
    for (uint8_t i = 0; i < data_pckt.data_length; i++)
    {
        *buffer_ptr++ = *data_pckt.data++;
    }

    uint64_t serialized_data = 0;
    memcpy(&serialized_data, buffer, sizeof(uint64_t));

    return serialized_data;
}

// Documented in .h
protocom_status_t serdes_protocom_deserialize_data(uint8_t *data, data_pckt_t *data_pckt)
{
    // Strip out the start byte
    data++;
    data_pckt->src = *data++;
    data_pckt->data_length = *(uint32_t *)data;

    data += 4;

    for (int i = 0; i < data_pckt->data_length; i++)
    {
        data_pckt->data[i] = *data++;
    }

    return PROTOCOM_SUCCESS;
}
