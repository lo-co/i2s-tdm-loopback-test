
#include "serdes_defs.h"
#include "serdes_memory.h"
#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief Provide a test pattern for validation
 *
 * First four buffers of the tx buffer will be filled with 1, 2,
 * 3, and 4 in that order.  The remainder are 0s.
 */
void fill_test_pattern();

/*******************************************************************************
 * Variables
 ******************************************************************************/

/**
 * @brief Buffer used for transmitting data on the I2S bus.
 */
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t tx_audio_buffer[BUFFER_NUMBER * BUFFER_SIZE], 4);

/**
 * @brief Buffer used for receiving data
 */
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t rx_audio_buffer[BUFFER_NUMBER * BUFFER_SIZE], 4);

/**
 * @brief Buffer used to queue data requests for transmission on the I2S bus
 */
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t tx_data_buffer[NUMBER_DATA_BUFFERS * DATA_BUFFER_SIZE], 4);

/**
 * @brief Buffer used to store audio received from the codec
 */
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t audio_src_data[NUMBER_CODEC_BUFFERS * BUFFER_SIZE], 4);

/**
 * @brief Position to read from the buffer on a transmit request
 */
static uint8_t s_audio_read_position = 0;

/**
 * @brief Position to write to when placing data into the audio buffer
 */
static uint8_t s_audio_write_position = 0;

/**
 * @brief Position to write to in data buffer
 */
static uint8_t s_data_write_position = 0;

/**
 * @brief Position to read from in the data buffer.
 *
 * Use this index when preparing to transmit data on the bus.
 */
static uint8_t s_data_read_position = 0;

/**
 * @brief Current position to receive data to from the I2S bus
 */
static uint8_t s_audio_rx_position = 0;

/**
 * @brief Current position to receive data to from the I2S bus
 */
static uint8_t s_audio_src_write_position = 0;

/**
 * @brief Current position to receive data to from the I2S bus
 */
static uint8_t s_audio_src_read_position = 0;

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
void serdes_mem_insert_audio_data(uint8_t *buffer, uint32_t length)
{
    uint8_t current_data_idx = 1;

    uint8_t *current_buffer = tx_audio_buffer + s_audio_write_position * BUFFER_SIZE;

    memcpy(current_buffer, buffer, length);

    while (s_data_read_position != s_data_write_position)
    {
        memcpy((current_buffer + BYTES_PER_SAMPLE * 6 * current_data_idx),
                &tx_data_buffer[s_data_read_position++ * DATA_BUFFER_SIZE],
                DATA_BUFFER_SIZE);

        if (s_data_read_position >= NUMBER_DATA_BUFFERS)
        {
            s_data_read_position = 0;
        }
    }

    if (++s_audio_write_position >= BUFFER_NUMBER)
    {
        s_audio_write_position = 0;
    }
}

void serdes_mem_insert_data_data(data_pckt_t data)
{
    if (++s_data_write_position >= NUMBER_DATA_BUFFERS)
    {
        s_data_write_position = 0;
    }
}

// Documentation in .h
uint8_t* serdes_get_next_tx_buffer()
{
    uint8_t *tx_buffer = tx_audio_buffer + s_audio_read_position++ * BUFFER_SIZE;

    if (s_audio_read_position >= BUFFER_NUMBER)
    {
        s_audio_read_position = 0;
    }
    return tx_buffer;

}

// Documentation in .h
uint8_t* serdes_get_next_rx_buffer()
{
    uint8_t *rx_buffer = rx_audio_buffer + s_audio_rx_position++ * BUFFER_SIZE;

    if (s_audio_rx_position >= BUFFER_NUMBER)
    {
        s_audio_rx_position = 0;
    }
    return rx_buffer;
}

// Documentation in .h
uint8_t* serdes_get_next_audio_src_buffer()
{
    uint8_t *rx_buffer = audio_src_data + s_audio_src_write_position++ * BUFFER_SIZE;

    if (s_audio_src_write_position >= NUMBER_CODEC_BUFFERS)
    {
        s_audio_src_write_position = 0;
    }
    return rx_buffer;
}

// Documented in .h
void serdes_memory_init()
{
    memset(tx_audio_buffer, 0, BUFFER_NUMBER * BUFFER_SIZE);
    fill_test_pattern();
    memset(rx_audio_buffer, 0, BUFFER_NUMBER * BUFFER_SIZE);
    memset(tx_data_buffer, 0, NUMBER_DATA_BUFFERS * DATA_BUFFER_SIZE);
    memset(audio_src_data, 0, NUMBER_CODEC_BUFFERS * BUFFER_SIZE);


    // Start the read buffer two positions behind the write.  So, not matter what we have
    // 2 buffers of silence before transmission of data
    s_audio_write_position = 2;
    s_audio_read_position = s_data_read_position = s_data_write_position = s_audio_rx_position = 0;
    s_audio_src_write_position = s_audio_src_read_position = 0;
}

// Documented above
void fill_test_pattern()
{
    memset(tx_audio_buffer, 1, BUFFER_SIZE);
    memset(tx_audio_buffer+BUFFER_SIZE, 2, BUFFER_SIZE);
    memset(tx_audio_buffer+2*BUFFER_SIZE, 3, BUFFER_SIZE);
    memset(tx_audio_buffer+3*BUFFER_SIZE, 4, BUFFER_SIZE);
}
