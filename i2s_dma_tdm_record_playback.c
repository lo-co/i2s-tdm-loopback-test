/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i2s_dma.h"
#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "fsl_ostimer.h"
#include <stdbool.h>
#include "fsl_cs42448.h"
#include "math.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_I2S_MASTER_CLOCK_FREQUENCY CLOCK_GetMclkClkFreq()
#define DEMO_AUDIO_SAMPLE_RATE          (48000)
#define DEMO_AUDIO_PROTOCOL             kCODEC_BusI2S
#define DEMO_I2S_TX                     (I2S3)
#define DEMO_I2S_RX                     (I2S1)
#define DEMO_DMA                        (DMA0)
#define DEMO_I2S_TX_CHANNEL             (7)
#define DEMO_I2S_RX_CHANNEL             (2)
#define DEMO_I2S_CLOCK_DIVIDER          (24576000 / DEMO_AUDIO_SAMPLE_RATE / 32 / 8)
#define DEMO_I2S_TX_MODE                kI2S_MasterSlaveNormalSlave
#define DEMO_I2S_RX_MODE                kI2S_MasterSlaveNormalMaster
#define DEMO_CODEC_I2C_BASEADDR         I2C2
#define DEMO_CODEC_I2C_INSTANCE         2U
#define DEMO_TDM_DATA_START_POSITION    1U
// Increase buffer size to accommodate adding sine into one channel
#define CHANNEL_PAIRS (4U)
#define SAMPLE_SIZE_MS (8)
#define SAMPLE_PER_MS (48)
#define BYTES_PER_SAMPLE (4)
#define NUM_CHANNELS (8)
#define BUFFER_SIZE   (SAMPLE_SIZE_MS * SAMPLE_PER_MS * BYTES_PER_SAMPLE * NUM_CHANNELS)
#define BUFFER_NUMBER (4U)
#define CONST_DATA (0xdeadbeef)
#define M_PI 3.14159265358979323846

// For SW2 functionality
#define APP_GPIO_INTA_IRQHandler GPIO_INTA_DriverIRQHandler
#define APP_SW_IRQ               GPIO_INTA_IRQn

/* demo audio sample rate */
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void DEMO_InitCodec(void);
extern void BORAD_CodecReset(bool state);
/*******************************************************************************
 * Variables
 ******************************************************************************/
cs42448_config_t cs42448Config = {
    .DACMode      = kCS42448_ModeSlave,
    .ADCMode      = kCS42448_ModeSlave,
    .reset        = NULL,
    .master       = false,
    .i2cConfig    = {.codecI2CInstance = DEMO_CODEC_I2C_INSTANCE},
    .format       = {.sampleRate = 48000U, .bitWidth = 24U},
    .bus          = kCS42448_BusTDM,
    .slaveAddress = CS42448_I2C_ADDR,
};

codec_config_t boardCodecConfig = {.codecDevType = kCODEC_CS42448, .codecDevConfig = &cs42448Config};
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t rcv_Buffer[BUFFER_NUMBER * BUFFER_SIZE], 4);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t tx_Buffer[BUFFER_NUMBER * BUFFER_SIZE], 4);

static uint32_t tx_index = 1U, rx_index = 0U;

volatile uint32_t emptyBlock = BUFFER_NUMBER;
extern codec_config_t boardCodecConfig;
codec_handle_t codecHandle;
static i2s_config_t s_TxConfig;
static i2s_config_t s_RxConfig;
static i2s_dma_handle_t s_i2sTxHandle;
static i2s_dma_handle_t s_i2sRxHandle;
static dma_handle_t s_i2sTxDmaHandle;
static dma_handle_t s_i2sRxDmaHandle;
static i2s_transfer_t xfer;

uint64_t evt_times[5] = {0};

int32_t wave[48] = {0};
uint8_t current_evt_idx = 0;

bool g_interruptEnabled = false;

static uint64_t g_tick_per_us = 0;

static uint64_t ostime_get_us();

/*******************************************************************************
 * Code
 ******************************************************************************/
static void generate_wave()
{
	// This is generating a shit wave but it works for the moment
	// Volume is 1/4 max
	int32_t max_vol = 88080384;

	PRINTF("Generating wave now...\r\n");

	for (uint8_t i = 0; i < 48; i++)
	{
		wave[i] = (int32_t)(max_vol * sin(2*M_PI /48*i));
	}

}
void APP_GPIO_INTA_IRQHandler(void)
{
    evt_times[current_evt_idx++] = ostime_get_us();
    /* clear the interrupt status */
    GPIO_PinClearInterruptFlag(SW2_GPIO, SW2_PORT, SW2_PIN, 0);
    /* Change state of switch. */
    g_interruptEnabled = !g_interruptEnabled;
    SDK_ISR_EXIT_BARRIER;
}

