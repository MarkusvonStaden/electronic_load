#include "adc.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"

#define SPI_MOSI   5
#define SPI_CLK    6
#define SPI_DAC_CS 7
#define SPI_SPEED  20 * 1000 * 1000

static const char *TAG = "DAC";

int             cal_input_voltage, cal_shunt_voltage;
static uint16_t data;
double          value;
uint8_t         mode;

spi_device_handle_t spi_handle;

spi_transaction_t trans = {
    .length = 16,
    .rx_buffer = NULL,
    .tx_buffer = &data,
};

void setup_dac(void) {
    spi_bus_config_t spi_bus_cfg = {
        .mosi_io_num = SPI_MOSI,
        .sclk_io_num = SPI_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .miso_io_num = -1,
    };

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 2 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = SPI_DAC_CS,
        .queue_size = 3,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi_bus_cfg, SPI_DMA_CH_AUTO));
    ESP_LOGD(TAG, "Bus initialized");
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev_cfg, &spi_handle));
    ESP_LOGD(TAG, "Device Added to Bus");
}

void update_hardware(void) {
    read_adc();
    float voltage;
    switch (mode) {
        case 0:  // DAC Disabled
            data = 0;
            break;

        case 1:  // Constant Current
            data = SPI_SWAP_DATA_TX(((uint16_t)(value / 4.096 * 4096)) | (1 << 12), 16);
            break;

        case 2:  // Constant Power
            voltage = value / (cal_input_voltage);
            data = SPI_SWAP_DATA_TX(((uint16_t)(voltage / 4.096 * 4096)) | (1 << 12), 16);
            break;

        case 3:  // Constant Resistance
            voltage = (cal_input_voltage) / value;
            ESP_LOGI(TAG, "voltage");
            data = SPI_SWAP_DATA_TX(((uint16_t)(voltage / 4.096 * 4096)) | (1 << 12), 16);
            break;

        default:
            data = 0;
            ESP_LOGE(TAG, "Invalid Mode");
            break;
    }
    ESP_ERROR_CHECK(spi_device_acquire_bus(spi_handle, portMAX_DELAY));
    ESP_ERROR_CHECK(spi_device_transmit(spi_handle, &trans));
    spi_device_release_bus(spi_handle);
}
