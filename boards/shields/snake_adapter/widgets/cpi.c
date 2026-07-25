/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <lvgl.h>

#include "cpi.h"
#include "helpers/display.h"

#define CPI_REFRESH_INTERVAL_MS 100

static bool cpi_widget_running;
static uint16_t *scaled_bitmap_cpi_font;
static int32_t last_rendered_cpi = -1;
static char last_rendered_label[9];
static uint16_t last_rendered_x;

static Slot cpi_slot;
static uint16_t cpi_font_scale = 4;
static uint16_t cpi_x;
static uint16_t cpi_y = 11;

#if DT_HAS_COMPAT_STATUS_OKAY(zmk_behavior_sensor_attr_cycle)

#if DT_HAS_CHOSEN(zmk_cpi_cycle)
#define CPI_CYCLE_NODE DT_CHOSEN(zmk_cpi_cycle)
#else
#define CPI_CYCLE_NODE DT_INST(0, zmk_behavior_sensor_attr_cycle)
#endif

/*
 * zmk-behavior-sensor-attr-cycle currently has no public state getter or change
 * event. Keep these definitions in sync with its data structure so the widget
 * can display the value selected by the behavior, including restored settings.
 */
struct sensor_attr_cycle_persistant_state {
    uint8_t index;
};

struct sensor_attr_cycle_data {
    const struct device *dev;
#if IS_ENABLED(CONFIG_SETTINGS)
    struct k_work_delayable load_work;
    struct k_work_delayable save_work;
#endif
    struct sensor_attr_cycle_persistant_state state;
};

static const int32_t cpi_cycle_values[] = DT_PROP(CPI_CYCLE_NODE, values);

static int32_t get_cpi(void) {
    const struct device *dev = DEVICE_DT_GET(CPI_CYCLE_NODE);
    const struct sensor_attr_cycle_data *data = dev->data;

    if (!device_is_ready(dev) || data->state.index >= ARRAY_SIZE(cpi_cycle_values)) {
        return cpi_cycle_values[0];
    }

    return cpi_cycle_values[data->state.index];
}

#else

static int32_t get_cpi(void) {
    return CONFIG_CPI_VALUE;
}

#endif

void print_cpi(void) {
    if (!cpi_widget_running || cpi_slot.number == SLOT_NUMBER_NONE) {
        return;
    }

    int32_t cpi = get_cpi();
    char label[9];
    int label_length = snprintf(label, sizeof(label), "CPI:%d", cpi);

    if (label_length <= 0 || label_length >= (int)sizeof(label)) {
        return;
    }

    uint16_t gap = cpi_font_scale >= 7 ? 3 : 2;
    uint16_t character_width = (3 * cpi_font_scale) + gap;
    uint16_t slot_width = (get_slot_mode() == SLOT_MODE_5 &&
                           cpi_slot.number == SLOT_NUMBER_2)
                              ? 240
                              : 120;
    uint16_t text_width = (character_width * label_length) - gap;
    uint16_t x = cpi_x + ((slot_width - text_width) / 2);

    if (last_rendered_label[0] != '\0') {
        uint8_t previous_length = strlen(last_rendered_label);
        print_char_array(scaled_bitmap_cpi_font, last_rendered_label, last_rendered_x,
                         cpi_y, cpi_font_scale, get_wpm_font_bg_color(),
                         get_wpm_font_bg_color(), FONT_SIZE_3x5, gap,
                         previous_length, previous_length);
    }

    print_char_array(scaled_bitmap_cpi_font, label, x, cpi_y, cpi_font_scale,
                     get_wpm_font_color(), get_wpm_font_bg_color(), FONT_SIZE_3x5,
                     gap, label_length, label_length);
    last_rendered_cpi = cpi;
    strcpy(last_rendered_label, label);
    last_rendered_x = x;
}

static void cpi_refresh_timer(lv_timer_t *timer) {
    ARG_UNUSED(timer);

    if (cpi_widget_running && get_cpi() != last_rendered_cpi) {
        print_cpi();
    }
}

void zmk_widget_cpi_init(void) {
    cpi_slot = get_slot_by_name(SLOT_NAME_CPI);
    if (cpi_slot.number == SLOT_NUMBER_NONE) {
        return;
    }

    if (get_slot_mode() == SLOT_MODE_5 && cpi_slot.number == SLOT_NUMBER_2) {
        cpi_font_scale = 7;
        cpi_x = 0;
        cpi_y = 20;
    } else {
        cpi_x = cpi_slot.x;
        cpi_y += cpi_slot.y;
    }

    uint16_t bitmap_size = (3 * cpi_font_scale) * (5 * cpi_font_scale);
    scaled_bitmap_cpi_font = k_malloc(bitmap_size * 2 * sizeof(uint16_t));
    lv_timer_create(cpi_refresh_timer, CPI_REFRESH_INTERVAL_MS, NULL);
}

void start_cpi_status(void) {
    cpi_widget_running = true;
}

void stop_cpi_status(void) {
    cpi_widget_running = false;
}
