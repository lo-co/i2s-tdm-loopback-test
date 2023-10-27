/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Peripherals v13.0
processor: MIMXRT685S
package_id: MIMXRT685SFVKB
mcu_data: ksdk2_0
processor_version: 14.0.0
board: MIMXRT685-AUD-EVK
functionalGroups:
- name: BOARD_InitPeripherals
  UUID: 6e259c09-af7d-49fc-a856-64d6d3ecebce
  called_from_default_init: true
  selectedCore: cm33
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
component:
- type: 'system'
- type_id: 'system_54b53072540eeeb8f8e9343e71f28176'
- global_system_definitions:
  - user_definitions: ''
  - user_includes: ''
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
component:
- type: 'uart_cmsis_common'
- type_id: 'uart_cmsis_common_9cb8e302497aa696fdbb5a4fd622c2a8'
- global_USART_CMSIS_common:
  - quick_selection: 'default'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
component:
- type: 'gpio_adapter_common'
- type_id: 'gpio_adapter_common_57579b9ac814fe26bf95df0a384c36b6'
- global_gpio_adapter_common:
  - quick_selection: 'default'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "peripherals.h"

/***********************************************************************************************************************
 * BOARD_InitPeripherals functional group
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * DMA0 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'DMA0'
- type: 'lpc_dma'
- mode: 'basic'
- custom_name_enabled: 'false'
- type_id: 'lpc_dma_1e560b38547e8785cc7d22228be0a18c'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'DMA0'
- config_sets:
  - fsl_dma:
    - dma_table:
      - 0: []
      - 1: []
      - 2: []
      - 3: []
    - dma_channels: []
    - init_interrupt: 'false'
    - dma_interrupt:
      - IRQn: 'DMA0_IRQn'
      - enable_interrrupt: 'enabled'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'false'
    - quick_selection: 'default'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/* Empty initialization function (commented out)
static void DMA0_init(void) {
} */

/***********************************************************************************************************************
 * NVIC initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'NVIC'
- type: 'nvic'
- mode: 'general'
- custom_name_enabled: 'false'
- type_id: 'nvic_57b5eef3774cc60acaede6f5b8bddc67'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'NVIC'
- config_sets:
  - nvic:
    - interrupt_table: []
    - interrupts: []
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/* Empty initialization function (commented out)
static void NVIC_init(void) {
} */

/***********************************************************************************************************************
 * FC4_I2S_TX initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'FC4_I2S_TX'
- type: 'flexcomm_i2s'
- mode: 'dma'
- custom_name_enabled: 'true'
- type_id: 'flexcomm_i2s_d821d1d3dded76c4d4194ae52cbf73a5'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'FLEXCOMM4'
- config_sets:
  - fsl_i2s:
    - i2s_config:
      - usage: 'playback'
      - masterSlave: 'kI2S_MasterSlaveNormalMaster'
      - sckPolM: 'false'
      - wsPolM: 'true'
      - clockConfig:
        - sampleRate_Hz: 'kSAI_SampleRate48KHz'
        - clockSource: 'FXCOMFunctionClock'
        - clockSourceFreq: 'ClocksTool_DefaultInit'
        - masterClockDependency: 'false'
      - mode: 'kI2S_ModeDspWsShort'
      - dataLengthM: '32'
      - stereo: 'kSAI_Stereo'
      - positionM: '1'
      - secondary_channels_array:
        - 0:
          - stereo: 'kSAI_Stereo'
          - positionM: '65'
        - 1:
          - stereo: 'kSAI_Stereo'
          - positionM: '129'
        - 2:
          - stereo: 'kSAI_Stereo'
          - positionM: '193'
      - frameLengthM: '256'
      - rightLow: 'false'
      - leftJust: 'false'
      - watermarkM_Tx: 'ki2s_TxFifo4'
      - txEmptyZeroTx: 'true'
      - pack48: 'false'
  - dmaCfg:
    - dma_channels:
      - dma_tx_channel:
        - DMA_source: 'kDma0RequestFlexcomm4Tx'
        - init_channel_priority: 'true'
        - dma_priority: 'kDMA_ChannelPriority3'
        - enable_custom_name: 'false'
    - i2s_dma_handle:
      - enable_custom_name: 'true'
      - handle_custom_name: 'FC4_I2S_Tx_Handle'
      - init_callback: 'true'
      - callback_fcn: 'fc4_i2s_tx_cb'
      - user_data: 'fc4I2sTxData'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
/* Flexcomm I2S configuration */
const i2s_config_t FC4_I2S_TX_config = {
  .masterSlave = kI2S_MasterSlaveNormalMaster,
  .mode = kI2S_ModeDspWsShort,
  .rightLow = false,
  .leftJust = false,
  .sckPol = false,
  .wsPol = true,
  .divider = 2,
  .oneChannel = false,
  .dataLength = 32,
  .frameLength = 256,
  .position = 1,
  .watermark = 4,
  .txEmptyZero = true,
  .pack48 = false
};
dma_handle_t FC4_I2S_TX_TX_Handle;
i2s_dma_handle_t FC4_I2S_Tx_Handle;

