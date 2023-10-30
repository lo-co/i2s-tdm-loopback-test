#ifndef _SERDES_EVENT_H
#define _SERDES_EVENT_H

typedef enum evt_type_e
{
    RCV_INTERRUPT = 0,
    START_TXMIT = 1,
    START_RCV = 2,
    RCV_TXMIT = 3,
} evt_type_t;

typedef struct event_s {
    uint8_t idx;
    uint64_t evt_times[5];
} event_t;

void serdes_update_evt_times(evt_type_t evt_type, event_t *events);

/** Initialize OS timer and the module */
void serdes_event_init();

#endif
