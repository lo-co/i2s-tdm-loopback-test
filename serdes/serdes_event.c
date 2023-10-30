#include "fsl_ostimer.h"
#include "fsl_clock.h"
#include "fsl_debug_console.h"

#include "serdes_event.h"

static uint64_t s_tick_per_us = 0;

/** Get the current timer time in us
 *
 * @returns 64-bit integer representing time in microseconds
*/
static uint64_t ostime_get_us();
static void print_evt_data(event_t events);

void serdes_update_evt_times(evt_type_t evt_type, event_t *events)
{
    uint64_t c_time = ostime_get_us();
    events->evt_times[evt_type] = c_time;
    PRINTF("%d: %d\r\n", evt_type, c_time);
}

void serdes_event_init()
{
    CLOCK_AttachClk(kHCLK_to_OSTIMER_CLK);
    OSTIMER_Init(OSTIMER0);

    RSTCTL1->PRSTCTL0 != RSTCTL1_PRSTCTL0_OSEVT_TIMER_RST_MASK;
    RSTCTL1->PRSTCTL0 &= ~RSTCTL1_PRSTCTL0_OSEVT_TIMER_RST_MASK;

    s_tick_per_us = (uint32_t)(CLOCK_GetFreq(kCLOCK_CoreSysClk) / 1e6);
}

static uint64_t ostime_get_us()
{
    const uint64_t timer_ticks = OSTIMER_GetCurrentTimerValue(OSTIMER0);

    return (uint64_t)(timer_ticks/s_tick_per_us);

}

static void print_evt_data(event_t events)
{
    PRINTF("EVENT\tTIME\r\nINT_RCV\t%d\r\nTXMIT_START\t%d\r\nTXMIT_RCV\t%d\r\n",
        events.evt_times[RCV_INTERRUPT], events.evt_times[START_TXMIT], events.evt_times[RCV_TXMIT]);
}