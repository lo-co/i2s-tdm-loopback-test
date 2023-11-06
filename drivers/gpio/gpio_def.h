#ifndef _GPIO_DEF_H
#define _GPIO_DEF_H

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define IOPCTL_PIO_ANAMUX_DI 0x00u        /*!<@brief Analog mux is disabled */
#define IOPCTL_PIO_FULLDRIVE_DI 0x00u     /*!<@brief Normal drive */
#define IOPCTL_PIO_FULLDRIVE_EN 0x0100u   /*!<@brief Full drive enable */
#define IOPCTL_PIO_FUNC0 0x00u            /*!<@brief Selects pin function 0 */
#define IOPCTL_PIO_FUNC1 0x01u            /*!<@brief Selects pin function 1 */
#define IOPCTL_PIO_INBUF_DI 0x00u         /*!<@brief Disable input buffer function */
#define IOPCTL_PIO_INBUF_EN 0x40u         /*!<@brief Enables input buffer function */
#define IOPCTL_PIO_INV_DI 0x00u           /*!<@brief Input function is not inverted */
#define IOPCTL_PIO_PSEDRAIN_DI 0x00u      /*!<@brief Pseudo Output Drain is disabled */
#define IOPCTL_PIO_PSEDRAIN_EN 0x0400u    /*!<@brief Pseudo Output Drain is enabled */
#define IOPCTL_PIO_PULLDOWN_EN 0x00u      /*!<@brief Enable pull-down function */
#define IOPCTL_PIO_PUPD_DI 0x00u          /*!<@brief Disable pull-up / pull-down function */
#define IOPCTL_PIO_SLEW_RATE_NORMAL 0x00u /*!<@brief Normal mode */
#define IOPCTL_PIO_PUPD_EN 0x10u          /*!<@brief Enable pull-up / pull-down function */
#define IOPCTL_PIO_PULLUP_EN 0x20u        /*!<@brief Enable pull-up function */

// Pin definitions
#define SW2_PORT    0U
#define SW2_PIN     10U

typedef enum led_color_e
{
    GREEN,
    BLUE,
    RED,
    MAX_LEDS
} led_color_t;

typedef struct pin_def_s
{
    uint8_t port;
    uint8_t pin;
} pin_def_t;

const pin_def_t led_pin_defs[3] =
    {{.port = 0, .pin = 14}, // GREEN
     {.port = 0, .pin = 26}, // BLUE
     {.port = 0, .pin = 31}  // RED
    };

const pin_def_t switch_pin_defs[2] =
    {{.port = 1, .pin = 10},
     {.port = 0, .pin = 10}
    };

// TODO: Add functionality to initialize ports and pins 1x so we don't step on initialized ports

#endif
