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

typedef struct fifostat_s{
    uint8_t err;
    bool empty;
    bool notfull;
    uint8_t lvl;
} fifostat_t;

typedef enum i2s_bus_e {
    TX = 0,
    RX,
    BUS_MAX
} i2s_bus_t;

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

void serdes_i2s_start_slave();

/**
 * @brief Boolean determining if the I2S bus is running
 *
 * @return true Bus is running
 * @return false Bus is not running
 */
bool serdes_i2s_is_running();

/* THE FOLLOWING ARE PROVIDED FOR DEBUGGING ACCESS */
/**
 * @brief Get the FIFOSTAT register
 *
 * @param i2s_bus - receive or transmit bus
 * @return Register value
 */
uint32_t serdes_i2s_get_fifo_status(i2s_bus_t i2s_bus);

/**
 * @brief Get the FIFOCFG register
 *
 * @param i2s_bus receive or transmit
 * @return Register value
 */
uint32_t serdes_i2s_get_fifo_config(i2s_bus_t i2s_bus);

/**
 * @brief Get the I2S CFG1 register
 *
 * @param i2s_bus Receive or transmit bus
 * @return Value of CFG1 register
 */
uint32_t serdes_i2s_get_cfg1(i2s_bus_t i2s_bus);

/**
 * @brief Get the I2S CFG2 register
 *
 * @param i2s_bus Receive or transmit bus
 * @return Value of CFG2 register
 */
uint32_t serdes_i2s_get_cfg2(i2s_bus_t i2s_bus);

/**
 * @brief Get the value of the I2S STAT register
 *
 * @param i2s_bus Receive or transmit bus
 * @return Value of STAT register
 */
uint32_t serdes_i2s_get_stat(i2s_bus_t i2s_bus);

uint32_t serdes_i2s_get_err_cnt();

uint64_t *serdes_i2s_rx_times();

#endif