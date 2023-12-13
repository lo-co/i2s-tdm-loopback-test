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

static uint8_t zero_buffer[SLAVE_BUFFER_SIZE] = {0};


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
                        .share_clk = true, .shared_clk_set = SHARED_SET_1, .active_ch_pairs = {PAIR_0, PAIR_3, PAIR_MAX, PAIR_MAX},
                        .enable_pd = false};
static i2s_context_t rx_i2s_context;
// Receive configuration is always a slave.  In the case of the MASTER NXP device, it is a slave to the transmit.  It will use the internal
// clock output of the shared signal.
static i2s_init_t rx_config = {.flexcomm_bus = FLEXCOMM_5, .is_transmit = false,
                        .is_master = false, .active_channels = NUM_CHANNELS,
                        .sample_rate = SAMPLE_RATE_HZ, .datalength = DATA_LEN_BITS,
                        .callback = rx_i2s_cb, .context = &rx_i2s_context,
                        .share_clk = false, .shared_clk_set = SHARED_SET_1, .active_ch_pairs = {PAIR_1, PAIR_2, PAIR_MAX, PAIR_MAX},
                        .enable_pd = false};

static uint8_t *rx_buffer = 0;

/**
 * @brief Data packet for incoming data
 */
static data_pckt_t data_pckt = {.src = SRC_NUM, .data_length = 0, .data = 0x0};

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

// Documented in .h
void serdes_i2s_init(bool is_master)
{
    if (!is_master)
    {
        const active_ch_pair_t ch_pairs[4] = {PAIR_1, PAIR_MAX, PAIR_MAX, PAIR_MAX};
        tx_config.flexcomm_bus = FLEXCOMM_5;
        tx_config.is_master = false;
        tx_config.shared_clk_set = NO_SHARE;
        tx_config.share_clk = false;

        rx_config.flexcomm_bus = FLEXCOMM_4;
        rx_config.shared_clk_set = NO_SHARE;

        for (uint8_t idx = 0; idx < 4; idx++)
        {
            tx_config.active_ch_pairs[idx] = ch_pairs[idx];
            rx_config.active_ch_pairs[idx] = ch_pairs[idx];
        }
    } else
    {
        tx_config.share_clk = true;
        tx_config.shared_clk_set = SHARED_SET_1;
        // rx_config.enable_pd = true;
        rx_config.shared_clk_set = SHARED_SET_1;
    }

    // Make sure the buffers contain nothing...
    serdes_memory_init(is_master);

    i2s_init(tx_config);
    i2s_init(rx_config);
    serdes_register_handler(DATA_RECEIVED, i2s_rx_event_cb);

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
    i2s_transfer_t txf = {.dataSize = SLAVE_BUFFER_SIZE};

    for (uint8_t prebuffer = 0; prebuffer < max_prebuffers; prebuffer++)
    {
        txf.data = serdes_get_next_rx_buffer();
        I2S_RxTransferReceiveDMA(rx_config.context->base, &rx_config.context->i2s_dma_handle, txf);

        // ALWAYS send zeros...the amp output is high impedance so someone needs to drive
        txf.data = zero_buffer;
        I2S_TxTransferSendDMA(tx_config.context->base, &tx_config.context->i2s_dma_handle, txf);
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
static void tx_i2s_cb(I2S_Type *,i2s_dma_handle_t *,status_t status,void *)
{
    // For the master, we assume that while transmitting there is always more data.
    // This allows for testing of the connection using something such as fill_memory
    // without constantly having to replenish the buffer explicitly.
    // TODO: this should be changed in the future so that no stale buffers are sent.
    if (tx_config.is_master || serdes_memory_more_audio_data())
    {
        i2s_transfer_t txf = {.data = serdes_get_next_tx_buffer(), .dataSize = tx_config.is_master ? MASTER_BUFFER_SIZE : SLAVE_BUFFER_SIZE};
        I2S_TxTransferSendDMA(tx_config.context->base, &tx_config.context->i2s_dma_handle, txf);
    }
    else if (!tx_config.is_master) {
        i2s_transfer_t txf = {.data = zero_buffer, .dataSize = SLAVE_BUFFER_SIZE};
        I2S_TxTransferSendDMA(tx_config.context->base, &tx_config.context->i2s_dma_handle, txf);

    }
}

// Documented above
static void rx_i2s_cb(I2S_Type *,i2s_dma_handle_t *,status_t ,void *)
{
    i2s_transfer_t txf = {.data = serdes_get_next_rx_buffer(), .dataSize = rx_config.is_master ? MASTER_BUFFER_SIZE : SLAVE_BUFFER_SIZE};
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
    uint8_t* buffer = (uint8_t *)usrData;

    for (uint32_t slot = 0; slot < MASTER_BUFFER_SIZE;)
    {
        uint64_t slot_data = *(uint64_t *)(buffer+slot);
        // This signals a start to the data packet

        if ((slot_data & 0xF) == 0xD)
        {
            protocom_status_t status = serdes_protocom_deserialize_data(slot_data, &data_pckt);
            if (status == PROTOCOM_SUCCESS)
            {
                serdes_push_event(HANDLE_DATA_RECEIVED, &data_pckt);
            }

        }
        slot += 16;
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
