#include <memory.h>
#include <stdio.h>

#include "dac.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MAIN";

void app_main(void) {
    setup_dac();
    update_dac(2.5);
}