static void FC4_I2S_TX_init(void) {
  /* Flexcomm I2S initialization */
  I2S_TxInit(FC4_I2S_TX_PERIPHERAL, &FC4_I2S_TX_config);
  /* Setup secondary channel 1 */
  I2S_EnableSecondaryChannel(FC4_I2S_TX_PERIPHERAL, kI2S_SecondaryChannel1, false, 65U);
  /* Setup secondary channel 2 */
  I2S_EnableSecondaryChannel(FC4_I2S_TX_PERIPHERAL, kI2S_SecondaryChannel2, false, 129U);
  /* Setup secondary channel 3 */
  I2S_EnableSecondaryChannel(FC4_I2S_TX_PERIPHERAL, kI2S_SecondaryChannel3, false, 193U);
  /* Enable the DMA 9 channel in the DMA */
  DMA_EnableChannel(FC4_I2S_TX_TX_DMA_BASEADDR, FC4_I2S_TX_TX_DMA_CHANNEL);
  /* Set the DMA 9 channel priority */
  DMA_SetChannelPriority(FC4_I2S_TX_TX_DMA_BASEADDR, FC4_I2S_TX_TX_DMA_CHANNEL, kDMA_ChannelPriority3);
  /* Create the DMA FC4_I2S_TX_TX_Handle handle */
  DMA_CreateHandle(&FC4_I2S_TX_TX_Handle, FC4_I2S_TX_TX_DMA_BASEADDR, FC4_I2S_TX_TX_DMA_CHANNEL);
  /* Create the I2S DMA handle */
  I2S_TxTransferCreateHandleDMA(FC4_I2S_TX_PERIPHERAL, &FC4_I2S_Tx_Handle, &FC4_I2S_TX_TX_Handle, fc4_i2s_tx_cb, fc4I2sTxData);
}

