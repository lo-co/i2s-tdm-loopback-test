/** @file serdes_defs.h
 *
 * File for common SERDES definitions.
 *
 * @author Matt Richardson
 * @date October 26, 2023
*/

#ifndef _SERDES_DEFS_H
#define _SERDES_DEFS_H

#define SAMPLE_RATE_HZ (48000)
#define SAMPLE_SIZE_MS (8U)
#define SAMPLE_PER_MS (48)
#define BYTES_PER_SAMPLE (4)
#define NUM_CHANNELS (8)
#define BUFFER_SIZE   (SAMPLE_SIZE_MS * SAMPLE_PER_MS * BYTES_PER_SAMPLE * NUM_CHANNELS)
#define BUFFER_NUMBER (8U)

/* Constants for data being pushed around */
#define CONST_DATA (0xdeadbeef)
#define M_PI 3.14159265358979323846

#endif