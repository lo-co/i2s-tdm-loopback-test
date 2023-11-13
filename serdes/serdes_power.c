#include "serdes_power.h"
// #include "serdes_event.h"
#include "board.h"
#include "fsl_pint.h"
#include "fsl_inputmux.h"
#include "fsl_clock.h"
#include "fsl_pca9420.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEEPSLEEP_RAM_APD 0xFFFFF8U
#define DEEPSLEEP_RAM_PPD 0x0U

#define EXCLUDE_FROM_DEEPSLEEP                                                                                \
    (((const uint32_t[]){SYSCTL0_PDSLEEPCFG0_RBB_PD_MASK,                                                                   \
                         (SYSCTL0_PDSLEEPCFG1_FLEXSPI_SRAM_APD_MASK | SYSCTL0_PDSLEEPCFG1_FLEXSPI_SRAM_PPD_MASK), \
                         DEEPSLEEP_RAM_APD, DEEPSLEEP_RAM_PPD}))

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
static void serdes_init_pmic();

// static void serdes_pint_wakeup_cb(pint_pin_int_t pintr, uint32_t pmatch_status);

static void serdes_config_pmic_modes(pca9420_modecfg_t *cfg);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static pca9420_handle_t pca9420Handle;
static pca9420_mode_t pca9420CurrMode;
static pca9420_modecfg_t pca9420CurrModeCfg;
static bool pmicVoltChangedForDeepSleep;

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

void serdes_power_init()
{
    pca9420_modecfg_t pca9420ModeCfg[4];
    power_pad_vrange_t vrange = {.Vdde0Range = kPadVol_171_198,
                                 .Vdde1Range = kPadVol_171_360,
                                 /* SD0 voltage is switchable, but in power_manager demo, it's fixed 3.3V. */
                                 .Vdde2Range = kPadVol_300_360};

    serdes_init_pmic();

    for (int mode = 0; mode < 4; mode++)
    {
        PCA9420_GetDefaultModeConfig(&pca9420ModeCfg[mode]);
    }
    serdes_config_pmic_modes(pca9420ModeCfg);

    PCA9420_WriteModeConfigs(&pca9420Handle, kPCA9420_Mode0, &pca9420ModeCfg[0], 4);

    /* Configure PMIC Vddcore value according to main clock. DSP not used in this demo. */
    serdes_set_pmic_voltage_freq(kPartTemp_0C_P85C, kVoltOpFullRange, CLOCK_GetFreq(kCLOCK_CoreSysClk), 0U);

    /* Indicate to power library that PMIC is used. */
    POWER_UpdatePmicRecoveryTime(1);

    POWER_SetPadVolRange(&vrange);
}

void serdes_power_enter_deepsleep()
{
    serdes_set_voltage_before_deepsleep();
    POWER_EnterDeepSleep(EXCLUDE_FROM_DEEPSLEEP);
    serdes_restore_voltage_after_deepsleep();
}
void serdes_set_vdd_core_voltage(uint32_t millivolt)
{
    PCA9420_GetCurrentMode(&pca9420Handle, &pca9420CurrMode);
    PCA9420_ReadModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);

    /* PMIC ouputs 500 + n*25mV */
    if ((millivolt - 500U) % 25U != 0)
    {
        millivolt += 25U; /* Round up. */
    }
    pca9420CurrModeCfg.sw1OutVolt = (pca9420_sw1_out_t)((millivolt - 500U) / 25U);

    PCA9420_WriteModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);
}


void serdes_set_voltage_before_deepsleep(void)
{
    PCA9420_GetCurrentMode(&pca9420Handle, &pca9420CurrMode);
    PCA9420_ReadModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);

    if (pca9420CurrModeCfg.sw1OutVolt == kPCA9420_Sw1OutVolt0V700)
    {
        pmicVoltChangedForDeepSleep = true;
        /* On resume from deep sleep with external PMIC, LVD is always used even if we have already disabled it.
         * Here we need to set up a safe threshold to avoid LVD reset and interrupt. */
        POWER_SetLvdFallingTripVoltage(kLvdFallingTripVol_720);
        pca9420CurrModeCfg.sw1OutVolt = kPCA9420_Sw1OutVolt0V750;
        PCA9420_WriteModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);
    }
    else
    {
    }
}
void serdes_restore_voltage_after_deepsleep(void)
{
    if (pmicVoltChangedForDeepSleep)
    {
        PCA9420_GetCurrentMode(&pca9420Handle, &pca9420CurrMode);
        PCA9420_ReadModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);
        pca9420CurrModeCfg.sw1OutVolt = kPCA9420_Sw1OutVolt0V700;
        PCA9420_WriteModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);
        pmicVoltChangedForDeepSleep = false;
    }
    else
    {
    }

}

