#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "display.h"
#include "io.h"

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

#define SWITCH_1 GPIO_NUM_3
#define SWITCH_2 GPIO_NUM_4
#define SWITCH_3 GPIO_NUM_18
#define SWITCH_4 GPIO_NUM_19
#define BUTTON GPIO_NUM_5

typedef struct {
    gpio_config_t config;
    uint8_t value;
} IOConfig;

IOConfig io_config[1] = {
    {
        .config = {
            .mode = GPIO_MODE_INPUT,
            .intr_type = GPIO_INTR_DISABLE,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .pin_bit_mask = ((1ULL<<SWITCH_1) | (1ULL<<SWITCH_2) | (1ULL<<SWITCH_3) | (1ULL<<SWITCH_4)),
        },
        .value = 0
    }
};

void init_gpio(void) {
    gpio_config(&io_config[0].config);
}

void app_main(void)
{
    init_display();
    init_io();
    init_gpio();
    size_t cnt = 0;
    while (1) {
        int level_switch_1 = gpio_get_level(SWITCH_1);
        printf("%zu: Level of pin %u is: %d.\n", cnt, GPIO_NUM_3, level_switch_1);
        int level_switch_2 = gpio_get_level(SWITCH_2);
        printf("%zu: Level of pin %u is: %d.\n", cnt, GPIO_NUM_4, level_switch_2);
        int level_switch_3 = gpio_get_level(SWITCH_3);
        printf("%zu: Level of pin %u is: %d.\n", cnt, GPIO_NUM_18, level_switch_3);
        int level_switch_4 = gpio_get_level(SWITCH_4);
        printf("%zu: Level of pin %u is: %d.\n", cnt, GPIO_NUM_19, level_switch_4);
        uint8_t bitmap = ((level_switch_4<<3) | (level_switch_3<<2) | (level_switch_2<<1) | (level_switch_1));
        printf("Current bitmap: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(bitmap));
        vTaskDelay(pdMS_TO_TICKS(100));
        cnt += 2;
    }
}
