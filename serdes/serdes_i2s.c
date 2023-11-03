#include "serdes_i2s.h"
#include "fsl_i2c.h"
#include "serdes_defs.h"
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
/** Transmit data if available in the buffer */
void tx_i2s_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *);

/** @brief Receive incoming data and make sure it gets where it needs to */
void rx_i2s_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *);

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

void tx_i2s_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *)
{
    i2s_transfer_t txf = {.data = serdes_get_next_tx_buffer(), .dataSize = BUFFER_SIZE};
    I2S_TxTransferSendDMA(tx_config.context->base, &tx_config.context->i2s_dma_handle, txf);

    // Make sure the buffer is in bounds
    tx_idx = tx_idx >= BUFFER_NUMBER ? 0 : tx_idx;

}

void rx_i2s_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *)
{
    i2s_transfer_t txf = {.data = serdes_get_next_rx_buffer(), .dataSize = BUFFER_SIZE};
    I2S_RxTransferReceiveDMA(rx_config.context->base, &rx_config.context->i2s_dma_handle, txf);

    // Here we will check to see if there is data in one of the info channels

    // Make sure the buffer is in bounds
    rx_idx = rx_idx >= BUFFER_NUMBER ? 0 : rx_idx;
}
