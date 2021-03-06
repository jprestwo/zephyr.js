// Copyright (c) 2016, Intel Corporation.

// Zephyr includes
#include <zephyr.h>

#include <device.h>
#include <init.h>
#include <adc.h>

#include "zjs_ipm.h"

#if defined(CONFIG_STDOUT_CONSOLE)
#include <stdio.h>
#define PRINT           printf
#else
#include <misc/printk.h>
#define PRINT           printk
#endif

/* specify delay between greetings (in ms); compute equivalent in ticks */

#define SLEEPTIME  1100
#define SLEEPTICKS (SLEEPTIME * sys_clock_ticks_per_sec / 800)

#define ADC_DEVICE_NAME "ADC_0"

/*
 * The analog input pin and channel number mapping
 * for Arduino 101 board.
 * A0 Channel 10
 * A1 Channel 11
 * A2 Channel 12
 * A3 Channel 13
 * A4 Channel 14
 * A5 Channel 15
 */
#define A0 10
#define A1 11
#define A2 12
#define A3 13
#define A4 14
#define A5 15
#define BUFFER_SIZE 4

static struct device* adc_dev;
static uint32_t pin_values[6] = {};
static uint32_t pin_send_updates[6] = {};

static uint8_t seq_buffer[BUFFER_SIZE];

int ipm_send_msg(uint32_t id, bool block, uint32_t type, uint32_t pin, uint32_t value) {
    struct zjs_ipm_message msg;
    msg.block = block;
    msg.type = type;
    msg.pin = pin;
    msg.value = value;
    return zjs_ipm_send(id, &msg, sizeof(msg));
}

uint32_t pin_read(uint8_t pin)
{
    uint8_t* buf = seq_buffer;

    struct adc_seq_entry entry = {
        .sampling_delay = 12,
        .channel_id = pin,
        .buffer = seq_buffer,
        .buffer_length = BUFFER_SIZE,
    };

    struct adc_seq_table entry_table = {
        .entries = &entry,
        .num_entries = 1,
    };

    if (!adc_dev) {
       PRINT("ARC - ADC device not found\n");
       return 0;
    }

    if (adc_read(adc_dev, &entry_table) != 0) {
        PRINT("ARC - couldn't read from pin %d\n", pin);
        return 0;
    }

    uint32_t raw_value = *((uint32_t *) buf);
    return raw_value;
}

void ipm_msg_receive_callback(void *context, uint32_t id, volatile void *data)
{
    struct zjs_ipm_message *msg = (struct zjs_ipm_message*) data;
    uint32_t pin = msg->pin;
    uint32_t reply_type = 0;
    uint32_t reply_value = 0;

    if (id != MSG_ID_AIO)
        return;

    if (msg->type == TYPE_AIO_OPEN) {
        if (pin < A0 || pin > A5) {
            PRINT("ARC - pin #%d out of range\n", pin);
            reply_type = TYPE_AIO_OPEN_FAIL;
        } else {
            reply_type = TYPE_AIO_OPEN_SUCCESS;
        }
    } else if (msg->type == TYPE_AIO_PIN_READ) {
        if (pin < A0 || pin > A5) {
            PRINT("ARC - pin #%d out of range\n", pin);
            reply_type = TYPE_AIO_PIN_READ_FAIL;
        }

        reply_type = TYPE_AIO_PIN_READ_SUCCESS;
        /*
         * FIX ME!
         * inside the interrupt, cannot read from the ADC pins
         * only from the main loop thread
         */
        //reply_value = pin_read(A0);
        reply_value = pin_values[pin-A0];
    } else if (msg->type == TYPE_AIO_PIN_ABORT) {
        PRINT("ARC - AIO abort() not supported\n");
        reply_type = TYPE_AIO_PIN_ABORT_SUCCESS;
    } else if (msg->type == TYPE_AIO_PIN_CLOSE) {
        if (pin < A0 || pin > A5) {
            PRINT("ARC - pin #%d out of range\n", pin);
            reply_type = TYPE_AIO_PIN_CLOSE_FAIL;
        } else {
            PRINT("ARC - AIO pin #%d is closed\n", pin);
            reply_type = TYPE_AIO_PIN_CLOSE_SUCCESS;
        }
    } else if (msg->type == TYPE_AIO_PIN_SUBSCRIBE) {
        if (pin < A0 || pin > A5) {
            PRINT("ARC - pin #%d out of range\n", pin);
            reply_type = TYPE_AIO_PIN_SUBSCRIBE_FAIL;
        } else {
            pin_send_updates[pin-A0] = 1;
            reply_type = TYPE_AIO_PIN_SUBSCRIBE_SUCCESS;
        }
    } else if (msg->type == TYPE_AIO_PIN_UNSUBSCRIBE) {
        if (pin < A0 || pin > A5) {
            PRINT("ARC - pin #%d out of range\n", pin);
            reply_type = TYPE_AIO_PIN_UNSUBSCRIBE_FAIL;
        } else {
            pin_send_updates[pin-A0] = 0;
            reply_type = TYPE_AIO_PIN_UNSUBSCRIBE_SUCCESS;
        }
    } else {
        PRINT("ARC - Unsupported message id %d\n", id);
    }

    ipm_send_msg(MSG_ID_AIO, msg->block, reply_type, pin, reply_value);
}

#ifdef CONFIG_MICROKERNEL
void mainloop(void)
#else
void main(void)
#endif
{
    PRINT("ARC -------------------------------------- \n");
    PRINT("ARC - AIO processor from sensor core (ARC)!\n");

    zjs_ipm_init();
    zjs_ipm_register_callback(MSG_ID_AIO, ipm_msg_receive_callback);

    adc_dev = device_get_binding(ADC_DEVICE_NAME);
    adc_enable(adc_dev);

    while (1) {
        /*
         * mainloop just reads all the values from all ADC pins
         * and stores them in the array
         */
        for (int i=0; i<=5; i++) {
            pin_values[i] = pin_read(i+A0);
            if (pin_send_updates[i]) {
                ipm_send_msg(MSG_ID_AIO, 0, TYPE_AIO_PIN_EVENT_VALUE_CHANGE,
                              i+A0, pin_values[i]);
                task_sleep(10);
            }

            /* FIX BUG
             * read value will read the old value
             * from the previous pin when switching pins
             * too fast, adding a sleep fix this issue
             */
            task_sleep(1);
        }

        task_sleep(SLEEPTICKS);
    }

    adc_disable(adc_dev);
}