/***********************************************************************************************************************
 * FC5_I2S initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'FC5_I2S'
- type: 'flexcomm_i2s'
- mode: 'dma'
- custom_name_enabled: 'true'
- type_id: 'flexcomm_i2s_d821d1d3dded76c4d4194ae52cbf73a5'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'FLEXCOMM5'
- config_sets:
  - fsl_i2s:
    - i2s_config:
      - usage: 'record'
      - masterSlave: 'kI2S_MasterSlaveNormalSlave'
      - sckPolM: 'false'
      - wsPolM: 'true'
      - clockConfig:
        - sampleRate_Hz: 'kSAI_SampleRate48KHz'
        - externalBitClockFrequency: '12288000 Hz'
        - masterClockDependency: 'false'
      - mode: 'kI2S_ModeDspWsShort'
      - dataLengthM: '32'
      - stereo: 'kSAI_Stereo'
      - positionM: '0'
      - secondary_channels_array:
        - 0:
          - stereo: 'kSAI_Stereo'
          - positionM: '64'
        - 1:
          - stereo: 'kSAI_Stereo'
          - positionM: '128'
        - 2:
          - stereo: 'kSAI_Stereo'
          - positionM: '192'
      - frameLengthM: '256'
      - rightLow: 'false'
      - leftJust: 'false'
      - watermarkM_Rx: 'ki2s_RxFifo1'
      - pack48: 'false'
  - dmaCfg:
    - dma_channels:
      - dma_rx_channel:
        - DMA_source: 'kDma0RequestFlexcomm5Rx'
        - init_channel_priority: 'false'
        - dma_priority: 'kDMA_ChannelPriority0'
        - enable_custom_name: 'false'
    - i2s_dma_handle:
      - enable_custom_name: 'true'
      - handle_custom_name: 'FC5_I2S_Rx_DMA_Handle'
      - init_callback: 'true'
      - callback_fcn: 'fc5_i2s_rx_cb'
      - user_data: 'fc5RxData'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
/* Flexcomm I2S configuration */
const i2s_config_t FC5_I2S_config = {
  .masterSlave = kI2S_MasterSlaveNormalSlave,
  .mode = kI2S_ModeDspWsShort,
  .rightLow = false,
  .leftJust = false,
  .sckPol = false,
  .wsPol = true,
  .divider = 1,
  .oneChannel = false,
  .dataLength = 32,
  .frameLength = 256,
  .position = 0,
  .watermark = 0,
  .txEmptyZero = false,
  .pack48 = false
};
dma_handle_t FC5_I2S_RX_Handle;
i2s_dma_handle_t FC5_I2S_Rx_DMA_Handle;

static void FC5_I2S_init(void) {
  /* Flexcomm I2S initialization */
  I2S_RxInit(FC5_I2S_PERIPHERAL, &FC5_I2S_config);
  /* Setup secondary channel 1 */
  I2S_EnableSecondaryChannel(FC5_I2S_PERIPHERAL, kI2S_SecondaryChannel1, false, 64U);
  /* Setup secondary channel 2 */
  I2S_EnableSecondaryChannel(FC5_I2S_PERIPHERAL, kI2S_SecondaryChannel2, false, 128U);
  /* Setup secondary channel 3 */
  I2S_EnableSecondaryChannel(FC5_I2S_PERIPHERAL, kI2S_SecondaryChannel3, false, 192U);
  /* Enable the DMA 10 channel in the DMA */
  DMA_EnableChannel(FC5_I2S_RX_DMA_BASEADDR, FC5_I2S_RX_DMA_CHANNEL);
  /* Create the DMA FC5_I2S_RX_Handle handle */
  DMA_CreateHandle(&FC5_I2S_RX_Handle, FC5_I2S_RX_DMA_BASEADDR, FC5_I2S_RX_DMA_CHANNEL);
  /* Create the I2S DMA handle */
  I2S_RxTransferCreateHandleDMA(FC5_I2S_PERIPHERAL, &FC5_I2S_Rx_DMA_Handle, &FC5_I2S_RX_Handle, fc5_i2s_rx_cb, fc5RxData);
}

