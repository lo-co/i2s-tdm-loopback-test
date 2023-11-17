/**
 * @file serdes_event.h
 * @author Matt Richardson (mattrichardson@meta.com)
 * @brief
 * @version 0.1
 * @date 2023-11-06
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _SERDES_EVENT_H
#define _SERDES_EVENT_H
#include <stdint.h>
/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

typedef enum evt_type_e
{
    SWITCH_2_PRESSED = 0,
    AUDIO_SRC_DATA_AVAILABLE,
    DATA_RECEIVED,
    SET_LED_STATE,
    WAKEUP,
    ENTER_DEEP_SLEEP,
    INSERT_DATA,
    DATA_AVAILABLE,
    HANDLE_DATA_RECEIVED,
    MAX_EVENT
} evt_type_t;

typedef enum evt_action_response_e
{
    EVENT_SUCCESS = 0,
    EVENT_QUEUE_FULL,
    EVENT_QUEUE_EMPTY,
    EVENT_CONTEXT_NOT_INITIALIZED,
    EVENT_HANDLER_ERROR,
    EVENT_NO_EVENTS
} evt_action_response_t;

typedef uint8_t (*event_handler)(void *userData);

typedef struct event_s {
    evt_type_t idx;
    event_handler handler;
    uint64_t event_time;
    void *usrData;
} event_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief Insert an event into the event queue.
 *
 * This function will assert if the context has not been initialized
 *
 *
 * @param event Event to be inserted into the queue.
 * @return Will return EVENT_SUCCESS if the an event was added or EVENT_QUEUE_FULL
 *         if there was no space available to add event.
 */
evt_action_response_t serdes_push_event(evt_type_t event_type, void *userData);

/**
 * @brief Retrieve event from the queue if there are any to be retrieved
 *
 * This function will assert if the context has not been initialized
 *
 * @return EVENT_SUCCESS if there is an event to be retrieved; EVENT_QUEUE_EMPTY
 *         if there is not event to be retrieved.
 */
evt_action_response_t serdes_dispatch_event();

/** Initialize OS timer and the module */
void serdes_event_init();

/**
 * @brief Reset the event context.  This will reset the event queue
 *
 * @return EVENT_SUCCESS on completion
 */
evt_action_response_t serdes_event_reset();

void serdes_register_handler(evt_type_t event, event_handler handler);

#endif