static void enable_sw_interrupt()
{
    gpio_interrupt_config_t config = {kGPIO_PinIntEnableEdge, kGPIO_PinIntEnableLowOrFall};

    EnableIRQ(APP_SW_IRQ);
    /* Initialize GPIO functionality on pin PIO0_10 (pin J3)  */
    // Ugh...config tool does a half ass job of this
    GPIO_SetPinInterruptConfig(SW2_GPIO, SW2_PORT, SW2_PIN, &config);
    GPIO_PinEnableInterrupt(SW2_GPIO, SW2_PORT, SW2_PIN, 0);
}

static void ostime_init()
{
    // static bool initialized = false;

    CLOCK_AttachClk(kHCLK_to_OSTIMER_CLK);
    OSTIMER_Init(OSTIMER0);

    RSTCTL1->PRSTCTL0 != RSTCTL1_PRSTCTL0_OSEVT_TIMER_RST_MASK;
    RSTCTL1->PRSTCTL0 &= ~RSTCTL1_PRSTCTL0_OSEVT_TIMER_RST_MASK;

    g_tick_per_us = (uint32_t)(CLOCK_GetFreq(kCLOCK_CoreSysClk) / 1e6);
}

static uint64_t ostime_get_us()
{
    const uint64_t timer_ticks = OSTIMER_GetCurrentTimerValue(OSTIMER0);

    return (uint64_t)(timer_ticks/g_tick_per_us);

}


static void i2s_rx_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
	i2s_transfer_t i2s_data = *(i2s_transfer_t *)userData;

	int32_t *new_data = (int32_t *)(i2s_data.data);

    // Use this to register an event
    static bool transmitting = false;
    static bool trans_rcv = false;

    if (!trans_rcv && transmitting && new_data[8] == wave[1] && new_data[16] == wave[2]) {
        evt_times[current_evt_idx++] = ostime_get_us();
        trans_rcv = true;
    }


	// Only intercept the signal if the interrupt is active...
	if (g_interruptEnabled){
        uint32_t i = 0;
        uint32_t wave_pos = 0;
        if (!transmitting)
        {
            evt_times[current_evt_idx++] = ostime_get_us();
            transmitting = true;
        }

		uint32_t num_elements = (SAMPLE_SIZE_MS * SAMPLE_PER_MS *  NUM_CHANNELS);

		for (; i < num_elements;wave_pos++)
		{
			new_data[i++] = wave[wave_pos % 48];
			new_data[i++] = wave[wave_pos % 48];
			i+=6;
		}
	}
    else
    {
        trans_rcv = transmitting = false;

    }

    if (emptyBlock < BUFFER_NUMBER)
    {
        xfer.data     = tx_Buffer + tx_index * BUFFER_SIZE;
        xfer.dataSize = BUFFER_SIZE;
        if (kStatus_Success == I2S_TxTransferSendDMA(DEMO_I2S_TX, &s_i2sTxHandle, xfer))
        {
            tx_index++;
        }
        if (tx_index == BUFFER_NUMBER)
        {
            tx_index = 0U;
        }
    }
    emptyBlock--;
}