/***********************************************************************************************************************
 * FLEXCOMM3 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'FLEXCOMM3'
- type: 'flexcomm_i2s'
- mode: 'dma'
- custom_name_enabled: 'false'
- type_id: 'flexcomm_i2s_d821d1d3dded76c4d4194ae52cbf73a5'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'FLEXCOMM3'
- config_sets:
  - fsl_i2s:
    - i2s_config:
      - usage: 'playback'
      - masterSlave: 'kI2S_MasterSlaveNormalMaster'
      - sckPolM: 'false'
      - wsPolM: 'true'
      - clockConfig:
        - sampleRate_Hz: 'kSAI_SampleRate48KHz'
        - clockSource: 'FXCOMFunctionClock'
        - clockSourceFreq: 'ClocksTool_DefaultInit'
        - masterClockDependency: 'false'
      - mode: 'kI2S_ModeDspWsShort'
      - dataLengthM: '32'
      - stereo: 'kSAI_Stereo'
      - positionM: '0'
      - secondary_channels_array:
        - 0:
          - stereo: 'kSAI_Stereo'
          - positionM: '64'
        - 1:
          - stereo: 'kSAI_Stereo'
          - positionM: '128'
        - 2:
          - stereo: 'kSAI_Stereo'
          - positionM: '192'
      - frameLengthM: '256'
      - rightLow: 'false'
      - leftJust: 'false'
      - watermarkM_Tx: 'ki2s_TxFifo4'
      - txEmptyZeroTx: 'true'
      - pack48: 'false'
  - dmaCfg:
    - dma_channels:
      - dma_tx_channel:
        - DMA_source: 'kDma0RequestFlexcomm3Tx'
        - init_channel_priority: 'true'
        - dma_priority: 'kDMA_ChannelPriority3'
        - enable_custom_name: 'true'
        - handle_custom_name: 'i2sTxDmaHandle'
    - i2s_dma_handle:
      - enable_custom_name: 'true'
      - handle_custom_name: 'i2sTxHandle'
      - init_callback: 'true'
      - callback_fcn: 'i2s_tx_Callback'
      - user_data: 'txData'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
/* Flexcomm I2S configuration */
const i2s_config_t FLEXCOMM3_config = {
  .masterSlave = kI2S_MasterSlaveNormalMaster,
  .mode = kI2S_ModeDspWsShort,
  .rightLow = false,
  .leftJust = false,
  .sckPol = false,
  .wsPol = true,
  .divider = 2,
  .oneChannel = false,
  .dataLength = 32,
  .frameLength = 256,
  .position = 0,
  .watermark = 4,
  .txEmptyZero = true,
  .pack48 = false
};
dma_handle_t i2sTxDmaHandle;
i2s_dma_handle_t i2sTxHandle;

static void FLEXCOMM3_init(void) {
  /* Flexcomm I2S initialization */
  I2S_TxInit(FLEXCOMM3_PERIPHERAL, &FLEXCOMM3_config);
  /* Setup secondary channel 1 */
  I2S_EnableSecondaryChannel(FLEXCOMM3_PERIPHERAL, kI2S_SecondaryChannel1, false, 64U);
  /* Setup secondary channel 2 */
  I2S_EnableSecondaryChannel(FLEXCOMM3_PERIPHERAL, kI2S_SecondaryChannel2, false, 128U);
  /* Setup secondary channel 3 */
  I2S_EnableSecondaryChannel(FLEXCOMM3_PERIPHERAL, kI2S_SecondaryChannel3, false, 192U);
  /* Enable the DMA 7 channel in the DMA */
  DMA_EnableChannel(FLEXCOMM3_TX_DMA_BASEADDR, FLEXCOMM3_TX_DMA_CHANNEL);
  /* Set the DMA 7 channel priority */
  DMA_SetChannelPriority(FLEXCOMM3_TX_DMA_BASEADDR, FLEXCOMM3_TX_DMA_CHANNEL, kDMA_ChannelPriority3);
  /* Create the DMA i2sTxDmaHandle handle */
  DMA_CreateHandle(&i2sTxDmaHandle, FLEXCOMM3_TX_DMA_BASEADDR, FLEXCOMM3_TX_DMA_CHANNEL);
  /* Create the I2S DMA handle */
  I2S_TxTransferCreateHandleDMA(FLEXCOMM3_PERIPHERAL, &i2sTxHandle, &i2sTxDmaHandle, i2s_tx_Callback, txData);
}

