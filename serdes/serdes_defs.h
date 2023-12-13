/** @file serdes_defs.h
 *
 * File for common SERDES definitions.
 *
 * @author Matt Richardson
 * @date October 26, 2023
*/

#ifndef _SERDES_DEFS_H
#define _SERDES_DEFS_H

#define SAMPLE_RATE_HZ  (48000U)
#define SAMPLE_SIZE_MS  (8U)
#define MS_PER_S        (1000U)
#define SAMPLE_PER_MS   (SAMPLE_RATE_HZ / MS_PER_S)
#define BYTES_PER_SAMPLE (4U)
#define NUM_CHANNELS (8)
#define BUFFER_SIZE   (SAMPLE_SIZE_MS * SAMPLE_PER_MS * BYTES_PER_SAMPLE * NUM_CHANNELS)

#define NUM_MASTER_CH (4U)
#define MASTER_BUFFER_SIZE (SAMPLE_SIZE_MS * SAMPLE_PER_MS * BYTES_PER_SAMPLE * NUM_MASTER_CH)

#define NUM_SLAVE_CH (2U)
#define SLAVE_BUFFER_SIZE (SAMPLE_SIZE_MS * SAMPLE_PER_MS * BYTES_PER_SAMPLE * NUM_SLAVE_CH)

#define NUM_SRC_CH (2U)
#define AUDIO_SRC_BUFFER_SIZE (SAMPLE_SIZE_MS * SAMPLE_PER_MS * BYTES_PER_SAMPLE * NUM_SRC_CH)

#define BUFFER_NUMBER (8U)
// Data will start at channel 6 (0 being the first channel and 7 being the last)
#define DATA_CHANNEL_START (3U)
#define CHANNEL_PAIR (2U)

// Each data entry is 64-bits or two channels.  Communication that extends beyond this
// will bleed into other channels
#define DATA_BUFFER_SIZE    (8U)
#define NUMBER_DATA_BUFFERS (10U)

#define NUMBER_CODEC_BUFFERS (2U)

/* Constants for data being pushed around */
#define CONST_DATA (0xdeadbeef)
#define M_PI 3.14159265358979323846

#endif