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
    bool sw_pressed;
    bool sw_pressed_prev;
};
