#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_iopctl.h"

#include "serdes_gpio.h"
#include "../drivers/gpio/gpio_def.h"

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
static void serdes_gpio_cfg_sw2_int();
static void serdes_gpio_pin_init();

/*******************************************************************************
 * Variables
 ******************************************************************************/
static bool serdes_pins_initialized =  false;
static serdes_gpio_cfg_t g_gpio_cfg;
/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

void serdes_gpio_init(serdes_gpio_cfg_t cfg_data)
{
    // Maintain a copy of the configuration structure
    g_gpio_cfg = cfg_data;
    serdes_gpio_pin_init();
    serdes_gpio_cfg_sw2_int();
}

static void serdes_gpio_cfg_sw2_int()
{
    gpio_interrupt_config_t config = {kGPIO_PinIntEnableEdge, kGPIO_PinIntEnableLowOrFall};

    // Enable GPIO interrupt A
    EnableIRQ(GPIO_INTA_IRQn);

    /* Initialize GPIO functionality on pin PIO0_10 (pin J3)  */
    // Ugh...config tool does a half ass job of this
    GPIO_SetPinInterruptConfig(GPIO, SW2_PORT, SW2_PIN, &config);
    GPIO_PinEnableInterrupt(GPIO, SW2_PORT, SW2_PIN, 0);
}

static void serdes_gpio_pin_init()
{
    /* Enables the clock for the GPIO0 module */
    // CLOCK_EnableClock(kCLOCK_HsGpio0);

    gpio_pin_config_t SW2_config = {
        .pinDirection = kGPIO_DigitalInput,
        .outputLogic = 0U
    };

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

    serdes_pins_initialized = true;

}

void SW2_GPIO_INTA_IRQHandler(void)
{
    /* clear the interrupt status */
    GPIO_PinClearInterruptFlag(GPIO, SW2_PORT, SW2_PIN, 0);
    g_gpio_cfg.sw2_cb(NULL);
    SDK_ISR_EXIT_BARRIER;
}