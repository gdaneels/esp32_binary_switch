#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "display.h"
#include "esp_log.h"

#define tag "DISPLAY"

static void display_task(void* arg) {
    ESP_LOGD(tag, "In display task.");

    QueueHandle_t message_queue = (QueueHandle_t)arg;
    while(1) {
        uint8_t number = 0;
        if (xQueueReceive(message_queue, &number, (TickType_t)10)) { 
            ESP_LOGI(tag, "Received message: %"PRIu8"", number);
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