/***********************************************************************************************************************
 * FLEXCOMM1 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'FLEXCOMM1'
- type: 'flexcomm_i2s'
- mode: 'dma'
- custom_name_enabled: 'false'
- type_id: 'flexcomm_i2s_d821d1d3dded76c4d4194ae52cbf73a5'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'FLEXCOMM1'
- config_sets:
  - fsl_i2s:
    - i2s_config:
      - usage: 'record'
      - masterSlave: 'kI2S_MasterSlaveNormalMaster'
      - sckPolM: 'false'
      - wsPolM: 'true'
      - clockConfig:
        - sampleRate_Hz: 'kSAI_SampleRate48KHz'
        - clockSource: 'FXCOMFunctionClock'
        - clockSourceFreq: 'ClocksTool_DefaultInit'
        - masterClockDependency: 'false'
      - mode: 'kI2S_ModeDspWsShort'
      - dataLengthM: '32'
      - stereo: 'kSAI_Stereo'
      - positionM: '0'
      - secondary_channels_array:
        - 0:
          - stereo: 'kSAI_Stereo'
          - positionM: '64'
        - 1:
          - stereo: 'kSAI_Stereo'
          - positionM: '128'
        - 2:
          - stereo: 'kSAI_Stereo'
          - positionM: '192'
      - frameLengthM: '256'
      - rightLow: 'false'
      - leftJust: 'false'
      - watermarkM_Rx: 'ki2s_RxFifo5'
      - pack48: 'false'
  - dmaCfg:
    - dma_channels:
      - dma_rx_channel:
        - DMA_source: 'kDma0RequestFlexcomm1Rx'
        - init_channel_priority: 'true'
        - dma_priority: 'kDMA_ChannelPriority2'
        - enable_custom_name: 'true'
        - handle_custom_name: 'i2sRxDmaHandle'
    - i2s_dma_handle:
      - enable_custom_name: 'true'
      - handle_custom_name: 'i2sRxHandle'
      - init_callback: 'true'
      - callback_fcn: 'i2s_rx_Callback'
      - user_data: 'rxData'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
/* Flexcomm I2S configuration */
const i2s_config_t FLEXCOMM1_config = {
  .masterSlave = kI2S_MasterSlaveNormalMaster,
  .mode = kI2S_ModeDspWsShort,
  .rightLow = false,
  .leftJust = false,
  .sckPol = false,
  .wsPol = true,
  .divider = 2,
  .oneChannel = false,
  .dataLength = 32,
  .frameLength = 256,
  .position = 0,
  .watermark = 4,
  .txEmptyZero = false,
  .pack48 = false
};
dma_handle_t i2sRxDmaHandle;
i2s_dma_handle_t i2sRxHandle;

static void FLEXCOMM1_init(void) {
  /* Flexcomm I2S initialization */
  I2S_RxInit(FLEXCOMM1_PERIPHERAL, &FLEXCOMM1_config);
  /* Setup secondary channel 1 */
  I2S_EnableSecondaryChannel(FLEXCOMM1_PERIPHERAL, kI2S_SecondaryChannel1, false, 64U);
  /* Setup secondary channel 2 */
  I2S_EnableSecondaryChannel(FLEXCOMM1_PERIPHERAL, kI2S_SecondaryChannel2, false, 128U);
  /* Setup secondary channel 3 */
  I2S_EnableSecondaryChannel(FLEXCOMM1_PERIPHERAL, kI2S_SecondaryChannel3, false, 192U);
  /* Enable the DMA 2 channel in the DMA */
  DMA_EnableChannel(FLEXCOMM1_RX_DMA_BASEADDR, FLEXCOMM1_RX_DMA_CHANNEL);
  /* Set the DMA 2 channel priority */
  DMA_SetChannelPriority(FLEXCOMM1_RX_DMA_BASEADDR, FLEXCOMM1_RX_DMA_CHANNEL, kDMA_ChannelPriority2);
  /* Create the DMA i2sRxDmaHandle handle */
  DMA_CreateHandle(&i2sRxDmaHandle, FLEXCOMM1_RX_DMA_BASEADDR, FLEXCOMM1_RX_DMA_CHANNEL);
  /* Create the I2S DMA handle */
  I2S_RxTransferCreateHandleDMA(FLEXCOMM1_PERIPHERAL, &i2sRxHandle, &i2sRxDmaHandle, i2s_rx_Callback, rxData);
}

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void)
{
  /* Global initialization */
  // DMA_Init(DMA0_DMA_BASEADDR);

  /* Initialize components */
  // FC4_I2S_TX_init();
  // FC5_I2S_init();
  FLEXCOMM3_init();
  FLEXCOMM1_init();
}

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void)
{
  BOARD_InitPeripherals();
}
