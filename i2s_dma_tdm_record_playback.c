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
#include "board/peripherals.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_CODEC_I2C_BASEADDR         I2C2
#define DEMO_CODEC_I2C_INSTANCE         2U
#define DEMO_TDM_DATA_START_POSITION    1U

/* Buffer related definitions */
// Increase buffer size to accommodate adding sine into one channel
#define SAMPLE_SIZE_MS (8U)
#define SAMPLE_PER_MS (48)
#define BYTES_PER_SAMPLE (4)
#define NUM_CHANNELS (8)
#define BUFFER_SIZE   (SAMPLE_SIZE_MS * SAMPLE_PER_MS * BYTES_PER_SAMPLE * NUM_CHANNELS)
#define BUFFER_NUMBER (8U)

/* Constants for data being pushed around */
#define CONST_DATA (0xdeadbeef)
#define M_PI 3.14159265358979323846

/* Definitions for switch usage... */
#define APP_GPIO_INTA_IRQHandler GPIO_INTA_DriverIRQHandler
#define APP_SW_IRQ               GPIO_INTA_IRQn

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

static uint32_t tx_index = 0U, rx_index = 0U;

volatile uint32_t emptyBlock = BUFFER_NUMBER;
extern codec_config_t boardCodecConfig;
codec_handle_t codecHandle;

void *rxData = NULL;
void *txData = NULL;
static i2s_transfer_t xfer;
static i2s_transfer_t tx_xfer;

uint64_t evt_times[5] = {0};

int32_t wave[48] = {0};
uint8_t current_evt_idx = 0;

bool g_interruptEnabled = false;
typedef struct event_s {
    uint8_t idx;
    uint64_t evt_times[5];
} event_t;

typedef enum evt_type_e
{
    RCV_INTERRUPT = 0,
    START_TXMIT = 1,
    RCV_TXMIT = 2
} evt_type_t;

static uint64_t g_tick_per_us = 0;

static uint64_t ostime_get_us();

static event_t current_events = {.idx = 0, .evt_times = {0}};

/*******************************************************************************
 * Code
 ******************************************************************************/
static void update_evt_times(evt_type_t evt_type, event_t *events);

static void generate_wave()
{
	// This is generating a shit wave but it works for the moment
	// Volume is 1/4 max
	int32_t max_vol = 16777216;

	PRINTF("Generating wave now...\r\n");

	for (uint8_t i = 0; i < 48; i++)
	{
		wave[i] = (int32_t)(max_vol * sin(2*M_PI /48*i));
	}
}