static void i2s_tx_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    emptyBlock++;
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    // Enable GPIO Ports...not certain why this not done in the pin mux functionality
    // Don't understand - all this does is enable the clocks for these ports but this
    // should already be done
    GPIO_PortInit(GPIO, 0U);
    GPIO_PortInit(GPIO, 1U);

    enable_sw_interrupt();

    CLOCK_EnableClock(kCLOCK_InputMux);

    /* I2C */
    CLOCK_AttachClk(kFFRO_to_FLEXCOMM2);

    /* attach AUDIO PLL clock to FLEXCOMM1 (I2S1) */
    CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM1);
    /* attach AUDIO PLL clock to FLEXCOMM3 (I2S3) */
    CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM3);

    /* attach AUDIO PLL clock to MCLK */
    CLOCK_AttachClk(kAUDIO_PLL_to_MCLK_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivMclkClk, 1);
    SYSCTL1->MCLKPINDIR = SYSCTL1_MCLKPINDIR_MCLKPINDIR_MASK;

    ostime_init();

    cs42448Config.i2cConfig.codecI2CSourceClock = CLOCK_GetFlexCommClkFreq(2);
    cs42448Config.format.mclk_HZ                = CLOCK_GetMclkClkFreq();

    PRINTF("I2S TDM record playback example started!\n\r");

    /* i2s configurations */
    /*
     * masterSlave = kI2S_MasterSlaveNormalMaster;
     * mode = kI2S_ModeI2sClassic;
     * rightLow = false;
     * leftJust = false;
     * pdmData = false;
     * sckPol = false;
     * wsPol = false;
     * divider = 1;
     * oneChannel = false;
     * dataLength = 16;
     * frameLength = 32;
     * position = 0;
     * fifoLevel = 4;
     */
    I2S_TxGetDefaultConfig(&s_TxConfig);
    s_TxConfig.divider     = DEMO_I2S_CLOCK_DIVIDER;
    s_TxConfig.mode        = kI2S_ModeDspWsShort;
    s_TxConfig.wsPol       = true;
    s_TxConfig.dataLength  = 32U;
    s_TxConfig.frameLength = 32 * 8U;
    s_TxConfig.position    = DEMO_TDM_DATA_START_POSITION;

    I2S_TxInit(DEMO_I2S_TX, &s_TxConfig);
    I2S_EnableSecondaryChannel(DEMO_I2S_TX, kI2S_SecondaryChannel1, false, 64 + DEMO_TDM_DATA_START_POSITION);
    I2S_EnableSecondaryChannel(DEMO_I2S_TX, kI2S_SecondaryChannel2, false, 128 + DEMO_TDM_DATA_START_POSITION);
    I2S_EnableSecondaryChannel(DEMO_I2S_TX, kI2S_SecondaryChannel3, false, 192 + DEMO_TDM_DATA_START_POSITION);

    /* i2s configurations */
    I2S_RxGetDefaultConfig(&s_RxConfig);
    s_RxConfig.divider     = DEMO_I2S_CLOCK_DIVIDER;
    s_RxConfig.masterSlave = kI2S_MasterSlaveNormalMaster;
    s_RxConfig.mode        = kI2S_ModeDspWsShort;
    s_RxConfig.wsPol       = true;
    s_RxConfig.dataLength  = 32U;
    s_RxConfig.frameLength = 32 * 8U;
    s_RxConfig.position    = DEMO_TDM_DATA_START_POSITION;

    I2S_RxInit(DEMO_I2S_RX, &s_RxConfig);
    I2S_EnableSecondaryChannel(DEMO_I2S_RX, kI2S_SecondaryChannel1, false, 64 + DEMO_TDM_DATA_START_POSITION);
    I2S_EnableSecondaryChannel(DEMO_I2S_RX, kI2S_SecondaryChannel2, false, 128 + DEMO_TDM_DATA_START_POSITION);
    I2S_EnableSecondaryChannel(DEMO_I2S_RX, kI2S_SecondaryChannel3, false, 192 + DEMO_TDM_DATA_START_POSITION);

    DMA_Init(DEMO_DMA);

    DMA_EnableChannel(DEMO_DMA, DEMO_I2S_TX_CHANNEL);
    DMA_EnableChannel(DEMO_DMA, DEMO_I2S_RX_CHANNEL);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_I2S_TX_CHANNEL, kDMA_ChannelPriority3);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_I2S_RX_CHANNEL, kDMA_ChannelPriority2);
    DMA_CreateHandle(&s_i2sTxDmaHandle, DEMO_DMA, DEMO_I2S_TX_CHANNEL);
    DMA_CreateHandle(&s_i2sRxDmaHandle, DEMO_DMA, DEMO_I2S_RX_CHANNEL);

    // void *usrData = (void *)&xfer;
    I2S_TxTransferCreateHandleDMA(DEMO_I2S_TX, &s_i2sTxHandle, &s_i2sTxDmaHandle, i2s_tx_Callback, NULL);
    I2S_RxTransferCreateHandleDMA(DEMO_I2S_RX, &s_i2sRxHandle, &s_i2sRxDmaHandle, i2s_rx_Callback, (void *)&xfer);

    /* codec initialization */
    DEMO_InitCodec();

    PRINTF("Starting TDM record playback\n\r");
    GPIO_PinWrite(GPIO, LED_BLUE_PORT, LED_BLUE_PIN, 0U);
    /* Port masking */
    GPIO_PortMaskedSet(GPIO, LED_BLUE_PORT, 0x0000FFFF);
    GPIO_PortMaskedWrite(GPIO, LED_BLUE_PORT, 0xFFFFFFFF);
    uint32_t port_state = GPIO_PortRead(GPIO, LED_BLUE_PORT);
    PRINTF("\r\n Standard port read: %x\r\n", port_state);
    port_state = GPIO_PortMaskedRead(GPIO, LED_BLUE_PORT);
    PRINTF("\r\n Masked port read: %x\r\n", port_state);
    GPIO_PinWrite(GPIO, LED_BLUE_PORT, LED_BLUE_PIN, 1U);
    GPIO_PortToggle(GPIO, LED_BLUE_PORT, 1u << LED_BLUE_PIN);

    port_state = GPIO_PortRead(GPIO, LED_BLUE_PORT);
    PRINTF("\r\n Standard port read: %x\r\n", port_state);

	for (uint32_t i = 0; i < BUFFER_SIZE * BUFFER_NUMBER; i++)
	{
		tx_Buffer[i] = rcv_Buffer[i] =  0;
	}

    generate_wave();

    while (1)
    {
        if (emptyBlock > 0)
        {
            xfer.data     = rcv_Buffer + rx_index * BUFFER_SIZE;
            xfer.dataSize = BUFFER_SIZE;
            if (kStatus_Success == I2S_RxTransferReceiveDMA(DEMO_I2S_RX, &s_i2sRxHandle, xfer))
            {
                rx_index++;
            }
            if (rx_index == BUFFER_NUMBER)
            {
                rx_index = 0U;
            }
        }
    }
}

static void DEMO_InitCodec(void)
{
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        PRINTF("CODEC_Init failed!\r\n");
        assert(false);
    }

    PRINTF("\r\nCodec Init Done.\r\n");
}
