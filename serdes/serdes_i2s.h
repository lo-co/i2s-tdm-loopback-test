/** @file serdes_i2s.h
 *
 * This module is intended to define the functionality for the bridge I2S.
 * The bridge I2S are the receive and transmit lines between the two
 * evaluation boards - FC4 and FC5.
 *
 * @author Matt Richardson
 * @date   October 27, 2023
 *
 *
*/

#ifndef _SERDES_I2S_H
#define _SERDES_I2S_H
#include "../drivers/i2s/i2s.h"

/** Initialize the two I2S buses used for bridge communication.
 *
 * This will initialize FC4 and FC5 for communication between boards depending
 * on the role of the board.  The roles are defined as follows:
 *
 * | Bus | MASTER | SLAVE |
 * | FC4 |   RX   |   TX  |
 * | FC5 |   TX   |   RX  |
 *
 * If the role is as master, then the tx clock will be shared with the receive
 * clock lines using SHARED_SET_1. If the role is slave, then both buses will be
 * configured as clock slaves.
 *
 * @param is_master Boolean defining the role of the board.  Roles are master (true)
 *                 and slave (false)
*/
void serdes_i2s_init(bool is_master);

/**
 * @brief Stop all DMA transfers on the SERDES bridge and disable I2S
 *
 */
void serdes_i2s_stop();

/**
 * @brief Start receive and transmit transactions.
 *
 * If the bus is master, it will start the clocks.
 *
 */
void serdes_i2s_start();

/**
 * @brief Boolean determining if the I2S bus is running
 *
 * @return true Bus is running
 * @return false Bus is not running
 */
bool serdes_i2s_is_running();

#endif