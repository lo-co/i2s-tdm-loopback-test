#include "fsl_ostimer.h"
#include "fsl_clock.h"
#include "fsl_debug_console.h"

#include "serdes_event.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/**
 * @brief Preprocessor definition of the memory size for events.
 *
 * Used by the event queue and context to determine the number of events to store.
 *
 */
#define EVENT_QUEUE_SIZE (10)

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

typedef struct event_context_s {
    event_t *events;        ///< Event queue
    uint8_t head;           ///< Current write position of event queue
    uint8_t tail;           ///< Current read position of event queue
    uint8_t size;           ///< Maximum number of events in queue
    bool full;              ///< True == queue is full
    bool is_initialized;    ///< Queue context has been initialized
} event_context_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/**
 * @brief Number of ticks per microsecond.
 *
 * This value will be based on the core system clock value which will be returned
 * at initialization.
 *
 */
static uint64_t s_tick_per_us = 0;

/**
 * @brief Static allocation of space for event data.
 *
 * This is the application space for queueing data.  The initial value is NULL
 * and the size will be dependent on the definition of EVENT_QUEUE_SIZE
 *
 */
static event_t event_queue[EVENT_QUEUE_SIZE] = {0};

/**
 * @brief System even context that is maintained by the event module.
 *
 */
static event_context_t *event_context;

static event_t registered_events[MAX_EVENT] = {0};

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/** Get the current timer time in us
 *
 * @returns 64-bit integer representing time in microseconds
*/
static uint64_t ostime_get_us();

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

// Documented in .h
void serdes_event_init()
{
    CLOCK_AttachClk(kHCLK_to_OSTIMER_CLK);
    OSTIMER_Init(OSTIMER0);

    RSTCTL1->PRSTCTL0 != RSTCTL1_PRSTCTL0_OSEVT_TIMER_RST_MASK;
    RSTCTL1->PRSTCTL0 &= ~RSTCTL1_PRSTCTL0_OSEVT_TIMER_RST_MASK;

    s_tick_per_us = (uint32_t)(CLOCK_GetFreq(kCLOCK_CoreSysClk) / 1e6);

    event_context->events = event_queue;
    event_context->is_initialized = true;
    serdes_event_reset();
}

// Documented in .h
evt_action_response_t serdes_event_reset()
{
    assert(event_context->is_initialized);

    memset(event_queue, 0, sizeof(event_queue));
    event_context->head = 0;
    event_context->tail = 0;
    event_context->size = EVENT_QUEUE_SIZE;
    event_context->full = false;
    return EVENT_SUCCESS;
}

// Documented in .h
evt_action_response_t serdes_push_event(evt_type_t event_type, void *userData)
{
    assert(event_context->is_initialized);

    if (event_context->full)
    {
        PRINTF("Attempt to push event into queue failed.  Queue is full...\r\n");
        return EVENT_QUEUE_FULL;
    }

    event_t event = registered_events[event_type];
    event.idx = event_type;
    event.usrData = userData;
    event.event_time = ostime_get_us();

    *(event_context->events + event_context->head) = event;

    if (++event_context->head >= event_context->size)
    {
        event_context->head = 0;
    }

    event_context->full = event_context->head == event_context->tail;

    return EVENT_SUCCESS;
}

// Documented in .h
evt_action_response_t serdes_dispatch_event()
{
    if (!event_context->is_initialized)
    {
        assert(event_context->is_initialized);
    }
    if (event_context->tail == event_context->head){
        return EVENT_QUEUE_EMPTY;
    }

    event_t event = *(event_context->events + event_context->tail);

    uint8_t status = 0;

    if (event.handler)
    {
        status = event.handler(event.usrData);
    }

    if (++event_context->tail >=event_context->size)
    {
        event_context->tail = 0;
    }

    event_context->full = false;

    return status == 0 ? EVENT_SUCCESS : EVENT_HANDLER_ERROR;
}

// Documented above
static uint64_t ostime_get_us()
{
    const uint64_t timer_ticks = OSTIMER_GetCurrentTimerValue(OSTIMER0);

    return (uint64_t)(timer_ticks/s_tick_per_us);
}

void serdes_register_handler(evt_type_t event, event_handler handler)
{
    // Make sure that we have not registered multiple handlers...
    assert(!registered_events[event].handler);

    registered_events[event].handler = handler;
}
