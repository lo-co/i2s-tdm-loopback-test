#ifndef _I2S_H_
#define _I2S_H_

#include "fsl_i2s.h"
#include "fsl_i2s_dma.h"

#ifdef defined(__cplusplus)
extern "C" {
#endif

/** Callback for I2S DMA */
typedef void (*i2s_cb_func)(I2S_Type *,i2s_dma_handle_t *,status_t ,void *);

typedef struct i2s_context_s
{
    I2S_Type *base;
    i2s_dma_handle_t *dma_handle;

} i2s_context_t;

typedef enum flexcomm_port_e {
    FLEXCOMM_1 = 0,
    FLEXCOMM_2,
    FLEXCOMM_3,
    FLEXCOMM_4,
    FLEXCOMM_5,
    FLEXCOMM_6,
    FLEXCOMM_7,
    FLEXCOMM_8,
} flexcomm_port_t;

/** I2S configuration structure */
typedef struct i2s_init_s
{
    flexcomm_port_t flexcomm_bus;
    bool    is_transmit;
    bool    is_master;
    uint8_t active_channels;
    uint32_t sample_rate;
    uint32_t datalength;
    bool is_initialized;
    i2s_cb_func callback;
    i2s_context_t *context;
} i2s_init_t;

static void i2s_init(i2s_init_t config);

static void i2s_deinit(i2s_init_t config);

#ifdef defined(__cplusplus)
}
#endif

#endif