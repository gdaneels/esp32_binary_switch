#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "display.h"
#include "esp_log.h"

#define tag "DISPLAY"

static void display_task(void* arg) {
    ESP_LOGD(tag, "In display task.");
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

esp_err_t init_display_task(QueueHandle_t message_queue)
{
    esp_err_t rc = ESP_OK;

    ESP_LOGI(tag, "Starting the DISPLAY component...");

    /* Start transmit task */
    if (pdPASS != xTaskCreate(display_task, "display_task",
                              2048, NULL,
                              1, NULL)) {
        ESP_LOGE(tag, "failed to start the display task, rc=%d", rc);
        return ESP_FAIL;
    }

    return rc;
}
