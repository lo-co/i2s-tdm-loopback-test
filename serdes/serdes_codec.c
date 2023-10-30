#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "fsl_cs42448.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

#define CODEC_I2C_BASEADDR         I2C2
#define CODEC_I2C_INSTANCE         2U

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

cs42448_config_t cs42448Config = {
    .DACMode      = kCS42448_ModeSlave,
    .ADCMode      = kCS42448_ModeSlave,
    .reset        = NULL,
    .master       = false,
    .i2cConfig    = {.codecI2CInstance = CODEC_I2C_INSTANCE},
    .format       = {.sampleRate = 48000U, .bitWidth = 24U},
    .bus          = kCS42448_BusTDM,
    .slaveAddress = CS42448_I2C_ADDR,
};

codec_config_t boardCodecConfig = {.codecDevType = kCODEC_CS42448, .codecDevConfig = &cs42448Config};
codec_handle_t codecHandle;

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

void serdes_codec_init()
{
    cs42448Config.i2cConfig.codecI2CSourceClock = CLOCK_GetFlexCommClkFreq(2);
    cs42448Config.format.mclk_HZ                = CLOCK_GetMclkClkFreq();

    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        PRINTF("CODEC_Init failed!\r\n");
        assert(false);
    }

    // Set the volume to the max on channels 0 and 1
    CODEC_SetVolume(&codecHandle, 0, 100);
    CODEC_SetVolume(&codecHandle, 1, 100);

    PRINTF("\r\nCodec Init Done.\r\n");
}
