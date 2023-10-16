#include <freertos/queue.h>
#include "esp_check.h"

typedef enum {
    IO_VALUE,
    IO_RESET
} IOMessageType;

typedef struct {
    IOMessageType type;
    uint8_t value;
} IOMessage;

esp_err_t init_io_task(QueueHandle_t message_queue);
