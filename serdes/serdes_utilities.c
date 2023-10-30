#include "serdes_utilities.h"
#include "serdes_defs.h"
#include "math.h"

void generate_tone(int32_t *tone, uint32_t tone_length, int32_t max_volume)
{

	for (uint8_t i = 0; i < tone_length; i++)
	{
		tone[i] = (int32_t)(max_volume * sin(2*M_PI /tone_length*i));
	}
}

