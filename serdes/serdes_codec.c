#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "fsl_cs42448.h"
#include "fsl_iopctl.h"
#include "fsl_debug_console.h"
#include "../drivers/i2s/i2s.h"
#include "../drivers/gpio/gpio_def.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

#define CODEC_I2C_BASEADDR         I2C2
#define CODEC_I2C_INSTANCE         2U

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/** Configure pin outs for use with the codec.
 *
 * Pins requred are defined in the MIMXRT685-AUD_EVKUM.pdf. I2C uses FLEXCOMM2 and
 * MCLK is run out for the MCLK pin on the CODEC MCLK.
*/
static void serdes_codec_pin_init();

/** Configure I2S for communication with the CODEC.
 *
 * This is defined in MIMXRT685-AUD-EVKUM, Table 24.  FC1 is used for receive
 * on the CODEC while FC3 is used for transmit.
*/
static void serdes_codec_i2s_init();

void codec_rx_cb(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData);
void codec_tx_cb(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData);

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

static codec_config_t boardCodecConfig = {.codecDevType = kCODEC_CS42448, .codecDevConfig = &cs42448Config};
static codec_handle_t codecHandle;

// Receiver for audio in from the codec
static i2s_context_t rx_i2s_context;
static i2s_init_t rx_i2s_cfg = {.flexcomm_bus = FLEXCOMM_1, .is_transmit = false,
                        .is_master = true, .active_channels = 8, .sample_rate = 48000,
                        .datalength = 32, .callback = codec_rx_cb, .context = &rx_i2s_context,
                        .share_clk = false, .shared_clk_set = NO_SHARE};

// Transmitter for audio to codec
static i2s_context_t tx_i2s_context;
static i2s_init_t tx_i2s_cfg = {.flexcomm_bus = FLEXCOMM_3, .is_transmit = true,
                        .is_master = true, .active_channels = 8, .sample_rate = 48000,
                        .datalength = 32, .callback = codec_tx_cb, .context = &tx_i2s_context,
                        .share_clk = false, .shared_clk_set = NO_SHARE};

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

void serdes_codec_init()
{
    serdes_codec_pin_init();
    serdes_codec_i2s_init();

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

static void serdes_codec_pin_init()
{
    const uint32_t pin_cfg = (IOPCTL_PIO_FUNC1 |
                              /* Enable pull-up / pull-down function */
                              IOPCTL_PIO_PUPD_EN |
                              /* Enable pull-up function */
                              IOPCTL_PIO_PULLUP_EN |
                              /* Enables input buffer function */
                              IOPCTL_PIO_INBUF_EN |
                              /* Normal mode */
                              IOPCTL_PIO_SLEW_RATE_NORMAL |
                              /* Normal drive */
                              IOPCTL_PIO_FULLDRIVE_DI |
                              /* Analog mux is disabled */
                              IOPCTL_PIO_ANAMUX_DI |
                              /* Pseudo Output Drain is enabled */
                              IOPCTL_PIO_PSEDRAIN_EN |
                              /* Input function is not inverted */
                              IOPCTL_PIO_INV_DI);
    // Pin 0_15 is SCL for FC2 I2C as defined in Table 300, p261
    IOPCTL_PinMuxSet(IOPCTL, 0U, 15U, pin_cfg);
    // Pin 0_16 is SDA for FC2 I2C as defined in Table 300, p261
    IOPCTL_PinMuxSet(IOPCTL, 0U, 16U, pin_cfg);

    const uint32_t mclk_pin_cfg = (IOPCTL_PIO_FUNC1 |
                                   /* Disable pull-up / pull-down function */
                                   IOPCTL_PIO_PUPD_DI |
                                   /* Enable pull-down function */
                                   IOPCTL_PIO_PULLDOWN_EN |
                                   /* Disable input buffer function */
                                   IOPCTL_PIO_INBUF_DI |
                                   /* Normal mode */
                                   IOPCTL_PIO_SLEW_RATE_NORMAL |
                                   /* Full drive enable */
                                   IOPCTL_PIO_FULLDRIVE_EN |
                                   /* Analog mux is disabled */
                                   IOPCTL_PIO_ANAMUX_DI |
                                   /* Pseudo Output Drain is disabled */
                                   IOPCTL_PIO_PSEDRAIN_DI |
                                   /* Input function is not inverted */
                                   IOPCTL_PIO_INV_DI);

    // Configure 1_10 as MCLK - defined in Table 300 p262
    IOPCTL_PinMuxSet(IOPCTL, 1U, 10U, mclk_pin_cfg);
}

static void serdes_codec_i2s_init()
{

}

void codec_rx_cb(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    // emptyBlock--;
    ;

    // if (g_interruptEnabled && emptyFC4Buffer >0){
    //     emptyFC4Buffer--;
    // }
}

void codec_tx_cb(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    // emptyBlock++;
    ;
}