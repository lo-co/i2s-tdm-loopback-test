/** @file serdes_codec.h
 *
 * Defines interface functions for using the Cirrus 42488 CODEC found
 * on the MIMXRT685-AUD-EVK.
 *
 * @author Matt Richardson
 * @date October 27 2023
*/
#ifndef _SERDES_CODEC_H
#define _SERDES_CODEC_H

/** Initialize the CODEC
 *
 * The CODEC here is the Cirrus Logic 42448 provided with the
 * audio EVK. This will set initialize the the CODEC via I2C and
 * set the volume to max on channels 0 and 1
*/
void serdes_codec_init();

/**
 * @brief Start the I2S bus to receive data from the codec.
 *
 */
void serdes_codec_src_start();

/**
 * @brief Stop I2S receive bus
 *
 */
void serdes_codec_src_stop();

#endif