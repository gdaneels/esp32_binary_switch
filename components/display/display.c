#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "display.h"
#include "io.h"
#include "esp_log.h"

#define QUEUE_BLOCKING_TIME_MS 10

#define tag "DISPLAY"

static void display_task(void* arg) {
    ESP_LOGD(tag, "In display task.");

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
