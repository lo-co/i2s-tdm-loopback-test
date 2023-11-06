#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_iopctl.h"

#include "serdes_gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SW2_GPIO_INTA_IRQHandler GPIO_INTA_DriverIRQHandler
#define SW2_IRQ                  GPIO_INTA_IRQn
/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
/** Configure the switch 2 interrupt.
 *
 * The ISR is overriden.  It is GPIO_INTA_DriverIRQHandler.
*/
static void serdes_gpio_cfg_sw2_int();
static void serdes_gpio_pin_init();

static uint8_t serdes_handle_led_event(void *userData);


/*******************************************************************************
 * Variables
 ******************************************************************************/
static bool serdes_pins_initialized =  false;
/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

void serdes_gpio_init()
{
    // Maintain a copy of the configuration structure
    serdes_gpio_pin_init();
    serdes_gpio_cfg_sw2_int();
    serdes_register_handler(SET_LED_STATE, serdes_handle_led_event);
}

static void serdes_gpio_cfg_sw2_int()
{
    gpio_interrupt_config_t config = {kGPIO_PinIntEnableEdge, kGPIO_PinIntEnableLowOrFall};

    // Enable GPIO interrupt A
    EnableIRQ(GPIO_INTA_IRQn);

    /* Initialize GPIO functionality on pin PIO0_10 (pin J3)  */
    GPIO_SetPinInterruptConfig(GPIO, SW2_PORT, SW2_PIN, &config);
    GPIO_PinEnableInterrupt(GPIO, SW2_PORT, SW2_PIN, 0);
}

static void serdes_gpio_pin_init()
{
    const gpio_pin_config_t SW2_config = {
        .pinDirection = kGPIO_DigitalInput,
        .outputLogic = 0U
    };

    // initialize the port for the switch
    GPIO_PortInit(GPIO, 0U);

    /* Initialize GPIO functionality on pin PIO0_10 (pin J3)  */
    /* This call will enable the port clock (in this case kCLOCK_HsGpio0 for port 0),
     * clear the peripheral and set the direction. */
    GPIO_PinInit(GPIO, SW2_PORT, SW2_PIN, &SW2_config);

    // Connect the pins
    const uint32_t sw2_cfg = (/* Pin is configured as PIO0_10 */
                          IOPCTL_PIO_FUNC0 |
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
    IOPCTL_PinMuxSet(IOPCTL, SW2_PORT, SW2_PIN, sw2_cfg);

    uint32_t led_cfg = (/* Pin is configured as PIO0_26 */
                        IOPCTL_PIO_FUNC0 |
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

    gpio_pin_config_t led_config = {
        kGPIO_DigitalOutput,
        0,
    };
    for (uint8_t idx = 0; idx < MAX_LEDS; idx++)
    {
        IOPCTL_PinMuxSet(IOPCTL, led_pin_defs[idx].port, led_pin_defs[idx].pin, led_cfg);
        GPIO_PinInit(GPIO, led_pin_defs[idx].port, led_pin_defs[idx].pin, &led_config);
    }


    serdes_pins_initialized = true;

}

void GPIO_INTA_DriverIRQHandler(void)
{
    /* clear the interrupt status */
    GPIO_PinClearInterruptFlag(GPIO, SW2_PORT, SW2_PIN, 0);
    serdes_push_event(SWITCH_2_PRESSED, NULL);
    SDK_ISR_EXIT_BARRIER;
}

void serdes_set_led_state (led_color_t led, bool is_on)
{
    GPIO_PinWrite(GPIO, led_pin_defs[led].port, led_pin_defs[led].pin, is_on);
}

static uint8_t serdes_handle_led_event(void *userData)
{
    led_state_t led_state = *(led_state_t *)userData;
    serdes_set_led_state(led_state.color, led_state.set_on);
    return 0;
}

