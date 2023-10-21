#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "io.h"
#include "esp_log.h"
#include "esp_timer.h"

/*
 * macro to log a byte in binary
 * borrowed from https://stackoverflow.com/a/3208376
 */
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 


// #define SWITCH_1 GPIO_NUM_3
// #define SWITCH_2 GPIO_NUM_4
// #define SWITCH_3 GPIO_NUM_18
// #define SWITCH_4 GPIO_NUM_19
// #define BUTTON_CONFIRMATION GPIO_NUM_5

#define SWITCH_1 GPIO_NUM_23
#define SWITCH_2 GPIO_NUM_22
#define SWITCH_3 GPIO_NUM_21
#define SWITCH_4 GPIO_NUM_20
#define BUTTON_CONFIRMATION GPIO_NUM_15

#define QUEUE_BLOCKING_TIME_MS 10

#define tag "IO"

typedef struct {
    gpio_config_t config;
    uint8_t value;
} IOConfig;

IOConfig io_config[2] = {
    {
        .config = {
            .mode = GPIO_MODE_INPUT,
            .intr_type = GPIO_INTR_DISABLE,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .pin_bit_mask = ((1ULL<<SWITCH_1) | (1ULL<<SWITCH_2) | (1ULL<<SWITCH_3) | (1ULL<<SWITCH_4)),
        },
        .value = 0
    },
    {
        .config = {
            .mode = GPIO_MODE_INPUT,
            .intr_type = GPIO_INTR_DISABLE,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .pin_bit_mask = (1ULL<<BUTTON_CONFIRMATION),
        },
        .value = 0
    }
};

/** debounce defines */
uint16_t debounce_delay_us = 50000; /** 50 ms */
uint32_t long_press_us = 4000000; /** 2000 ms */

static void send_message(QueueHandle_t message_queue, IOMessage* msg) { 
    if (xQueueSend(message_queue, msg, (TickType_t)pdMS_TO_TICKS(QUEUE_BLOCKING_TIME_MS))) { 
        ESP_LOGI(tag, "Sent message: %"PRIu8"", msg->value);
    } else {
        ESP_LOGE(tag, "Failed to send message with type %d and value %"PRIu8".", msg->type, msg->value);
    }
}

static void check_confirmation_button(QueueHandle_t message_queue, IOMessage* msg) {
    static int64_t last_debounce_time_button_confirmation_us = 0;
    static int last_level_button_confirmation = 1;
    static int prev_state_button_confirmation = 1;
    static bool long_pressed = false;

    int level_button_confirmation = gpio_get_level(BUTTON_CONFIRMATION);
    if (level_button_confirmation != last_level_button_confirmation) {
        last_debounce_time_button_confirmation_us = esp_timer_get_time();
    }

    if (esp_timer_get_time() - last_debounce_time_button_confirmation_us > debounce_delay_us) {
        if (level_button_confirmation != prev_state_button_confirmation) {
            /** button was released */
            if (level_button_confirmation == 1) {
                /** in case of short press, send when released */
                if (!long_pressed) {
                    ESP_LOGI(tag, "Button was released and short pressed.");
                    msg->type = IO_VALUE;
                    send_message(message_queue, msg);
                }
                long_pressed = false; /** reset long press state */
            }
        } else if (level_button_confirmation == prev_state_button_confirmation && level_button_confirmation == 0) {
            if (esp_timer_get_time() - last_debounce_time_button_confirmation_us > long_press_us) { 
                /** check for long_press bool, so this can only be triggered once during the same press */
                if (!long_pressed) {
                    ESP_LOGI(tag, "Button hold down for long press.");
                    long_pressed = true;
                    /** in case of long press, send when still pressing */
                    msg->type = IO_RESET;
                    send_message(message_queue, msg);
                }
            }
        }
        prev_state_button_confirmation = level_button_confirmation;
    }

    last_level_button_confirmation = level_button_confirmation;
}

static void io_task(void* arg) {
    ESP_LOGD(tag, "In IO task.");

    QueueHandle_t message_queue = (QueueHandle_t) arg;

    gpio_config(&io_config[0].config);
    gpio_config(&io_config[1].config);

    IOMessage msg = { 0 };
    while (1) {
        int level_switch_1 = gpio_get_level(SWITCH_1);
        int level_switch_2 = gpio_get_level(SWITCH_2);
        int level_switch_3 = gpio_get_level(SWITCH_3);
        int level_switch_4 = gpio_get_level(SWITCH_4);
        msg.value = ((level_switch_4<<3) | (level_switch_3<<2) | (level_switch_2<<1) | (level_switch_1));
        // printf("Current bitmap: "BYTE_TO_BINARY_PATTERN": %u\n", BYTE_TO_BINARY(bitmap), bitmap);
       
        check_confirmation_button(message_queue, &msg);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

esp_err_t init_io_task(QueueHandle_t message_queue)
{
    esp_err_t rc = ESP_OK;

    ESP_LOGI(tag, "Starting the IO component...");

    /* Start transmit task */
    if (pdPASS != xTaskCreate(io_task, "IO_task",
                              2048, message_queue,
                              1, NULL)) {
        ESP_LOGE(tag, "failed to start IO task, rc=%d", rc);
        return ESP_FAIL;
    }

    return rc;
}
