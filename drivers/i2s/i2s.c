/** @file i2s.c
 *
 * Driver implementation of I2S.
*/

#include "i2s.h"
#include "assert.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_iopctl.h"
#include "i2s_defs.h"
#include "../gpio/gpio_def.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FLEXCOMM_CLOCK_SOURCE 24576000UL

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

typedef struct i2s_pins_s {
    uint8_t port;
    uint8_t sck_pin;
    uint8_t ws_pin;
    uint8_t data_pin;
} i2s_pins_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/** Populate the peripheral base address
 *
 * @param config I2S configuration for bus
*/
static void set_peripheral_address(i2s_init_t config);

/** Configure I2S pins
 *
 * All pins for I2S will have the following configuration:
 * FUNC: 1
 * PUPDENA: Disabled
 * PUPDSEL: 0
 * IBENA: enabled
 * SLEWRATE: standard
 * FULLDRIVE: normal
 * AMENA: disabled
 * ODENA: disabled
 * IIENA: not inverted
 * These settings are suggested for I2S functionality on pg 576 of the
 * manual, Table 581
 *
 * @param config I2S configuration structure
 *
*/
static void i2s_pin_init(i2s_init_t config);

static void dma_init(i2s_init_t config);

/** Set up clock functionalities associated with I2S
 *
 * This will do several actions based on the input configuration:
 * * attach audio pll clocks to the appropriate flexcomm ports
 * * set up signal sharing.
 * * set the mclk direction
 *
 * I2S signal sharing is discussed in chapter 4.  Registers are defined in
 * 4.5.6 and I2S signal bridging is discussed at length in 4.6.2.  Bridging
 *
 * @param config I2S configuration structure
*/
static void clock_init(i2s_init_t config);

/**
 * @brief Retrieve the DMA Channel number for a particular bus
 *
 * DMA Channels are defined in Table 364; channel is determined by whether it
 * is receive or transmit
 *
 * @param fc_port Flexcomm port number that I2S bus is on
 * @param is_transmit boolean indicating whether the port is configured for
 *                    transmitting or not
 * @return Channel number
 */
static uint8_t get_dma_channel(flexcomm_port_t fc_port, bool is_transmit);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/** Is DMA initialized
 *
 * This assumes that all I2S buses use the same DMA instance (DMA0). Ensure
 * that DMA gets initialized only once.
*/
static bool base_dma_initialized = false;

/** Pin definitions for Flexcomm busses supporting I2S
 *
 * These pinouts are described in Table 4.  Each one of these
 * pinout designations are for use with FUNC1 in the pin mux
 * configuration.
 *
 */
const i2s_pins_t i2s_pin_config[] =
{
    {.port = 0U, .sck_pin = 0U,  .ws_pin = 1U,  .data_pin = 2U},    // FC 0
    {.port = 0U, .sck_pin = 7U,  .ws_pin = 8U,  .data_pin = 9U},    // FC 1
    {.port = 0U, .sck_pin = 14U, .ws_pin = 15U, .data_pin = 16U},   // FC 2
    {.port = 0U, .sck_pin = 21U, .ws_pin = 22U, .data_pin = 23U},   // FC 3
    {.port = 0U, .sck_pin = 28U, .ws_pin = 29U, .data_pin = 30U},   // FC 4
    {.port = 1U, .sck_pin = 3U,  .ws_pin = 4U,  .data_pin = 5U},    // FC 5
    {.port = 3U, .sck_pin = 25U, .ws_pin = 26U, .data_pin = 27U},   // FC 6
    {.port = 4U, .sck_pin = 0U,  .ws_pin = 1U,  .data_pin = 2U},    // FC 7
};

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

