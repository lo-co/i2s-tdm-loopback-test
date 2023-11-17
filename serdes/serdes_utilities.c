#include "serdes_utilities.h"
#include "serdes_defs.h"
#include "math.h"
#define MAX_VOL (2147483647U)

void generate_tone(int32_t *tone, uint32_t tone_length, uint8_t vol_per)
{
	int32_t volume = ( vol_per * MAX_VOL )/ 100;
	for (uint8_t i = 0; i < tone_length; i++)
	{
		tone[i] = volume*(int32_t)(sin(2*M_PI /tone_length*i));
	}
}