void APP_GPIO_INTA_IRQHandler(void)
{
    update_evt_times(RCV_INTERRUPT, &current_events);
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

static void update_evt_times(evt_type_t evt_type, event_t *events)
{
    uint64_t c_time = ostime_get_us();
    events->evt_times[evt_type] = c_time;
    PRINTF("%d: %d\r\n", evt_type, c_time);
}

static void print_evt_data(event_t events)
{
    PRINTF("EVENT\tTIME\r\nINT_RCV\t%d\r\nTXMIT_START\t%d\r\nTXMIT_RCV\t%d\r\n",
        events.evt_times[RCV_INTERRUPT], events.evt_times[START_TXMIT], events.evt_times[RCV_TXMIT]);
}
static void rcv_i2s_data(i2s_transfer_t *rcv_transfer)
{

	int32_t *new_data = (int32_t *)(tx_Buffer + tx_index*BUFFER_SIZE);

    memcpy(new_data, rcv_transfer->data, BUFFER_SIZE);
    tx_xfer.data     = (uint8_t *)new_data;
    tx_xfer.dataSize = BUFFER_SIZE;

    // Use this to register an event
    static bool transmitting = false;
    static bool trans_rcv = false;

    if (!trans_rcv && transmitting && new_data[0] > 0) {
        update_evt_times(RCV_TXMIT, &current_events);
        PRINTF("Receiving transmission\r\n");
        print_evt_data(current_events);
        trans_rcv = true;
    }

    uint32_t num_elements = (SAMPLE_SIZE_MS * SAMPLE_PER_MS *  NUM_CHANNELS);

	// Only intercept the signal if the interrupt is active...
	if (g_interruptEnabled){
        if (!transmitting)
        {
            update_evt_times(START_TXMIT, &current_events);
            transmitting = true;
        }

		for (uint32_t i = 0, wave_pos = 0; i < num_elements; wave_pos++)
		{
            // Copy incoming data on 0 and 1 into 2 and 3
			new_data[i+2] = new_data[i];
			new_data[i+3] = new_data[i+1];

			new_data[i++] = wave[wave_pos % 48];
			new_data[i++] = wave[wave_pos % 48];
			i+=6;
		}

        status_t i2s_status = kStatus_Success;
        i2s_status = I2S_TxTransferSendDMA(FC5_I2S_TX_PERIPHERAL, &FC4_I2S_Tx_Handle, tx_xfer);

        if (kStatus_Success != i2s_status)
        {
            PRINTF("TXFR Failure");
        }

	}
    else
    {
		for (uint32_t i = 0, wave_pos = 0; i < num_elements; wave_pos++)
		{
            // Copy incoming data on 0 and 1 into 2 and 3
			new_data[i+2] = new_data[i];
			new_data[i+3] = new_data[i+1];

			new_data[i++] = 0;
			new_data[i++] = 0;
			i+=6;
		}
        trans_rcv = transmitting = false;

    }

    if (emptyBlock < BUFFER_NUMBER)
    {
        if (kStatus_Success == I2S_TxTransferSendDMA(FLEXCOMM3_PERIPHERAL, &i2sTxHandle, tx_xfer))
        {
            tx_index++;
        }
        if (tx_index == BUFFER_NUMBER)
        {
            tx_index = 0U;
        }
    }
}
void * fc5TxData = NULL;
void fc5_i2s_tx_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *)
{
    ;
}

void i2s_rx_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    emptyBlock--;
}

void i2s_tx_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
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
    BOARD_InitBootPeripherals();

    // Enable GPIO Ports...not certain why this not done in the pin mux functionality
    // Don't understand - all this does is enable the clocks for these ports but this
    // should already be done
    GPIO_PortInit(GPIO, 0U);
    GPIO_PortInit(GPIO, 1U);

    enable_sw_interrupt();

    CLOCK_EnableClock(kCLOCK_InputMux);

    SYSCTL1->MCLKPINDIR = SYSCTL1_MCLKPINDIR_MCLKPINDIR_MASK;

    ostime_init();

    cs42448Config.i2cConfig.codecI2CSourceClock = CLOCK_GetFlexCommClkFreq(2);
    cs42448Config.format.mclk_HZ                = CLOCK_GetMclkClkFreq();

    PRINTF("I2S TDM record playback example started!\n\r");

    /* codec initialization */
    DEMO_InitCodec();

    PRINTF("Starting TDM record playback\n\r");

    /* Initialize buffer */
	for (uint32_t i = 0; i < BUFFER_SIZE * BUFFER_NUMBER; i++)
	{
		tx_Buffer[i] = rcv_Buffer[i] =  0;
	}

    /* Generate wave for I2S transmission */
    generate_wave();

    while (1)
    {
        if (emptyBlock > 0)
        {
            xfer.data     = rcv_Buffer + rx_index * BUFFER_SIZE;
            xfer.dataSize = BUFFER_SIZE;
            if (kStatus_Success == I2S_RxTransferReceiveDMA(FLEXCOMM1_PERIPHERAL, &i2sRxHandle, xfer))
            {
                rx_index++;
            }
            if (rx_index >= BUFFER_NUMBER)
            {
                rx_index = 0U;
            }
        }
        rcv_i2s_data(&xfer);
    }
}

static void DEMO_InitCodec(void)
{
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        PRINTF("CODEC_Init failed!\r\n");
        assert(false);
    }
    CODEC_SetVolume(&codecHandle, 0, 100);
    CODEC_SetVolume(&codecHandle, 1, 100);

    PRINTF("\r\nCodec Init Done.\r\n");
}
