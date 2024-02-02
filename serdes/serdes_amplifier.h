/**
 * @file serdes_amplifier.h
 * @author Matt Richardson(matt.richardson@msrconsults.com)
 * @brief Implementation of the SERDES amplifier
 * @version 0.1
 * @date 2023-11-28
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _SERDES_AMP_H
#define _SERDES_AMP_H

typedef enum serdes_amp_code_e
{
    AMP_SUCCESS = 0,
    CMD_FAILURE,
    PVDD_LOW
} serdes_amp_code_t;

/**
 * @brief Amp initialization
 *
 * Sets up I2C read and write functions and then dumps the configuration registers,
 * then sets those registers with the correct values.  This should be called at boot
 * and before the amp is enabled.
 *
 */
void serdes_amp_init();

/**
 * @brief Enables the amplifier
 *
 * This function should be called _only_ after audio clocks have started.
 *
 * @return Failure if anything but 0
 */
serdes_amp_code_t serdes_amp_start();

/**
 * @brief Disable the amplifier
 *
 */
void serdes_amp_stop();

#endif