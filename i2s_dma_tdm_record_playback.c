/** @file i2s_dma_tdm_playback.c
 *
 * This is the original file used for benchmarking I2S communication and functionality.
 * This file will be superceded by main.c
 *
 * @author Matt Richardson
*/

#include <stdio.h>
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include <stdbool.h>
#include "drivers/i2s/i2s.h"
#include "serdes/serdes_event.h"
#include "serdes/serdes_codec.h"
#include "serdes/serdes_defs.h"
#include "serdes/serdes_utilities.h"
#include "serdes/serdes_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Definitions for switch usage... */
// #define APP_GPIO_INTA_IRQHandler GPIO_INTA_DriverIRQHandler
// #define APP_SW_IRQ               GPIO_INTA_IRQn

/*******************************************************************************
 * Variables
 ******************************************************************************/
void i2s_rx_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData);
void fc4_i2s_tx_cb(I2S_Type *,i2s_dma_handle_t *,status_t status,void *);
void fc5_i2s_rx_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *);
void i2s_tx_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData);
// Bridge I2S
i2s_context_t fc4_i2s_context;
i2s_init_t fc4_config = {.flexcomm_bus = FLEXCOMM_4, .is_transmit = true,
                        .is_master = true, .active_channels = 8, .sample_rate = 48000,
                        .datalength = 32, .callback = fc4_i2s_tx_cb, .context = &fc4_i2s_context,
                        .share_clk = false, .shared_clk_set = NO_SHARE};
i2s_context_t fc5_i2s_context;
i2s_init_t fc5_config = {.flexcomm_bus = FLEXCOMM_5, .is_transmit = false,
                        .is_master = false, .active_channels = 8, .sample_rate = 48000,
                        .datalength = 32, .callback = fc5_i2s_rx_cb, .context = &fc5_i2s_context,
                        .share_clk = false, .shared_clk_set = NO_SHARE};
// Receiver for audio in from the codec
i2s_context_t fc1_i2s_context;
i2s_init_t fc1_config = {.flexcomm_bus = FLEXCOMM_1, .is_transmit = false,
                        .is_master = true, .active_channels = 8, .sample_rate = 48000,
                        .datalength = 32, .callback = i2s_rx_Callback, .context = &fc1_i2s_context,
                        .share_clk = false, .shared_clk_set = NO_SHARE};
// Transmitter for audio to codec
i2s_context_t fc3_i2s_context;
i2s_init_t fc3_config = {.flexcomm_bus = FLEXCOMM_3, .is_transmit = true,
                        .is_master = true, .active_channels = 8, .sample_rate = 48000,
                        .datalength = 32, .callback = i2s_tx_Callback, .context = &fc3_i2s_context,
                        .share_clk = false, .shared_clk_set = NO_SHARE};

AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t rcv_Buffer[BUFFER_NUMBER * BUFFER_SIZE], 4);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t tx_Buffer[BUFFER_NUMBER * BUFFER_SIZE], 4);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t rcv5_buffer[BUFFER_NUMBER * BUFFER_SIZE], 4);

static uint32_t tx_index = 0U, rx_index = 0U;

volatile uint32_t emptyBlock = BUFFER_NUMBER;
volatile uint32_t emptyFC4Buffer = BUFFER_NUMBER;

static i2s_transfer_t xfer;
static i2s_transfer_t tx_xfer;

uint64_t evt_times[5] = {0};

int32_t wave[48] = {0};

uint16_t g_tx_error = 0;

bool g_interruptEnabled = false;

static event_t current_events = {.idx = 0, .evt_times = {0}};

/*******************************************************************************
 * Code
 ******************************************************************************/
/** Switch 2 callback for interrupt*/
void sw2_int_cb(void *usrData)
{
    (void)usrData;
    serdes_update_evt_times(RCV_INTERRUPT, &current_events);
    if (g_interruptEnabled)
    {
        memset(rcv5_buffer, 0, BUFFER_NUMBER * BUFFER_SIZE);
    }
    g_interruptEnabled = !g_interruptEnabled;
}

