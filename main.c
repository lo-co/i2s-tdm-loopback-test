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
#include "serdes/serdes_i2s.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef BOARD_IS_MASTER
#define BOARD_IS_MASTER 1
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

uint8_t sw2_int_cb(void *usrData);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static led_state_t led_state = {.color = GREEN, .set_on = true};

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
    serdes_register_handler(SWITCH_2_PRESSED, sw2_int_cb);
    serdes_gpio_init();
    serdes_i2s_init(BOARD_IS_MASTER);
    CLOCK_EnableClock(kCLOCK_InputMux);
    PRINTF("SERDES main application starting...\r\n");
    /********** END INITIALIZATION *************/

    if (BOARD_IS_MASTER)
    {
        PRINTF("Board is master\r\n");
        serdes_codec_init();

        // initialize amps and mics
    }

    // Kick off main loop...
    while (true)
    {
        // Call the event dispatcher and handle any incoming events...
        serdes_dispatch_event();
    }
}

uint8_t sw2_int_cb(void *usrData)
{
    (void)usrData;
    if (!serdes_i2s_is_running())
    {
        led_state.set_on = true;
        PRINTF("Starting I2S bus...\r\n");
        serdes_i2s_start();
        if (BOARD_IS_MASTER)
        {
            serdes_codec_src_start();
        }
    }
    else
    {
        led_state.set_on = false;
        if (BOARD_IS_MASTER)
        {
            serdes_codec_src_stop();
        }
        serdes_i2s_stop();
        PRINTF("Stopping I2S transmission now..\r\n");
    }
    serdes_push_event(SET_LED_STATE, &led_state);
    return 0;
}

