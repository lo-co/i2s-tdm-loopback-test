/** @file serdes_utilities.h
 *
 * Define useful functions for SERDES testing.
*/
#ifndef _SERDES_UTILITIES_H
#define _SERDES_UTILITIES_H

#include "fsl_common.h"

/** Generate a tone
 *
 * Generate a sine tone with one full cycle running through the tone
 * length.  Determine the frequency by dividing the sampling frequency
 * in Hz by the length of the tone provided below.
 *
 * @param tone 32-bit integer storage for the tone.
 * @param tone_length Number of points that define a whole cycle.
 * @param max_volume Maximum volume - 32-bit max is maximum volume.
*/
void generate_tone(int32_t *tone, uint32_t tone_length, int32_t max_volume);

#endif
