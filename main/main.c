#include <memory.h>
#include <stdio.h>

#include "adc.h"
#include "dac.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "webserver.h"

static const char   *TAG = "MAIN";
static TimerHandle_t timer_handle = NULL;

void app_main(void) {
    setup_dac();
    setup_adc();
    setup_server();

    timer_handle = xTimerCreate(
        "read ADC",
        50 / portTICK_PERIOD_MS,
        pdTRUE,
        NULL,
        update_hardware);

    xTimerStart(timer_handle, portMAX_DELAY);
}
