/** @file main.c
 *
 *
 * ## Validation of master
 * * Debug console up and running (11/2)
 * * Switch interrupt functional (11/2)
 * * I2S callable and firing (11/2)
 * * I2S data received - validated using 1,2,3,4 for first 4 buffers (11/2)
 *
 *
 * TODO:
 * * add LED for system
 * * add amp for system
 *
 * @author Matt Richardson
*/

#include <stdio.h>
#include "board.h"
#include "pin_mux.h"
#include "fsl_debug_console.h"
#include "serdes/serdes_codec.h"
#include "serdes/serdes_defs.h"
#include "serdes/serdes_event.h"
#include "serdes/serdes_gpio.h"
#include "serdes/serdes_amplifier.h"
#include "serdes/serdes_i2s.h"
#include "serdes/serdes_memory.h"
#include "serdes/serdes_protocom.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/**
 * @brief Event handler for pressing switch 2.
 *
 * Switch two functionality is defined only for the master.  When the switch two
 * button is pressed, the I2S bridge will be activated.  When the bridge is
 * activated, the green LED will light up.
 *
 * @param usrData UNUSED for the master
 * @return 0 on success
 */
static uint8_t sw2_int_cb(void *usrData);

static uint8_t wakeup_cb(void *usrData);

static uint8_t enter_deep_sleep_cb(void *usrData);

/**
 * @brief Handle the serialization of data for transfer across the I2S bridge
 *
 * Once data is serialized, flag that data is available for transmit via
 * the DATA_AVAILABLE event.
 *
 * @param usrData data_pckt_t structure of data to be serialized
 * @return MEMORY_STATUS_FULL if buffer is full
 * @return 0 if success
 */
static uint8_t data_handler(void *usrData);

/**
 * @brief Handle request sent across the bridge
 *
 * This will fire an event representing  the action requested that came
 * across the bridge.
 *
 * @param usrData data_pckt_t structure of data to be acted on
 * @return uint8_t
 */
static uint8_t process_data_received(void *usrData);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static led_state_t led_state = {.color = GREEN, .set_on = true};

/** Determines whether the boad is master or not.
 *
 * This is handled by GPIO
*/
static bool is_master = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    /********** BEGIN INITIALIZATION *************/
    BOARD_InitBootClocks();
    BOARD_InitBootPins();
    BOARD_InitDebugConsole();
    serdes_event_init();
    serdes_register_handler(WAKEUP, wakeup_cb);
    serdes_register_handler(ENTER_DEEP_SLEEP, enter_deep_sleep_cb);
    serdes_register_handler(INSERT_DATA, data_handler);
    serdes_register_handler(SWITCH_2_PRESSED, sw2_int_cb);
    serdes_register_handler(HANDLE_DATA_RECEIVED, process_data_received);
    is_master = serdes_gpio_init();
    serdes_i2s_init(is_master);

    CLOCK_EnableClock(kCLOCK_InputMux);

    PRINTF("SERDES main application starting...\r\n");
    /********** END INITIALIZATION *************/

    if (is_master)
    {
        PRINTF("Board is master\r\n");
        serdes_amp_init();
        serdes_codec_init();
    }
    else {
        PRINTF("Board is slave\r\n");
        led_state.set_on = true;
        led_state.color = BLUE;
        serdes_push_event(SET_LED_STATE, &led_state);
    }

    // Kick off main loop...
    while (true)
    {
        // Call the event dispatcher and handle any incoming events...
        serdes_dispatch_event();

        if (!is_master && serdes_memory_data_ready())
        {
            // Transmit I2S here
        }
    }
}

// Documented boave
static uint8_t sw2_int_cb(void *usrData)
{
    if (is_master)
    {
        (void)usrData;
        if (!serdes_i2s_is_running())
        {
            led_state.set_on = true;
            led_state.color = GREEN;
            PRINTF("Starting I2S bus...\r\n");
            serdes_i2s_start();
            // FIXME: Issue with codec data not properly inserted into data channels.
            // For now we will fill with a tone for testing
            // serdes_codec_src_start();
            serdes_amp_start();
        }
        else
        {
            led_state.set_on = false;
            led_state.color = GREEN;

            serdes_amp_stop();
            serdes_i2s_stop();
            // serdes_codec_src_stop();

            PRINTF("Stopping I2S transmission now..\r\n");
        }
        serdes_push_event(SET_LED_STATE, &led_state);
    }
    return 0;
}

static uint8_t wakeup_cb(void *usrData)
{
    uint32_t power_state = *(uint32_t *)usrData;
    PRINTF("System is awake.  Current power state is 0x%X\r\n", power_state);
    return 0;
}

static uint8_t enter_deep_sleep_cb(void *usrData)
{
    PRINTF("Entering deep sleep...\r\n");
    return 0;
}

// Documented above
static uint8_t data_handler(void *usrData)
{
    data_pckt_t data_pckt = *(data_pckt_t *)usrData;

    memory_status_t status = serdes_mem_insert_data_data(serdes_protocom_serialize_data(data_pckt));

    // If the board is not actively transmitting then we are sending 0s;
    // we will then increment the buffer and make sure the data sits in the
    // buffer.  Here is an issue - what if we want to bundle.
    // TODO: Handle bundling of requests
    if (!is_master)
    {
        serdes_mem_insert_audio_data(NULL, BUFFER_SIZE);
    }
    if (MEMORY_STATUS_SUCCESS == status)
    {
        serdes_push_event(DATA_AVAILABLE, NULL);
    }

    return status;
}

// Documented above
static uint8_t process_data_received(void *usrData)
{
    data_pckt_t data_pckt = *(data_pckt_t *)usrData;

    switch (data_pckt.src)
    {
        case SRC_GPIO:
            led_state.color = BLUE;
            led_state.set_on = *data_pckt.data == 0xFF;
            serdes_push_event(SET_LED_STATE, &led_state);
            break;

        default:
            PRINTF("DATA NOT HANDLED\r\n");
            break;
    }

    return 0;
}
