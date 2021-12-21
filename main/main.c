/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "i2c-simple-example";
#define I2C_MASTER_TIMEOUT_MS 1000

#define I2C_MASTER_SCL_IO GPIO_NUM_6 /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO GPIO_NUM_7 /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 0             /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 400000    /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0  /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0  /*!< I2C master doesn't need buffer */
#define MCP4725_ADDR 0x60

// mcp4725_write(uint16_t *data)
// {
//     return i2c_master_write_to_device(I2C_MASTER_NUM, MCP4725_ADDR, data, 2, portTICK_RATE_MS);
// }

void dac(void)
{
    int i2c_master_port = 0;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ, // select frequency specific to your project
    };

    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);

    const static uint16_t mask = 0x0FFF;
    uint16_t data = 4096;
    const unsigned char value = data && mask;

    while (true)
    {
        i2c_master_write_to_device(I2C_MASTER_NUM, MCP4725_ADDR, &value, 2, portTICK_RATE_MS);
        // ESP_ERROR_CHECK(mcp4725_write(&value));
        ESP_LOGI(TAG, "Still running");

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    xTaskCreate(dac, "dac", 2048, NULL, 1, NULL);
}
