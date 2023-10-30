#include "serdes_i2s.h"

#include "fsl_i2c.h"

#define SAMPLE_SIZE_MS (8U)
#define SAMPLE_RATE_HZ (48000)
#define SAMPLE_PER_MS (SAMPLE_RATE_HZ / 1000)
#define BYTES_PER_SAMPLE (4)
#define NUMBER_TDM_CH  (8)
#define BUFFER_SIZE   (SAMPLE_SIZE_MS * SAMPLE_PER_MS * BYTES_PER_SAMPLE * NUMBER_TDM_CH)
#define BUFFER_NUMBER (8U)

#define DATA_LEN_BITS  (BYTES_PER_SAMPLE * 8U)

/** Transmit data if available in the buffer */
void tx_i2s_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *);

/** @brief Receive incoming data and make sure it gets where it needs to */
void rx_i2s_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *);

uint8_t tx_idx = 0, rx_idx = 0;

/* Memory set aside for receive and transmit transactions */
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t rx_buffer[BUFFER_NUMBER * BUFFER_SIZE], 4);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t tx_buffer[BUFFER_NUMBER * BUFFER_SIZE], 4);

// Bridge I2S
i2s_context_t tx_i2s_context;
i2s_init_t tx_config = {.flexcomm_bus = FLEXCOMM_4, .is_transmit = true,
                        .is_master = true, .active_channels = NUMBER_TDM_CH,
                        .sample_rate = SAMPLE_RATE_HZ, .datalength = DATA_LEN_BITS,
                        .callback = tx_i2s_cb, .context = &tx_i2s_context,
                        .share_clk = false, .shared_clk_set = NO_SHARE};
i2s_context_t rx_i2s_context;
i2s_init_t rx_config = {.flexcomm_bus = FLEXCOMM_5, .is_transmit = false,
                        .is_master = true, .active_channels = NUMBER_TDM_CH,
                        .sample_rate = SAMPLE_RATE_HZ, .datalength = DATA_LEN_BITS,
                        .callback = rx_i2s_cb, .context = &rx_i2s_context,
                        .share_clk = false, .shared_clk_set = NO_SHARE};

void serdes_i2s_init(bool is_slave)
{
    if (is_slave)
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
    memset(rx_buffer, 0, BUFFER_NUMBER * BUFFER_SIZE);
    memset(tx_buffer, 0, BUFFER_NUMBER * BUFFER_SIZE);

    i2s_init(tx_config);
    i2s_init(rx_config);
}

void serdes_i2s_start()
{

}

void tx_i2s_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *)
{
    i2s_transfer_t txf = {.data = tx_buffer + tx_idx++*BUFFER_SIZE, .dataSize = BUFFER_SIZE};
    I2S_TxTransferSendDMA(tx_config.context->base, &tx_config.context->i2s_dma_handle, txf);

    // Make sure the buffer is in bounds
    tx_idx = tx_idx >= BUFFER_NUMBER ? 0 : tx_idx;

}

void rx_i2s_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *)
{
    i2s_transfer_t txf = {.data = rx_buffer + rx_idx++*BUFFER_SIZE, .dataSize = BUFFER_SIZE};
    I2S_RxTransferReceiveDMA(rx_config.context->base, &rx_config.context->i2s_dma_handle, txf);

    // Make sure the buffer is in bounds
    rx_idx = rx_idx >= BUFFER_NUMBER ? 0 : rx_idx;
}
