/** @file serdes_gpio.h
 *
 * @author Matt Richardson
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

bool serdes_gpio_init();

void serdes_set_led_state (led_color_t led, bool is_on);

#endif
