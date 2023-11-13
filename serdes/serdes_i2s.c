#include "serdes_i2s.h"
#include "fsl_i2c.h"
#include "serdes_defs.h"
#include "serdes_event.h"
#include "serdes_memory.h"
#include "../drivers/i2s/i2s_defs.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DATA_LEN_BITS  (BYTES_PER_SAMPLE * 8U)

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief Callback for an I2S DMA transmit transfer.
 *
 * This call will line up the next transfer, reading the next bit of memory from
 * the audio data buffer.
 *
 */
static void tx_i2s_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *);

/**
 * @brief Callback for receiving I2S data.
 *
 * This call will flag an audio received event.  The dispatcher for that event
 * will then parse the incoming stream for any event data
 *
 */
static void rx_i2s_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *);

/**
 * @brief Event handler for data received on the RX line.
 *
 * This will provide the buffer for parsing for data that will require a response
 * from the system.
 *
 * @param usrData This will be the input buffer from the receive line
 * @return 0 on success
 */
static uint8_t i2s_rx_event_cb(void *usrData);

uint8_t i2s_tx_data_request(void *usrData);


/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint8_t tx_idx = 0, rx_idx = 0;

/**
 * @brief Variable indicating whether the I2S (tx and rcv) is active
 */
static bool bus_is_running = false;

// Bridge I2S
i2s_context_t tx_i2s_context;
i2s_init_t tx_config = {.flexcomm_bus = FLEXCOMM_4, .is_transmit = true,
                        .is_master = true, .active_channels = NUM_CHANNELS,
                        .sample_rate = SAMPLE_RATE_HZ, .datalength = DATA_LEN_BITS,
                        .callback = tx_i2s_cb, .context = &tx_i2s_context,
                        .share_clk = false, .shared_clk_set = NO_SHARE};
i2s_context_t rx_i2s_context;
i2s_init_t rx_config = {.flexcomm_bus = FLEXCOMM_5, .is_transmit = false,
                        .is_master = true, .active_channels = NUM_CHANNELS,
                        .sample_rate = SAMPLE_RATE_HZ, .datalength = DATA_LEN_BITS,
                        .callback = rx_i2s_cb, .context = &rx_i2s_context,
                        .share_clk = false, .shared_clk_set = NO_SHARE};

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

// Documented in .h
void serdes_i2s_init(bool is_master)
{
    if (!is_master)
    {
        tx_config.flexcomm_bus = FLEXCOMM_5;
        tx_config.is_master = false;
        rx_config.flexcomm_bus = FLEXCOMM_4;
        rx_config.is_master = false;
    } else
    {
        tx_config.share_clk = true;
        tx_config.shared_clk_set = SHARED_SET_1;
        rx_config.shared_clk_set = SHARED_SET_1;

    }

    // Make sure the buffers contain nothing...
    serdes_memory_init();

    i2s_init(tx_config);
    i2s_init(rx_config);
    serdes_register_handler(DATA_RECEIVED, i2s_rx_event_cb);
    serdes_register_handler(DATA_AVAILABLE, i2s_tx_data_request);

    // Just run...listening
    if (!is_master)
    {
        serdes_i2s_start_slave();
    }
}

// Documented in .h
void serdes_i2s_start()
{
    bus_is_running = true;
    // In the future, this should send 0's maybe?
    i2s_transfer_t txf = {.data = serdes_get_next_tx_buffer(), .dataSize = BUFFER_SIZE};
    I2S_TxTransferSendDMA(tx_config.context->base, &tx_config.context->i2s_dma_handle, txf);
    txf.data = serdes_get_next_rx_buffer();
    I2S_RxTransferReceiveDMA(rx_config.context->base, &rx_config.context->i2s_dma_handle, txf);
}

void serdes_i2s_start_slave()
{
    bus_is_running = true;
    i2s_transfer_t txf = {.data = serdes_get_next_rx_buffer(), .dataSize = BUFFER_SIZE};
    I2S_RxTransferReceiveDMA(rx_config.context->base, &rx_config.context->i2s_dma_handle, txf);
    // serdes_push_event(DATA_RECEIVED, txf.data);
}

// Documented in .h
void serdes_i2s_stop()
{
    i2s_stop(tx_config);
    i2s_stop(rx_config);
    bus_is_running = false;
}

// Documented in .h
bool serdes_i2s_is_running()
{
    return bus_is_running;
}

// Documented above
static void tx_i2s_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *)
{
    if (serdes_memory_more_audio_data())
    {
        i2s_transfer_t txf = {.data = serdes_get_next_tx_buffer(), .dataSize = BUFFER_SIZE};
        I2S_TxTransferSendDMA(tx_config.context->base, &tx_config.context->i2s_dma_handle, txf);

        // Make sure the buffer is in bounds
        tx_idx = tx_idx >= BUFFER_NUMBER ? 0 : tx_idx;
    }
}

// Documented above
static void rx_i2s_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *)
{
    i2s_transfer_t txf = {.data = serdes_get_next_rx_buffer(), .dataSize = BUFFER_SIZE};
    I2S_RxTransferReceiveDMA(rx_config.context->base, &rx_config.context->i2s_dma_handle, txf);

    serdes_push_event(DATA_RECEIVED, txf.data);

    // Make sure the buffer is in bounds
    rx_idx = rx_idx >= BUFFER_NUMBER ? 0 : rx_idx;
}

// Documented above
uint8_t i2s_rx_event_cb(void *usrData)
{
    // TODO: is there any reason that data might show up farther down the line than the first packet?
    // Parse events here
    return 0;
}

uint8_t i2s_tx_data_request(void *usrData)
{
    i2s_transfer_t txf = {.data = serdes_get_next_tx_buffer(), .dataSize = BUFFER_SIZE};
    status_t status = I2S_TxTransferSendDMA(tx_config.context->base, &tx_config.context->i2s_dma_handle, txf);

    if (!status)
    {
        ;
    }
    return 0;
}

uint32_t serdes_i2s_get_fifo_status(i2s_bus_t i2s_bus)
{
    if (i2s_bus == TX)
    {
        return tx_config.context->base->FIFOSTAT;
    }
    else
    {
        return rx_config.context->base->FIFOSTAT;
    }
}

uint32_t serdes_i2s_get_fifo_config(i2s_bus_t i2s_bus)
{
    if (i2s_bus == TX)
    {
        return tx_config.context->base->FIFOCFG;
    }
    else
    {
        return rx_config.context->base->FIFOCFG;
    }
}

uint32_t serdes_i2s_get_cfg1(i2s_bus_t i2s_bus)
{
    if (i2s_bus == TX)
    {
        return tx_config.context->base->CFG1;
    }
    else
    {
        return rx_config.context->base->CFG1;
    }
}

uint32_t serdes_i2s_get_cfg2(i2s_bus_t i2s_bus)
{
    if (i2s_bus == TX)
    {
        return tx_config.context->base->CFG2;
    }
    else
    {
        return rx_config.context->base->CFG2;
    }
}

uint32_t serdes_i2s_get_stat(i2s_bus_t i2s_bus)
{
    if (i2s_bus == TX)
    {
        return tx_config.context->base->STAT;
    }
    else
    {
        return rx_config.context->base->STAT;
    }
}


