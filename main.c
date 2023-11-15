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
#include "serdes/serdes_memory.h"
#include "serdes/serdes_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef BOARD_IS_MASTER
#define BOARD_IS_MASTER 0
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static uint8_t sw2_int_cb(void *usrData);

static uint8_t wakeup_cb(void *usrData);

static uint8_t enter_deep_sleep_cb(void *usrData);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static led_state_t led_state = {.color = GREEN, .set_on = true};
static uint8_t button_press_data = 0xFF;
uint32_t mem[1024] = {0};

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
    serdes_register_handler(SWITCH_2_PRESSED, sw2_int_cb);
    serdes_gpio_init();
    serdes_i2s_init(BOARD_IS_MASTER);
    CLOCK_EnableClock(kCLOCK_InputMux);
    PRINTF("SERDES main application starting...\r\n");
    /********** END INITIALIZATION *************/

    if (BOARD_IS_MASTER)
    {
        PRINTF("Board is master\r\n");
        // serdes_codec_init();

        // initialize amps and mics
    }
    else {
        PRINTF("Board is slave\r\n");
    }

    // Kick off main loop...
    while (true)
    {
        // Call the event dispatcher and handle any incoming events...
        serdes_dispatch_event();

        if (!BOARD_IS_MASTER && serdes_memory_data_ready())
        {
            // Transmit I2S here

        }
    }
}

static uint8_t sw2_int_cb(void *usrData)
{
    if (BOARD_IS_MASTER)
    {
        (void)usrData;
        if (!serdes_i2s_is_running())
        {
            led_state.set_on = true;
            PRINTF("Starting I2S bus...\r\n");
            serdes_i2s_start();
            if (BOARD_IS_MASTER)
            {
                // serdes_codec_src_start();
            }
        }
        else
        {
            led_state.set_on = false;
            if (BOARD_IS_MASTER)
            {
                // serdes_codec_src_stop();
            }
            serdes_i2s_stop();
            PRINTF("Stopping I2S transmission now..\r\n");
        }
        serdes_push_event(SET_LED_STATE, &led_state);
    }
    else // BOARD is slave...
    {
        const int mem_len = 1024;
        uint64_t *times = serdes_i2s_rx_times();

        memcpy(mem, serdes_get_last_rx_buffer(), mem_len);
        PRINTF("RX FIFOCFG:\t0x%X\r\n", serdes_i2s_get_fifo_config(RX));
        PRINTF("RX FIFOSTAT:\t0x%X\r\n", serdes_i2s_get_fifo_status(RX));
        PRINTF("RX CFG1:\t0x%X\r\n", serdes_i2s_get_cfg1(RX));
        PRINTF("RX CFG2:\t0x%X\r\n", serdes_i2s_get_cfg2(RX));
        PRINTF("RX STAT:\t0x%X\r\n", serdes_i2s_get_stat(RX));
        PRINTF("RX Err Cnt: %d\r\n", serdes_i2s_get_err_cnt());
        PRINTF("RX Times: \r\n");
        for (int i = 0; i < 125; i++)
        {
            PRINTF("%d:\t%d\r\n", i, times[i]);

        }

        // TODO: probably shove this off to GPIO or mem module...for now play away...
        static data_pckt_t data = {.src = SRC_GPIO, .data = &button_press_data, .data_length = 1};
        // serdes_mem_insert_data_data(data);
        // serdes_push_event(INSERT_DATA, &data);
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
