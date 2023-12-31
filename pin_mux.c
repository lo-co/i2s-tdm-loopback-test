/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v14.0
processor: MIMXRT685S
package_id: MIMXRT685SFVKB
mcu_data: ksdk2_0
processor_version: 14.0.0
board: MIMXRT685-AUD-EVK
pin_labels:
- {pin_num: J3, pin_signal: PIO0_10/FC1_CTS_SDA_SSEL0/SCT0_GPI7/SCT0_OUT7/CTIMER1_MAT3/FC0_SSEL2/SEC_PIO0_10, label: SW2, identifier: SW2, comment: Input pin for
    switch used to turn off and on data xfer}
- {pin_num: A2, pin_signal: PIO0_26/FC3_SSEL2/SCT0_GPI6/SCT0_OUT6/CTIMER_INP7/SEC_PIO0_26/ADC0_3, label: LED_BLUE, identifier: LED_BLUE, comment: ON == data transmission}
- {pin_num: D11, pin_signal: PIO0_28/FC4_SCK/CTIMER4_MAT0/I2S_BRIDGE_CLK_OUT/SEC_PIO0_28, label: 'J27[3]', identifier: FC4_SCK, comment: Bitclock for FC4}
- {pin_num: B10, pin_signal: PIO0_29/FC4_TXD_SCL_MISO_WS/CTIMER4_MAT1/I2S_BRIDGE_WS_OUT/SEC_PIO0_29, label: 'J27[2]', identifier: FC4_I2S_WS, comment: FC4 word select}
- {pin_num: C11, pin_signal: PIO0_30/FC4_RXD_SDA_MOSI_DATA/CTIMER4_MAT2/I2S_BRIDGE_DATA_OUT/SEC_PIO0_30, label: 'JP42[2]', identifier: FC4_I2S_Data, comment: FC4
    I2S data}
- {pin_num: G16, pin_signal: PIO1_3/FC5_SCK, label: 'J28[6]/J58[7]/ARD_SPI_CLK', identifier: FC5_I2S_SCK, comment: FC5 bitclock}
- {pin_num: G17, pin_signal: PIO1_4/FC5_TXD_SCL_MISO_WS, label: 'J28[5]/J45[32]/J58[5]/ARD_SPI_MISO', identifier: FC5_I2S_WS, comment: FC5 I2S word select}
- {pin_num: J16, pin_signal: PIO1_5/FC5_RXD_SDA_MOSI_DATA, label: 'JP41[2]', identifier: FC5_I2S_Data, comment: FC5 I2S data}
- {pin_num: B1, pin_signal: PIO1_9/FC5_SSEL3/SCT0_GPI7/UTICK_CAP1/CTIMER1_MAT3/ADC0_12, label: WL_REG_ON, identifier: BOARD_INITPINS_WL_REG_ON;WL_REG_ON}
- {pin_num: N1, pin_signal: PIO1_13/HS_SPI_MOSI/CTIMER2_MAT2/FLEXSPI0B_DATA2, label: TimingInt2, identifier: Timing_Int_2, comment: DO used for timing purposes}
- {pin_num: N2, pin_signal: PIO1_14/HS_SPI_SSEL0/CTIMER2_MAT3/FLEXSPI0B_DATA3, label: TimingInt1, identifier: Timing_Int_1, comment: Used as a DO for timing purposes}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "fsl_iopctl.h"
#include "pin_mux.h"

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 *
 * END ****************************************************************************************************************/
void BOARD_InitBootPins(void)
{
    BOARD_InitDebugConsolePins();
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitDebugConsolePins:
- options: {callFromInitBoot: 'true', coreID: cm33, enableClock: 'true'}
- pin_list:
  - {pin_num: G4, peripheral: FLEXCOMM0, signal: RXD_SDA_MOSI_DATA, pin_signal: PIO0_2/FC0_RXD_SDA_MOSI_DATA/CTIMER0_MAT2/I2S_BRIDGE_DATA_IN/SEC_PIO0_2, pupdena: disabled,
    pupdsel: pullDown, ibena: enabled, slew_rate: normal, drive: normal, amena: disabled, odena: disabled, iiena: disabled}
  - {pin_num: G2, peripheral: FLEXCOMM0, signal: TXD_SCL_MISO_WS, pin_signal: PIO0_1/FC0_TXD_SCL_MISO_WS/CTIMER0_MAT1/I2S_BRIDGE_WS_IN/SEC_PIO0_1, pupdena: disabled,
    pupdsel: pullDown, ibena: disabled, slew_rate: normal, drive: normal, amena: disabled, odena: disabled, iiena: disabled}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitDebugConsolePins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
/* Function assigned for the Cortex-M33 */
void BOARD_InitDebugConsolePins(void)
{

    const uint32_t port0_pin1_config = (/* Pin is configured as FC0_TXD_SCL_MISO_WS */
                                        IOPCTL_PIO_FUNC1 |
                                        /* Disable pull-up / pull-down function */
                                        IOPCTL_PIO_PUPD_DI |
                                        /* Enable pull-down function */
                                        IOPCTL_PIO_PULLDOWN_EN |
                                        /* Disable input buffer function */
                                        IOPCTL_PIO_INBUF_DI |
                                        /* Normal mode */
                                        IOPCTL_PIO_SLEW_RATE_NORMAL |
                                        /* Normal drive */
                                        IOPCTL_PIO_FULLDRIVE_DI |
                                        /* Analog mux is disabled */
                                        IOPCTL_PIO_ANAMUX_DI |
                                        /* Pseudo Output Drain is disabled */
                                        IOPCTL_PIO_PSEDRAIN_DI |
                                        /* Input function is not inverted */
                                        IOPCTL_PIO_INV_DI);
    /* PORT0 PIN1 (coords: G2) is configured as FC0_TXD_SCL_MISO_WS */
    IOPCTL_PinMuxSet(IOPCTL, 0U, 1U, port0_pin1_config);

    const uint32_t port0_pin2_config = (/* Pin is configured as FC0_RXD_SDA_MOSI_DATA */
                                        IOPCTL_PIO_FUNC1 |
                                        /* Disable pull-up / pull-down function */
                                        IOPCTL_PIO_PUPD_DI |
                                        /* Enable pull-down function */
                                        IOPCTL_PIO_PULLDOWN_EN |
                                        /* Enables input buffer function */
                                        IOPCTL_PIO_INBUF_EN |
                                        /* Normal mode */
                                        IOPCTL_PIO_SLEW_RATE_NORMAL |
                                        /* Normal drive */
                                        IOPCTL_PIO_FULLDRIVE_DI |
                                        /* Analog mux is disabled */
                                        IOPCTL_PIO_ANAMUX_DI |
                                        /* Pseudo Output Drain is disabled */
                                        IOPCTL_PIO_PSEDRAIN_DI |
                                        /* Input function is not inverted */
                                        IOPCTL_PIO_INV_DI);
    /* PORT0 PIN2 (coords: G4) is configured as FC0_RXD_SDA_MOSI_DATA */
    IOPCTL_PinMuxSet(IOPCTL, 0U, 2U, port0_pin2_config);
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
