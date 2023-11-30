#include "serdes_i2s.h"
#include "serdes_defs.h"
#include "serdes_event.h"
#include "serdes_memory.h"
#include "serdes_protocom.h"
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

/**
 * @brief Event handler for indicating that there is outgoing data avaialble
 *
 * This function is used only by the slave.  Generally, the slave is not transmitting.
 * However, when the TX line is idle, the slave will fill an audio buffer with data
 * and raise the flag to indicate that the audio buffer needs to be sent.  Once a
 * transfer is triggered, the transfer will continue until the read and write positions
 * catch up to each other.
 *
 * @param usrData UNUSED but required for the event handler
 * @return Success
 */
static uint8_t i2s_tx_data_request(void *usrData);


/*******************************************************************************
 * Variables
 ******************************************************************************/

/**
 * @brief Variable indicating whether the I2S (tx and rcv) is active
 */
static bool bus_is_running = false;

// Bridge I2S
static i2s_context_t tx_i2s_context;
static i2s_init_t tx_config = {.flexcomm_bus = FLEXCOMM_4, .is_transmit = true,
                        .is_master = true, .active_channels = NUM_CHANNELS,
                        .sample_rate = SAMPLE_RATE_HZ, .datalength = DATA_LEN_BITS,
                        .callback = tx_i2s_cb, .context = &tx_i2s_context,
                        .share_clk = false, .shared_clk_set = NO_SHARE};
static i2s_context_t rx_i2s_context;
static i2s_init_t rx_config = {.flexcomm_bus = FLEXCOMM_5, .is_transmit = false,
                        .is_master = true, .active_channels = NUM_CHANNELS,
                        .sample_rate = SAMPLE_RATE_HZ, .datalength = DATA_LEN_BITS,
                        .callback = rx_i2s_cb, .context = &rx_i2s_context,
                        .share_clk = false, .shared_clk_set = NO_SHARE};

static uint8_t *rx_buffer = 0;

// FIXME: we won't overwhelm the command handler, but what if we did?
static uint8_t data[2] = {0};

/**
 * @brief Data packet for incoming data
 */
static data_pckt_t data_pckt = {.src = SRC_NUM, .data_length = 0, .data = data};

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
    const uint8_t max_prebuffers = 2;

    bus_is_running = true;

    // In the future, this should send 0's maybe?
    // Kick the transmit  and receive- make sure that the transmit buffer never runs dry for I2S
    i2s_transfer_t txf = {.dataSize = BUFFER_SIZE};
    for (uint8_t prebuffer = 0; prebuffer < max_prebuffers; prebuffer++)
    {
        txf.data = serdes_get_next_tx_buffer();
        I2S_TxTransferSendDMA(tx_config.context->base, &tx_config.context->i2s_dma_handle, txf);

        txf.data = serdes_get_next_rx_buffer();
        I2S_RxTransferReceiveDMA(rx_config.context->base, &rx_config.context->i2s_dma_handle, txf);
    }
}

// Documented in .h
void serdes_i2s_start_slave()
{
    const uint8_t max_prebuffers = 2;
    // Kick start the receive buffers - this will cause at least an 8 ms latency in the receive
    // unless we shorten the first buffer and just chuck it...
    bus_is_running = true;
    i2s_transfer_t txf = {.dataSize = BUFFER_SIZE};

    for (uint8_t prebuffer = 0; prebuffer < max_prebuffers; prebuffer++)
    {
        txf.data = serdes_get_next_rx_buffer();
        I2S_RxTransferReceiveDMA(rx_config.context->base, &rx_config.context->i2s_dma_handle, txf);

    }
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
    // For the master, we assume that while transmitting there is always more data.
    // This allows for testing of the connection using something such as fill_memory
    // without constantly having to replenish the buffer explicitly.
    // TODO: this should be changed in the future so that no stale buffers are sent.
    if (tx_config.is_master || serdes_memory_more_audio_data())
    {
        i2s_transfer_t txf = {.data = serdes_get_next_tx_buffer(), .dataSize = BUFFER_SIZE};
        I2S_TxTransferSendDMA(tx_config.context->base, &tx_config.context->i2s_dma_handle, txf);
    }
}

// Documented above
static void rx_i2s_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *)
{
    i2s_transfer_t txf = {.data = serdes_get_next_rx_buffer(), .dataSize = BUFFER_SIZE};
    I2S_RxTransferReceiveDMA(rx_config.context->base, &rx_config.context->i2s_dma_handle, txf);

    if (!serdes_mem_get_next_read_buffer(&rx_buffer)) {
        serdes_push_event(DATA_RECEIVED, rx_buffer);
    }

    // Clear any slave errors...
    if (rx_config.context->base->STAT & 0x2)
    {
        rx_config.context->base->STAT = 0x2;
    }
}

// Documented above
uint8_t i2s_rx_event_cb(void *usrData)
{
    (void)usrData;

    // TODO: remove this when done testing
    if (rx_config.is_master){

        uint8_t* buffer = (uint8_t *)usrData;

        // Data that is transmitted by the slave will be found starting at DATA_CHANNEL_START
        const uint32_t data_start = BYTES_PER_SAMPLE * CHANNEL_PAIR * DATA_CHANNEL_START;
        const uint32_t increment = BYTES_PER_SAMPLE * NUM_CHANNELS;
        for (uint32_t slot = data_start; slot < BUFFER_SIZE;)
        {
            // This signals a start to the data packet
            if (*(buffer+slot) == 0x1)
            {
                memset(data, 0, sizeof(data));
                serdes_protocom_deserialize_data(buffer+slot, &data_pckt);
                serdes_push_event(HANDLE_DATA_RECEIVED, &data_pckt);
            }
            slot += increment;
        }
    }
    // TODO: is there any reason that data might show up farther down the line than the first packet?
    return 0;
}

// Documented above
static uint8_t i2s_tx_data_request(void *usrData)
{
    (void)usrData;
    i2s_transfer_t txf = {.data = serdes_get_next_tx_buffer(), .dataSize = BUFFER_SIZE};
    I2S_TxTransferSendDMA(tx_config.context->base, &tx_config.context->i2s_dma_handle, txf);

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
