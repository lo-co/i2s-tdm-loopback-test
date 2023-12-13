
/**
 * @file max98388_registers.h
 * @author Matt Richardson (mattrichardson@meta.com)
 * @brief Register list for the MAX 98388 amplifier
 * @version 0.1
 * @date 2023-11-27
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _MAX09388_REG_H
#define _MAX09388_REG_H

#define SWRESET 0x2000

// Device Status registers
#define STAT_RAW1    0x2001
#define STAT_RAW2    0x2002

#define STAT_STATE1  0x2004
#define STAT_STATE2  0x2005

// Thermal Protection
#define THERMAL_WRN  0x2020

// Error Monitoring
#define SPK_MON_THRESH  0x2031
#define SPK_MON_LOAD    0x2032
#define SPK_MON_DUR     0x2033
#define ERROR_MON_CTL   0x2037

// PCM Registers
#define PCM_MODE_CFG    0x2040
#define PCM_CLK_SETUP   0x2041
#define PCM_SAMPLE_RATE 0x2042
#define PCM_TX_CTL1     0x2044
#define PCM_TX_CTL2     0x2045
#define PCM_HIZ_CTL1    0x2050
#define PCM_HIZ_CTL2    0x2051
#define PCM_HIZ_CTL3    0x2052
#define PCM_HIZ_CTL4    0x2053
#define PCM_HIZ_CTL5    0x2054
#define PCM_HIZ_CTL6    0x2055
#define PCM_HIZ_CTL7    0x2056
#define PCM_HIZ_CTL8    0x2057
#define PCM_RX_SRC1     0x2058
#define PCM_RX_SRC2     0x2059
#define PCM_TX_DRV_STR  0x205C
#define PCM_TX_SRC_EN   0x205D
#define PCM_RX_EN       0x205E
#define PCM_TX_EN       0x205F

// Speaker Channel Control
#define SPK_VOL_CTL             0x2090
#define SPK_CH_CFG              0x2091
#define SPK_AMP_OUT_CFG         0x2092
#define SPK_AMP_SSM_CFG         0x2093
#define SPK_AMP_EDGE_RATE_CTL   0x2094
#define SPK_CH_PINK_NOISE_EN    0x209E
#define SPK_CH_AMP_EN           0x209F

// IV Data Channel Control
#define IV_DATA_DSP_CTL 0x20A0
#define IV_DATA_EN      0x20A7

// Brownout Protection ALC
#define BRWNOUT_PROT_ALC_THRESH             0x20E0
#define BRWNOUT_PROT_ALC_RATES              0x20E1
#define BRWNOUT_PROT_ALC_ATTN               0x20E2
#define BRWNOUT_PROT_ALC_RELEASE            0x20E3
#define BRWNOUT                             0x20E4
#define BRWNOUT_PROT_ALC_INF_HOLD_RELEASE   0x20EE
#define BRWNOUT_PROT_ALC_EN                 0x20EF

// System config
#define AUTO_RST        0x210E
#define GLOBAL_EN       0x210F

// Revision ID
#define DEV_REV_ID      0x22FF
#endif