
#ifndef _SERDES_POWER_H
#define _SERDES_POWER_H
#include "fsl_power.h"

void serdes_power_init();

void serdes_set_voltage_before_deepsleep(void);

void serdes_restore_voltage_after_deepsleep(void);

bool serdes_set_pmic_voltage_freq(power_part_temp_range_t tempRange,
                                 power_volt_op_range_t voltOpRange,
                                 uint32_t cm33Freq,
                                 uint32_t dspFreq);

void serdes_set_vdd_core_voltage(uint32_t millivolt);

void serdes_power_enter_deepsleep();

#endif