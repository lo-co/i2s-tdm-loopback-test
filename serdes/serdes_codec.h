#ifndef _SERDES_CODEC_H
#define _SERDES_CODEC_H

/** Initialize the CODEC
 *
 * The CODEC here is the Cirrus Logic 42448 provided with the
 * audio EVK. This will set initialize the the CODEC via I2C and
 * set the volume to max on channels 0 and 1
*/
void serdes_codec_init();

#endif