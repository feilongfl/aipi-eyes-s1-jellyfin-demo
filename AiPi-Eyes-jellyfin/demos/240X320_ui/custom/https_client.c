#include "FreeRTOS.h"
#include "custom.h"
#include "queue.h"
#include "task.h"
#include <lwip/api.h>
#include <lwip/arch.h>
#include <lwip/inet.h>
#include <lwip/opt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

extern TaskHandle_t https_Handle;
extern xQueueHandle queue;
static const char* REQUEST = "GET " "%s" " HTTP/1.0\r\n"
"Host: " "%s" "\r\n"
"User-Agent: Eyes \r\n"
"\r\n";

void https_jellyfin_task(void *arg) {
  while (1) {
    vTaskDelay(50 / portTICK_RATE_MS);
  }
}