static void rcv_i2s_data(i2s_transfer_t *rcv_transfer)
{
	int32_t *new_data = (int32_t *)(tx_Buffer + tx_index*BUFFER_SIZE);

    memcpy(new_data, rcv_transfer->data, BUFFER_SIZE);
    tx_xfer.data     = (uint8_t *)new_data;
    tx_xfer.dataSize = BUFFER_SIZE;

    // Use this to register an event
    static bool transmitting = false;

    uint32_t num_elements = (SAMPLE_SIZE_MS * SAMPLE_PER_MS *  NUM_CHANNELS);

	// Only intercept the signal if the interrupt is active...
	if (g_interruptEnabled){
        if (!transmitting)
        {
            serdes_update_evt_times(START_TXMIT, &current_events);
            transmitting = true;
        }

		for (uint32_t i = 0, wave_pos = 0; i < num_elements; wave_pos++)
		{
			new_data[i++] = wave[wave_pos % 48];
			new_data[i++] = wave[wave_pos % 48];

            new_data[i+6] =  new_data[i+7] = 0xdeadbeef;
			i+=6;
		}

        if (emptyFC4Buffer < BUFFER_NUMBER)
        {
            I2S_TxTransferSendDMA(fc4_config.context->base, &fc4_config.context->i2s_dma_handle, tx_xfer);
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
        // trans_rcv = transmitting = false;
        transmitting = false;

    }

    if (emptyBlock < BUFFER_NUMBER)
    {
        if (kStatus_Success == I2S_TxTransferSendDMA(fc3_config.context->base, &fc3_config.context->i2s_dma_handle, tx_xfer))
        {
            tx_index++;
        }
        if (tx_index == BUFFER_NUMBER)
        {
            tx_index = 0U;
        }
    }
}
void fc5_i2s_rx_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *)
{
    ;
}

void fc4_i2s_tx_cb(I2S_Type *,i2s_dma_handle_t *,status_t status,void *)
{
    emptyFC4Buffer++;
}


void i2s_rx_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    emptyBlock--;

    if (g_interruptEnabled && emptyFC4Buffer >0){
        emptyFC4Buffer--;
    }
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
    serdes_gpio_cfg_t gpio_cfg = {.sw2_cb = sw2_int_cb};
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    serdes_gpio_init(gpio_cfg);
    i2s_init(fc1_config);
    i2s_init(fc3_config);
    i2s_init(fc4_config);
    i2s_init(fc5_config);

    // Provide a quarter of the volume...
    generate_tone(wave, 48, 16777216);

    // Enable GPIO Ports...not certain why this not done in the pin mux functionality
    // Don't understand - all this does is enable the clocks for these ports but this
    // should already be done
    // GPIO_PortInit(GPIO, 0U);
    // GPIO_PortInit(GPIO, 1U);

    // enable_sw_interrupt();

    CLOCK_EnableClock(kCLOCK_InputMux);

    serdes_event_init();

    PRINTF("I2S TDM record playback example started!\n\r");

    serdes_codec_init();

    PRINTF("Starting TDM record playback\n\r");

    /* Initialize buffer */
	for (uint32_t i = 0; i < BUFFER_SIZE * BUFFER_NUMBER; i++)
	{
		tx_Buffer[i] = rcv_Buffer[i] = rcv5_buffer[i] = 0;
	}

    /* Generate wave for I2S transmission */
    uint8_t rcv5_idx = 0;
    i2s_transfer_t txfr_data;
    static bool trans_rcv = false;
    static bool start_rcv = false;
    while (1)
    {
        if (emptyBlock > 0)
        {
            xfer.data     = rcv_Buffer + rx_index * BUFFER_SIZE;
            xfer.dataSize = BUFFER_SIZE;
            if (kStatus_Success == I2S_RxTransferReceiveDMA(fc1_config.context->base, &fc1_config.context->i2s_dma_handle, xfer))
            {
                rx_index++;
            }
            if (rx_index >= BUFFER_NUMBER)
            {
                rx_index = 0U;
            }
        }
        rcv_i2s_data(&xfer);
        if (g_tx_error != 0 && !(g_tx_error % 10)){
            PRINTF("Tx Error: %d", g_tx_error);
        }
        if (g_interruptEnabled)
        {
            if (!start_rcv)
            {
                serdes_update_evt_times(START_RCV, &current_events);
                start_rcv = true;
            }

            txfr_data.data     = rcv5_buffer + rcv5_idx * BUFFER_SIZE;
            txfr_data.dataSize = BUFFER_SIZE;
            status_t status = 0;

            if (kStatus_Success == (status = I2S_RxTransferReceiveDMA(fc5_config.context->base, &fc5_config.context->i2s_dma_handle, txfr_data)))
            {
                rcv5_idx = rcv5_idx + 1 >= BUFFER_NUMBER ? 0 : rcv5_idx+1;
                uint32_t *data = (uint32_t *)txfr_data.data;

                // Can't tell, but it looks like the highest byte can be garbage (it
                // actually looks like it is the highest bit, but this will do.)
                if (*(data+8) != 0 && (*(data+8 )<<4 == *(data + 9)<<4) && !trans_rcv)
                {
                    serdes_update_evt_times(RCV_TXMIT, &current_events);
                    trans_rcv = true;
                }
            }
        }
        else
        {
            trans_rcv = false;
            start_rcv = false;

        }
    }
}

