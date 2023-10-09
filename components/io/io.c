#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "io.h"
#include "esp_log.h"

#define tag "IO"

static void io_task(void* arg) {
    ESP_LOGD(tag, "In IO task.");
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

esp_err_t init_io_task(void)
{
    esp_err_t rc = ESP_OK;

    ESP_LOGI(tag, "Starting the IO component...");

    /* Start transmit task */
    if (pdPASS != xTaskCreate(io_task, "IO_task",
                              2048, NULL,
                              1, NULL)) {
        ESP_LOGE(tag, "failed to start IO task, rc=%d", rc);
        return ESP_FAIL;
    }

    return rc;
}
