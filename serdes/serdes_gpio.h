/**
 * @file serdes_gpio.h
 * @author Matt Richardson (mattrichardson@meta.com)
 * @brief
 * @version 0.1
 * @date 2023-12-13
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _SERDES_GPIO_H
#define _SERDES_GPIO_H

#include "../drivers/gpio/gpio_def.h"
#include "serdes_event.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
typedef struct led_state_s
{
    led_color_t color;
    bool set_on;
} led_state_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief Initialize SERDES specific GPIO
 *
 * @return true Initialization successful
 * @return false Initialization failed
 */
bool serdes_gpio_init();

/**
 * @brief Set the LED state for LED led
 *
 * @param led LED to set the state for
 * @param is_on True will turn on the LED, False will turn it off
 */
void serdes_set_led_state (led_color_t led, bool is_on);

#endif
