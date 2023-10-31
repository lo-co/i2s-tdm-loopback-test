/** @file serdes_gpio.h
 *
 * @author Matt Richardson
*/

#ifndef _SERDES_GPIO_H
#define _SERDES_GPIO_H

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

typedef void(*interrupt_cb)(void *usrData);

typedef struct serdes_gpio_cfg_s
{
    interrupt_cb sw2_cb;

} serdes_gpio_cfg_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

void serdes_gpio_init(serdes_gpio_cfg_t cfg_data);

#endif