void i2s_init(i2s_init_t config)
{
    // Must have between 2 and 8 channels and they must be in pairs
    assert(config.active_channels > 2 && config.active_channels <= 8
            && !(config.active_channels % 2));

    assert(config.flexcomm_bus >= FLEXCOMM_0 && config.flexcomm_bus < FLEXCOMM_ND);

    i2s_config_t i2sConfig;
    clock_init(config);

    i2s_pin_init(config);

    if (config.is_transmit)
    {
        /* Default config:
        *   masterSlave = kI2S_MasterSlaveNormalMaster;
        *   mode = kI2S_ModeI2sClassic;
        *   rightLow = false;
        *   leftJust = false;
        *   pdmData = false;
        *   sckPol = false;
        *   wsPol = false;
        *   divider = 1;
        *   oneChannel = false;
        *   dataLength = 16;
        *   frameLength = 32;
        *   position = 0;
        *   watermark = 4;
        *   txEmptyZero = true;
        *   pack48 = false;
        */
        I2S_TxGetDefaultConfig(&i2sConfig);
    } else
    {
        /* Default config:
        *   masterSlave = kI2S_MasterSlaveNormalSlave;
        *   mode = kI2S_ModeI2sClassic;
        *   rightLow = false;
        *   leftJust = false;
        *   pdmData = false;
        *   sckPol = false;
        *   wsPol = false;
        *   divider = 1;
        *   oneChannel = false;
        *   dataLength = 16;
        *   frameLength = 32;
        *   position = 0;
        *   watermark = 4;
        *   txEmptyZero = false;
        *   pack48 = false;
        */
        I2S_RxGetDefaultConfig(&i2sConfig);
    }

    i2sConfig.position = config.is_transmit ? 1 : 0;
    i2sConfig.masterSlave = config.is_master ? kI2S_MasterSlaveNormalMaster : kI2S_MasterSlaveNormalSlave;
    i2sConfig.mode = config.active_channels > 2 ? kI2S_ModeDspWsShort : kI2S_ModeI2sClassic;
    i2sConfig.dataLength = config.datalength;
    i2sConfig.frameLength = config.datalength * config.active_channels;
    i2sConfig.divider = config.is_master ? (FLEXCOMM_CLOCK_SOURCE / config.sample_rate / i2sConfig.frameLength) : 1;
    i2sConfig.wsPol = true;

    set_peripheral_address(config);

    if (config.is_transmit)
    {
        I2S_TxInit(config.context->base, &i2sConfig);
    } else
    {
        I2S_RxInit(config.context->base, &i2sConfig);
    }

    if (config.active_channels > 2)
    {
        // Divide the active number of channels by 2; we have already established the
        // channels are in pairs
        uint8_t channel_pairs = config.active_channels >> 1;
        for (uint8_t frame = 1; frame < channel_pairs; frame++)
        {
            uint8_t position = i2sConfig.position + frame * i2sConfig.dataLength * 2;
            I2S_EnableSecondaryChannel(config.context->base, frame-1, false, position);
        }
    }

    // I2S CONFIGURATION COMPLETE //

    dma_init(config);
}

void i2s_stop(i2s_init_t config)
{
    /* This one call will do everything we need:
     * * Disable DMA interrupts
     * * Abort DMA transfers
     * * Disable DMA
     * * Disable I2S
     *
     * So this function is essentially a wrapper so that the modules above
     * do not have to call the fsl_i2s_dma module.
    */
    I2S_TransferAbortDMA(config.context->base, &config.context->i2s_dma_handle);
}


// Documented in .h
static void i2s_pin_init(i2s_init_t config)
{
    uint32_t pin_config = (
                        IOPCTL_PIO_FUNC1 |
                        IOPCTL_PIO_PUPD_DI |
                        IOPCTL_PIO_PULLDOWN_EN |
                        IOPCTL_PIO_INBUF_EN |
                        IOPCTL_PIO_SLEW_RATE_NORMAL |
                        IOPCTL_PIO_FULLDRIVE_EN |
                        IOPCTL_PIO_ANAMUX_DI |
                        IOPCTL_PIO_PSEDRAIN_DI |
                        IOPCTL_PIO_INV_DI);

    IOPCTL_PinMuxSet(IOPCTL, i2s_pin_config[config.flexcomm_bus].port, i2s_pin_config[config.flexcomm_bus].sck_pin,   pin_config);
    IOPCTL_PinMuxSet(IOPCTL, i2s_pin_config[config.flexcomm_bus].port, i2s_pin_config[config.flexcomm_bus].ws_pin,    pin_config);
    IOPCTL_PinMuxSet(IOPCTL, i2s_pin_config[config.flexcomm_bus].port, i2s_pin_config[config.flexcomm_bus].data_pin,  pin_config);
}

