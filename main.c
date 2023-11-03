/** @file main.c
 *
 *
 * ## Validation of master
 * * Debug console up and running (11/2)
 * * Switch interrupt functional (11/2)
 * * I2S callable and firing (11/2)
 * * I2S data received - validated using 1,2,3,4 for first 4 buffers (11/2)
 * @author Matt Richardson
*/

#include <stdio.h>
#include "board.h"
#include "pin_mux.h"
#include "fsl_debug_console.h"
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

void sw2_int_cb(void *usrData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static serdes_gpio_cfg_t gpio_cfg = {.sw2_cb = sw2_int_cb};
static event_t current_events = {.idx = 0, .evt_times = {0}};
static bool g_interruptEnabled = false;

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
    serdes_gpio_init(gpio_cfg);
    serdes_i2s_init(BOARD_IS_MASTER);
    CLOCK_EnableClock(kCLOCK_InputMux);
    PRINTF("SERDES main application starting...\r\n");
    /********** END INITIALIZATION *************/

    if (BOARD_IS_MASTER)
    {
        PRINTF("Board is master\r\n");
        // setup codec as an audio source
        // initialize amps and mics
    }


    // Kick off main loop...
    // TODO: replace with event handler to handle incoming messages from
    // peripherals and system
    while (true)
    {
        if (g_interruptEnabled)
        {
            PRINTF("SW2 interrupt fired...\r\n");
            if (!serdes_i2s_is_running())
            {
                PRINTF("Starting I2S bus...\r\n");
                serdes_i2s_start();

            }
            else
            {
                serdes_i2s_stop();
                PRINTF("Stopping I2S transmission now..\r\n");
            }
            g_interruptEnabled = false;
        }
    }
}

void sw2_int_cb(void *usrData)
{
    (void)usrData;
    serdes_update_evt_times(RCV_INTERRUPT, &current_events);
    g_interruptEnabled = !g_interruptEnabled;
}