// static void serdes_configure_pin_interrupt()
// {
//     INPUTMUX_Init(INPUTMUX);
//     INPUTMUX_AttachSignal(INPUTMUX, kPINT_PinInt0, kINPUTMUX_GpioPort1Pin1ToPintsel);
//     INPUTMUX_Deinit(INPUTMUX);

//     PINT_Init(PINT);
//     PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableFallEdge, serdes_pint_wakeup_cb);
//     PINT_EnableCallback(PINT); /* Enable callbacks for PINT */

//     EnableDeepSleepIRQ(PIN_INT0_IRQn);

// }

// static void serdes_pint_wakeup_cb(pint_pin_int_t pintr, uint32_t pmatch_status)
// {
//     static uint32_t power_event_flags = 0;
//     power_event_flags = POWER_GetEventFlags();
//     // Exclude nothing...
//     const uint32_t exclude_pd[4] = {SYSCTL0_PDSLEEPCFG0_RBB_PD_MASK,                                                                   \
//                          (SYSCTL0_PDSLEEPCFG1_FLEXSPI_SRAM_APD_MASK | SYSCTL0_PDSLEEPCFG1_FLEXSPI_SRAM_PPD_MASK), \
//                          0xFFFFF8U, 0};

//     if ((power_event_flags & PMC_FLAGS_DEEPPDF_MASK) != 0)
//     {
//         POWER_ClearEventFlags(PMC_FLAGS_DEEPPDF_MASK);
//         // serdes_push_event(WAKEUP, &power_event_flags);
//     }
//     else
//     {
//         BOARD_EnterDeepSleep(exclude_pd);
//     }
// }

static void serdes_init_pmic()
{
    pca9420_config_t pca9420Config;
    CLOCK_AttachClk(kSFRO_to_FLEXCOMM15);
    BOARD_PMIC_I2C_Init();
    PCA9420_GetDefaultConfig(&pca9420Config);
    pca9420Config.I2C_SendFunc    = BOARD_PMIC_I2C_Send;
    pca9420Config.I2C_ReceiveFunc = BOARD_PMIC_I2C_Receive;
    PCA9420_Init(&pca9420Handle, &pca9420Config);
}

static void serdes_config_pmic_modes(pca9420_modecfg_t *cfg)
{
    assert(cfg);

    /* Configuration PMIC mode to align with power lib like below:
     *  0b00    run mode, no special.
     *  0b01    deep sleep mode, vddcore 0.7V.
     *  0b10    deep powerdown mode, vddcore off.
     *  0b11    full deep powerdown mode vdd1v8 and vddcore off. */

    /* Mode 1: VDDCORE 0.7V. */
    cfg[1].sw1OutVolt = kPCA9420_Sw1OutVolt0V700;

    /* Mode 2: VDDCORE off. */
    cfg[2].enableSw1Out = false;

    /* Mode 3: VDDCORE, VDD1V8 and VDDIO off. */
    cfg[3].enableSw1Out  = false;
    cfg[3].enableSw2Out  = false;
    cfg[3].enableLdo2Out = false;

}

bool serdes_set_pmic_voltage_freq(power_part_temp_range_t tempRange,
                                 power_volt_op_range_t voltOpRange,
                                 uint32_t cm33Freq,
                                 uint32_t dspFreq)
{
    POWER_SetVddCoreSupplySrc(kVddCoreSrc_PMIC);
    POWER_SetPmicCoreSupplyFunc(serdes_set_vdd_core_voltage);
    return POWER_SetVoltageForFreq(tempRange, voltOpRange, cm33Freq, dspFreq, 0U);
}
