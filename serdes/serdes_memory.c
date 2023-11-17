
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

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

void serdes_mem_insert_audio_data(uint8_t *buffer, uint32_t length)
{
    uint64_t *current_buffer = (uint64_t *)(tx_audio_buffer + s_audio_write_position * BUFFER_SIZE);

    // Make sure the buffer is zeroed...
    memset(current_buffer, 0, BUFFER_SIZE);

    uint64_t *incomming_buffer = (uint64_t *)buffer;

    uint16_t buffer_len_64 = BUFFER_SIZE / 8;

    // Make sure all audio channels aside from 0 and 1 are muted and channels 6 and 7 contain
    // data if available...
    for (uint16_t sample_pair = 0; sample_pair < buffer_len_64; )
    {
        if (incomming_buffer != NULL)
        {
            current_buffer[sample_pair] = incomming_buffer[sample_pair];
            sample_pair++;
            // For the moment, fill channels 2-5 with zeros.  Not certain if anything else should go
            // out on these...
            current_buffer[sample_pair++] = 0;
            current_buffer[sample_pair++] = 0;
        }
        else
        {
            sample_pair += 3;
        }
        // If there is data available, place that data in channels 6-7
        if (s_data_read_position != s_data_write_position)
        {
            current_buffer[sample_pair++] = tx_data_buffer[s_data_read_position++];

            if (s_data_read_position >= NUMBER_DATA_BUFFERS)
            {
                s_data_read_position = 0;
            }
        }
        else // No data available - fill 6 and 7 with zeros
        {
            if (incomming_buffer == NULL)
            {
                break;
            }
            current_buffer[sample_pair++] = 0;
        }
    }

    if (++s_audio_write_position >= BUFFER_NUMBER)
    {
        s_audio_write_position = 0;
    }
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
    return MEMORY_STATUS_SUCCESS;
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
    uint8_t *rx_buffer = rx_audio_buffer + s_audio_rx_write_position++ * BUFFER_SIZE;

    memset(rx_buffer, 0, BUFFER_SIZE);

    if (s_audio_rx_write_position >= BUFFER_NUMBER)
    {
        s_audio_rx_write_position = 0;
    }
    return rx_buffer;
}

memory_status_t serdes_mem_get_next_read_buffer(uint8_t **buffer)
{
    if (s_audio_rx_read_position == s_audio_rx_write_position)
    {
        return MEMORY_STATUS_EMPTY;
    }

    *buffer = rx_audio_buffer + s_audio_rx_read_position++ * BUFFER_SIZE;

    if (s_audio_rx_read_position >= BUFFER_NUMBER)
    {
        s_audio_rx_read_position = 0;
    }
    return MEMORY_STATUS_SUCCESS;
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
    // fill_test_pattern();
    memset(rx_audio_buffer, 0, BUFFER_NUMBER * BUFFER_SIZE);
    memset(tx_data_buffer, 0, NUMBER_DATA_BUFFERS * DATA_BUFFER_SIZE);
    memset(audio_src_data, 0, NUMBER_CODEC_BUFFERS * BUFFER_SIZE);


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
        uint32_t buffer_size = NUM_CHANNELS * BUFFER_SIZE / 4;

        // Set teh buffer to zero and fill it with the tonein channels
        // 0 and 1.
        memset(tx_buffer, 0, buffer_size);
        for (uint32_t i = 0, j = 0; i < buffer_size; )
        {
            *(tx_buffer + i++) = tone_input[j];
            *(tx_buffer + i++) = tone_input[j++];
            j = j >=48 ? 0 : j;
            i += 2;
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
