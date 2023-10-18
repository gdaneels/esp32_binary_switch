#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "display.h"

#include "io.h"
#include "esp_log.h"
#include "ssd1306.h"

#define QUEUE_BLOCKING_TIME_MS 10

#define tag "DISPLAY"

#define I2C_MASTER_SCL_IO 10        /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 11        /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0    /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency */

static ssd1306_handle_t ssd1306_dev = NULL;

static void display_task(void* arg) {
    ESP_LOGD(tag, "In display task.");

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    ssd1306_dev = ssd1306_create(I2C_MASTER_NUM, SSD1306_I2C_ADDRESS);
    ssd1306_refresh_gram(ssd1306_dev);
    ssd1306_clear_screen(ssd1306_dev, 0x00);

    char data_str[10] = {0};
    sprintf(data_str, "C STR");
    ssd1306_draw_string(ssd1306_dev, 70, 16, (const uint8_t *)data_str, 16, 1);
    ssd1306_refresh_gram(ssd1306_dev);

    QueueHandle_t message_queue = (QueueHandle_t)arg;
    while(1) {
        IOMessage msg = { 0 };
        if (xQueueReceive(message_queue, &msg, (TickType_t)pdMS_TO_TICKS(QUEUE_BLOCKING_TIME_MS))) { 
            ESP_LOGI(tag, "Received message: %"PRIu8"", msg.value);
        }
    }
}

esp_err_t init_display_task(QueueHandle_t message_queue)
{
    esp_err_t rc = ESP_OK;

    ESP_LOGI(tag, "Starting the DISPLAY component...");

    /* Start transmit task */
    if (pdPASS != xTaskCreate(display_task, "display_task",
                              2048, message_queue,
                              1, NULL)) {
        ESP_LOGE(tag, "failed to start the display task, rc=%d", rc);
        return ESP_FAIL;
    }

    return rc;
}
