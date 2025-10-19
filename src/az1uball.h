#pragma once

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/mutex.h>

/* Bit Masks */
#define MSK_SWITCH_STATE    0b10000000

struct az1uball_config {
    struct i2c_dt_spec i2c;
};

struct az1uball_data {
    const struct device *dev;
    struct k_work_delayable work;
    struct k_mutex data_lock;
    bool sw_pressed;
    bool sw_pressed_prev;
    atomic_t x_buffer;
    atomic_t y_buffer;
    uint32_t last_interrupt_time;
    uint32_t previous_interrupt_time;
    int previous_x;
    int previous_y;
    int smoothed_x;
    int smoothed_y;
};
