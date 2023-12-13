
#include "serdes_defs.h"
#include "serdes_event.h"
#include "serdes_memory.h"
#include "serdes_utilities.h"
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
void fill_test_pattern(bool tone);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* The following variables define the pools of memory available to the different devices.
*/
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
AT_NONCACHEABLE_SECTION_ALIGN(static uint64_t tx_data_buffer[NUMBER_DATA_BUFFERS], 4);

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
static uint8_t s_audio_rx_write_position = 0;

static uint8_t s_audio_rx_read_position = 0;

/**
 * @brief Current position to receive data to from the I2S bus
 */
static uint8_t s_audio_src_write_position = 0;

/**
 * @brief Current position to receive data to from the I2S bus
 */
static uint8_t s_audio_src_read_position = 0;

static bool data_memory_full = false;

/**
 * @brief This is the stored value of the transmit buffer size
 *
 */
static uint32_t tx_buffer_size = 0;

/**
 * @brief Stored value of the receive buffer size
 *
 */
static uint32_t rx_buffer_size = 0;

/**
 * @brief Stored value of the board role.
 *
 */
static bool sys_is_master = false;

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

// FIXME: THIS IS CURRENTLY ONLY USED ON THE SLAVE..
void serdes_mem_insert_audio_data(uint8_t *buffer, uint32_t length)
{
    // The ISR which utilizes the position can create a race condition.  Write to the new position
    // and then once the write is complete, update the position. The way this was done previously
    // meant that the data at that position would never go out
    uint8_t temp_write_position = s_audio_write_position + 1 >= BUFFER_NUMBER ? 0 : s_audio_write_position + 1;
    uint64_t *current_buffer = (uint64_t *)(tx_audio_buffer + temp_write_position * tx_buffer_size);

    // Make sure the buffer is zeroed...
    memset(current_buffer, 0, tx_buffer_size);

    uint16_t buffer_len_64 = tx_buffer_size / 8;

    for (uint16_t sample_pair = 0; sample_pair < buffer_len_64; )
    {
       // If there is data available, place that data in channels 6-7
        if (s_data_read_position != s_data_write_position)
        {
            // Place the data in the data buffer in the transmit buffer
            current_buffer[sample_pair++] = tx_data_buffer[s_data_read_position++];
            if (s_data_read_position >= NUMBER_DATA_BUFFERS)
            {
                s_data_read_position = 0;
            }
        }
        else
        {
            break;
        }
    }

   s_audio_write_position = temp_write_position;
}

// TODO: Fill out this function so data is inserted properly
// Will have to define a protocol for this
// TODO: report to user if they try to insert data when no space available
memory_status_t serdes_mem_insert_data_data(uint64_t data)
{
    if (data_memory_full)
    {
        return MEMORY_STATUS_FULL;
    }

    tx_data_buffer[s_data_write_position++] = data;

    data_memory_full = s_data_write_position == s_data_read_position;

    if (s_data_write_position >= NUMBER_DATA_BUFFERS)
    {
        s_data_write_position = 0;
    }

    return MEMORY_STATUS_SUCCESS;
}

// Documentation in .h
uint8_t* serdes_get_next_tx_buffer()
{
    uint8_t *tx_buffer = tx_audio_buffer + s_audio_read_position++ * tx_buffer_size;

    if (s_audio_read_position >= BUFFER_NUMBER)
    {
        s_audio_read_position = 0;
    }
    return tx_buffer;
}

// Documentation in .h
uint8_t* serdes_get_next_rx_buffer()
{
    uint8_t *rx_buffer = rx_audio_buffer + s_audio_rx_write_position++ * rx_buffer_size;

    memset(rx_buffer, 0, rx_buffer_size);

    if (s_audio_rx_write_position >= BUFFER_NUMBER)
    {
        s_audio_rx_write_position = 0;
    }
    return rx_buffer;
}

// Documentation in .h
memory_status_t serdes_mem_get_next_read_buffer(uint8_t **buffer)
{
    if (s_audio_rx_read_position == s_audio_rx_write_position)
    {
        return MEMORY_STATUS_EMPTY;
    }

    *buffer = rx_audio_buffer + s_audio_rx_read_position++ * rx_buffer_size;

    if (s_audio_rx_read_position >= BUFFER_NUMBER)
    {
        s_audio_rx_read_position = 0;
    }
    return MEMORY_STATUS_SUCCESS;
}

// Documentation in .h
uint8_t* serdes_get_next_audio_src_buffer()
{
    uint8_t *rx_buffer = audio_src_data + s_audio_src_write_position++ * AUDIO_SRC_BUFFER_SIZE;

    if (s_audio_src_write_position >= NUMBER_CODEC_BUFFERS)
    {
        s_audio_src_write_position = 0;
    }
    return rx_buffer;
}

// Documented in .h
void serdes_memory_init(bool is_master)
{
    sys_is_master = is_master;
    tx_buffer_size = is_master ? MASTER_BUFFER_SIZE : SLAVE_BUFFER_SIZE;
    rx_buffer_size = is_master ? MASTER_BUFFER_SIZE : SLAVE_BUFFER_SIZE;

    memset(tx_audio_buffer, 0, BUFFER_NUMBER * tx_buffer_size);
    fill_test_pattern(true);
    memset(rx_audio_buffer, 0, BUFFER_NUMBER * rx_buffer_size);
    memset(tx_data_buffer, 0, NUMBER_DATA_BUFFERS * DATA_BUFFER_SIZE);
    memset(audio_src_data, 0, NUMBER_CODEC_BUFFERS * AUDIO_SRC_BUFFER_SIZE);

    // Start the read buffer two positions behind the write.  So, not matter what we have
    // 2 buffers of silence before transmission of data
    s_audio_write_position = 2;
    s_audio_read_position = s_data_read_position = s_data_write_position = 0;
    s_audio_rx_read_position = s_audio_rx_write_position = 0;
    s_audio_src_write_position = s_audio_src_read_position = 0;
}

// Documented in .h
bool serdes_memory_data_ready()
{
    return s_data_read_position != s_data_write_position;
}

bool serdes_memory_more_audio_data()
{
    return s_audio_read_position != s_audio_write_position;
}

// Documented above
void fill_test_pattern(bool tone)
{
    if (tone)
    {
        int32_t tone_input[48] = {0};
        generate_tone(tone_input, 48, 50);
        uint32_t *tx_buffer = (uint32_t *)tx_audio_buffer;
        uint32_t buffer_size = NUM_CHANNELS * tx_buffer_size / 4;

        // Set the buffer to zero and fill it with the tonein channels
        // 0 and 1.
        for (uint32_t i = 0, j = 0; i < buffer_size; )
        {
            *(tx_buffer + i++) = tone_input[j];
            *(tx_buffer + i) = tone_input[j++];
            j = j >= 48 ? 0 : j;
            // if (i > 1)
            // {
            i += 3;
            // }
        }
        for (uint8_t idx = 1; idx < BUFFER_NUMBER; idx++)
        {
            memcpy(tx_audio_buffer+tx_buffer_size * idx, tx_audio_buffer, tx_buffer_size);
        }
    }
    else
    {
        memset(tx_audio_buffer, 1, BUFFER_SIZE);
        memset(tx_audio_buffer+BUFFER_SIZE, 2, BUFFER_SIZE);
        memset(tx_audio_buffer+2*BUFFER_SIZE, 3, BUFFER_SIZE);
        memset(tx_audio_buffer+3*BUFFER_SIZE, 4, BUFFER_SIZE);
    }
}
