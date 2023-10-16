#include <stdio.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "display.h"
#include "io.h"

#define tag "MAIN"

#define QUEUE_SIZE 10
static QueueHandle_t message_queue;

void app_main(void)
{
    message_queue = xQueueCreate(QUEUE_SIZE, sizeof(IOMessage));
    if (!message_queue) {
        ESP_LOGE(tag, "Message queue could not be initialized.");
    }
    init_io_task(message_queue);
    init_display_task(message_queue);
}