// Documented above
static void set_peripheral_address(i2s_init_t config)
{
    switch(config.flexcomm_bus)
    {
        case FLEXCOMM_0:
            config.context->base = (I2S_Type *)FLEXCOMM0;
            break;
        case FLEXCOMM_1:
            config.context->base = (I2S_Type *)FLEXCOMM1;
            break;
        case FLEXCOMM_2:
            config.context->base = (I2S_Type *)FLEXCOMM2;
            break;
        case FLEXCOMM_3:
            config.context->base = (I2S_Type *)FLEXCOMM3;
            break;
        case FLEXCOMM_4:
            config.context->base = (I2S_Type *)FLEXCOMM4;
            break;
        case FLEXCOMM_5:
            config.context->base = (I2S_Type *)FLEXCOMM5;
            break;
        case FLEXCOMM_6:
            config.context->base = (I2S_Type *)FLEXCOMM6;
            break;
        case FLEXCOMM_7:
            config.context->base = (I2S_Type *)FLEXCOMM7;
            break;
        default:
            break;

    }

}

static void dma_init(i2s_init_t config)
{
    // I2S peripheral must be defined
    assert(config.context->base);

    // Initialize the DMA module as necessary..
    if (!base_dma_initialized)
    {
        DMA_Init(I2S_DMA);
        base_dma_initialized = true;
    }

    uint32_t dma_channel = (uint32_t)get_dma_channel(config.flexcomm_bus, config.is_transmit);

    DMA_EnableChannel(I2S_DMA, dma_channel);
    DMA_SetChannelPriority(I2S_DMA, dma_channel, kDMA_ChannelPriority3);
    DMA_CreateHandle(&config.context->dma_handle, I2S_DMA, dma_channel);

    if (config.is_transmit)
    {
        I2S_TxTransferCreateHandleDMA(config.context->base, &config.context->i2s_dma_handle,
                                      &config.context->dma_handle, config.callback, NULL);

    } else {
        I2S_RxTransferCreateHandleDMA(config.context->base, &config.context->i2s_dma_handle,
                                      &config.context->dma_handle, config.callback, NULL);
    }
}

// Defined above
static void clock_init(i2s_init_t config)
{
    switch (config.flexcomm_bus)
    {
        case FLEXCOMM_0:
            CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM0);
            break;
        case FLEXCOMM_1:
            CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM1);
            break;
        case FLEXCOMM_2:
            CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM2);
            break;
        case FLEXCOMM_3:
            CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM3);
            break;
        case FLEXCOMM_4:
            CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM4);
            break;
        case FLEXCOMM_5:
            CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM5);
            break;
        case FLEXCOMM_6:
            CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM6);
            break;
        case FLEXCOMM_7:
            CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM7);
            break;
        default:
            break;
    }

    // Set up signal sharing as requested.  This is defined in section 4.5.6.11 for shared control set 0,
    // 4.5.6.12 for shared set 2 and sections 4.5.6.3-10 for the control selection in I2S signal sharing.
    // Bridging is explained in 4.6.2
    if (config.share_clk && config.shared_clk_set != NO_SHARE)
    {
        SYSCTL1->SHAREDCTRLSET[config.shared_clk_set-1] = SYSCTL1_SHAREDCTRLSET_SHAREDSCKSEL(config.flexcomm_bus) |
                                    SYSCTL1_SHAREDCTRLSET_SHAREDWSSEL(config.flexcomm_bus);
    }
    else if (config.shared_clk_set != NO_SHARE)
    {
        SYSCTL1->FCCTRLSEL[config.flexcomm_bus] = SYSCTL1_FCCTRLSEL_SCKINSEL(config.shared_clk_set) |
                                                  SYSCTL1_FCCTRLSEL_WSINSEL(config.shared_clk_set);
    }

    // Set MCLK direction as output; this is defined in section 4.5.6.1
    // This can be set multiple times no big deal...
    SYSCTL1->MCLKPINDIR = SYSCTL1_MCLKPINDIR_MCLKPINDIR_MASK;
}

static uint8_t get_dma_channel(flexcomm_port_t fc_port, bool is_transmit)
{
    return is_transmit ? 1 + 2 * fc_port : 2 * fc_port;
}
