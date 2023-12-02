#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_iopctl.h"
#include "fsl_inputmux.h"
#include "fsl_pint.h"
#include "fsl_power.h"

#include "board.h"
#include "serdes_gpio.h"
#include "serdes_protocom.h"

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

/**
 * @brief Call to handle pin muxing for the EVK
 *
 */
static void serdes_gpio_pin_init();

/**
 * @brief Handler to be registered with the event dispatcher for an LED event
 *
 * @param userData Type is led_state_t - this will be passed to the event to change the LED
 * @return 0 if Success
 */
static uint8_t serdes_handle_led_event(void *userData);

/**
 * @brief Configure switch 1 for providing an interrupt while asleep
 *
 */
static void serdes_configure_pin_interrupt();

static void serdes_pint_cb(pint_pin_int_t pintr, uint32_t pmatch_status);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static bool serdes_pins_initialized =  false;
static bool is_slave = false;
static uint8_t led_state = 0x0;
static data_pckt_t led_data = {.src = SRC_GPIO, .data_length = 1, .data = &led_state};
/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

void serdes_gpio_init(bool is_master)
{
    is_slave = !is_master;
    // Maintain a copy of the configuration structure
    serdes_gpio_pin_init();
    serdes_gpio_cfg_sw2_int();
    serdes_configure_pin_interrupt();
    serdes_register_handler(SET_LED_STATE, serdes_handle_led_event);
}

static void serdes_gpio_cfg_sw2_int()
{
    gpio_interrupt_config_t config = {kGPIO_PinIntEnableEdge, kGPIO_PinIntEnableLowOrFall};

    // Enable GPIO interrupt A
    EnableIRQ(GPIO_INTA_IRQn);
    // EnableIRQ(GPIO_INTB_IRQn);

    /* Initialize GPIO functionality on pin PIO0_10 (pin J3)  */
    GPIO_SetPinInterruptConfig(GPIO, switch_pin_defs[SWITCH_2].port, switch_pin_defs[SWITCH_2].pin, &config);
    GPIO_PinEnableInterrupt(GPIO, switch_pin_defs[SWITCH_2].port, switch_pin_defs[SWITCH_2].pin, kGPIO_InterruptA);

    // GPIO_SetPinInterruptConfig(GPIO, switch_pin_defs[SWITCH_1].port, switch_pin_defs[SWITCH_1].pin, &config);
    // GPIO_PinEnableInterrupt(GPIO, switch_pin_defs[SWITCH_1].port, switch_pin_defs[SWITCH_1].pin, kGPIO_InterruptB);
}

static void serdes_gpio_pin_init()
{
    // initialize the port for the switch
    GPIO_PortInit(GPIO, 0U);
    GPIO_PortInit(GPIO, 1U);

    const gpio_pin_config_t switch_pin_cfg = {
        .pinDirection = kGPIO_DigitalInput,
        .outputLogic = 0U
    };

    // Connect the pins
    const uint32_t switch_cfg = (IOPCTL_PIO_FUNC0 |
                                IOPCTL_PIO_PUPD_DI |
                                IOPCTL_PIO_PULLDOWN_EN |
                                IOPCTL_PIO_INBUF_EN |
                                IOPCTL_PIO_SLEW_RATE_NORMAL |
                                IOPCTL_PIO_FULLDRIVE_DI |
                                IOPCTL_PIO_ANAMUX_DI |
                                IOPCTL_PIO_PSEDRAIN_DI |
                                IOPCTL_PIO_INV_DI);

    for (uint8_t idx = 0; idx < MAX_SWITCHES; idx++)
    {
        IOPCTL_PinMuxSet(IOPCTL, switch_pin_defs[idx].port, switch_pin_defs[idx].pin, switch_cfg);
        GPIO_PinInit(GPIO, switch_pin_defs[idx].port, switch_pin_defs[idx].pin, &switch_pin_cfg);
    }

    uint32_t led_cfg = (IOPCTL_PIO_FUNC0 |
                        IOPCTL_PIO_PUPD_DI |
                        IOPCTL_PIO_PULLDOWN_EN |
                        IOPCTL_PIO_INBUF_DI |
                        IOPCTL_PIO_SLEW_RATE_NORMAL |
                        IOPCTL_PIO_FULLDRIVE_DI |
                        IOPCTL_PIO_ANAMUX_DI |
                        IOPCTL_PIO_PSEDRAIN_DI |
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

    // This will provide a DIO to indicate SW2 has been pressed.
    // Use this as a trigger for capturing a logic trace
    IOPCTL_PinMuxSet(IOPCTL, 0, 5, led_cfg);
    GPIO_PinInit(GPIO, 0, 5, &led_config);

    serdes_pins_initialized = true;
}

void GPIO_INTA_DriverIRQHandler(void)
{
    /* clear the interrupt status */
    GPIO_PinClearInterruptFlag(GPIO, switch_pin_defs[SWITCH_2].port, switch_pin_defs[SWITCH_2].pin, kGPIO_InterruptA);
    serdes_push_event(SWITCH_2_PRESSED, NULL);
    SDK_ISR_EXIT_BARRIER;
}
// void GPIO_INTB_DriverIRQHandler(void)
// {
//     GPIO_PinClearInterruptFlag(GPIO, switch_pin_defs[SWITCH_1].port, switch_pin_defs[SWITCH_1].pin, kGPIO_InterruptB);
//     serdes_push_event(ENTER_DEEP_SLEEP, NULL);
//     SDK_ISR_EXIT_BARRIER;
// }

static void serdes_configure_pin_interrupt()
{
    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, kPINT_PinInt0, kINPUTMUX_GpioPort1Pin1ToPintsel);
    INPUTMUX_Deinit(INPUTMUX);

    PINT_Init(PINT);
    PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableFallEdge, serdes_pint_cb);
    PINT_EnableCallback(PINT); /* Enable callbacks for PINT */

    EnableDeepSleepIRQ(PIN_INT0_IRQn);
}

void serdes_set_led_state (led_color_t led, bool is_on)
{
    GPIO_PinWrite(GPIO, 0, 5, is_on);
    GPIO_PinWrite(GPIO, led_pin_defs[led].port, led_pin_defs[led].pin, is_on);
}

static uint8_t serdes_handle_led_event(void *userData)
{
    led_state_t led_state = *(led_state_t *)userData;
    serdes_set_led_state(led_state.color, led_state.set_on);
    return 0;
}

static void serdes_pint_cb(pint_pin_int_t pintr, uint32_t pmatch_status)
{
    if (is_slave)
    {
        led_state = !led_state ? 0xFF : 0x0;
        serdes_push_event(INSERT_DATA, &led_data);
    }
    else{
        static uint32_t power_event_flags = 0;
        power_event_flags = POWER_GetEventFlags();
        // Exclude nothing...
        if ((power_event_flags & PMC_FLAGS_DEEPPDF_MASK) != 0)
        {
            POWER_ClearEventFlags(PMC_FLAGS_DEEPPDF_MASK);
            serdes_push_event(WAKEUP, &power_event_flags);
        }
        else
        {
            // BOARD_EnterDeepSleep(exclude_pd);
            // POWER_EnterSleep();
        }
    }
}
