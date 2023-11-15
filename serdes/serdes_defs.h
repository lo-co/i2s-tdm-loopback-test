/** @file serdes_defs.h
 *
 * File for common SERDES definitions.
 *
 * @author Matt Richardson
 * @date October 26, 2023
*/

#ifndef _SERDES_DEFS_H
#define _SERDES_DEFS_H

#define SAMPLE_RATE_HZ  (16000U)
#define SAMPLE_SIZE_MS  (8U)
#define MS_PER_S        (1000U)
#define SAMPLE_PER_MS   (SAMPLE_RATE_HZ / MS_PER_S)
#define BYTES_PER_SAMPLE (4U)
#define NUM_CHANNELS (8)
#define BUFFER_SIZE   (SAMPLE_SIZE_MS * SAMPLE_PER_MS * BYTES_PER_SAMPLE * NUM_CHANNELS)
#define BUFFER_NUMBER (8U)

// Each data entry is 64-bits or two channels.  Communication that extends beyond this
// will bleed into other channels
#define DATA_BUFFER_SIZE    (8U)
#define NUMBER_DATA_BUFFERS (10U)

#define NUMBER_CODEC_BUFFERS (2U)

/* Constants for data being pushed around */
#define CONST_DATA (0xdeadbeef)
#define M_PI 3.14159265358979323846

#endif