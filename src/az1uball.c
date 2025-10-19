/*
 * Copyright (c) 2022 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT palette_az1uball

#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <math.h>
#include <stdlib.h>
#include "az1uball.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(az1uball, LOG_LEVEL_DBG);

#define POLL_INTERVAL K_MSEC(10)   // 通常時: 10ms (100Hz)

/* Execution functions for asynchronous work */
static void az1uball_work_handler(struct k_work_delayable *work)
{
    struct az1uball_data *data = CONTAINER_OF(work, struct az1uball_data, work);
    const struct az1uball_config *config = data->dev->config;
    uint8_t buf[5];
    int ret;

    // Read data from I2C
    ret = i2c_read_dt(&config->i2c, buf, sizeof(buf));
    if (ret < 0) {
        LOG_ERR("Failed to read movement data from AZ1YBALL: %d", ret);
        return;
    }

    /* Calculate deltas */
    int16_t delta_x = (int16_t)buf[2] - (int16_t)buf[3]; // RIGHT - LEFT
    int16_t delta_y = (int16_t)buf[1] - (int16_t)buf[0]; // DOWN - UP

    /* Report movement immediately if non-zero */
    if (delta_x != 0 || delta_y != 0) {
        /* Report relative X movement */
        if (delta_x != 0) {
            ret = input_report_rel(data->dev, INPUT_REL_X, delta_x, true, K_NO_WAIT);
            if (ret < 0) {
                LOG_ERR("Failed to report delta_x: %d", ret);
            } else {
                LOG_DBG("Reported delta_x: %d", delta_x);
            }
        }

        /* Report relative Y movement */
        if (delta_y != 0) {
            ret = input_report_rel(data->dev, INPUT_REL_Y, delta_y, true, K_NO_WAIT);
            if (ret < 0) {
                LOG_ERR("Failed to report delta_y: %d", ret);
            } else {
                LOG_DBG("Reported delta_y: %d", delta_y);
            }
        }
    }

    /* Update switch state */
    data->sw_pressed = (buf[4] & MSK_SWITCH_STATE) != 0;

    /* Report switch state if it changed */
    if (data->sw_pressed != data->sw_pressed_prev) {
        ret = input_report_key(data->dev, INPUT_BTN_0, data->sw_pressed ? 1 : 0, true, K_NO_WAIT);
        if (ret < 0) {
            LOG_ERR("Failed to report key");
        } else {
            LOG_DBG("Reported key");
        }

        LOG_DBG("Reported switch state: %d", data->sw_pressed);

        data->sw_pressed_prev = data->sw_pressed;
    }

    k_work_reschedule(&data->work, POLL_INTERVAL);
}

/* Initialization of AZ1UBALL */
static int az1uball_init(const struct device *dev)
{
    const struct az1uball_config *config = dev->config;
    struct az1uball_data *data = dev->data;
    int ret;

    data->dev = dev;
    data->sw_pressed_prev = false;

    /* Check if the I2C device is ready */
    if (!device_is_ready(config->i2c.bus)) {
        LOG_ERR("I2C bus device is not ready: 0x%x", config->i2c.addr);
        return -ENODEV;
    }

    /* Set turbo mode */
    uint8_t cmd = 0x91;
    ret = i2c_write_dt(&config->i2c, &cmd, sizeof(cmd));
    if (ret < 0) {
        LOG_ERR("Failed to set turbo mode");
        return ret;
    }

    k_work_init_delayable(&data->work, az1uball_work_handler);
    k_work_schedule(&data->work, POLL_INTERVAL);

    return 0;
}

#define AZ1UBALL_DEFINE(n)                                             \
    static struct az1uball_data az1uball_data_##n;                     \
    static const struct az1uball_config az1uball_config_##n = {        \
        .i2c = I2C_DT_SPEC_INST_GET(n),                                \
    };                                                                 \
    DEVICE_DT_INST_DEFINE(n,                                           \
                          az1uball_init,                               \
                          NULL,                                        \
                          &az1uball_data_##n,                          \
                          &az1uball_config_##n,                        \
                          POST_KERNEL,                                 \
                          CONFIG_INPUT_INIT_PRIORITY,                  \
                          NULL);

DT_INST_FOREACH_STATUS_OKAY(AZ1UBALL_DEFINE)