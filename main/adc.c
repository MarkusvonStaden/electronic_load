#include "driver/adc.h"

#include "esp_adc_cal.h"
#include "esp_err.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ADC1_EXAMPLE_CHAN0 ADC1_CHANNEL_4
#define ADC2_EXAMPLE_CHAN0 ADC1_CHANNEL_3

// ADC Attenuation
#define ADC_EXAMPLE_ATTEN       ADC_ATTEN_DB_11
#define ADC_EXAMPLE_CALI_SCHEME ESP_ADC_CAL_VAL_EFUSE_TP

static int input_voltage, shunt_voltage;
int        cal_input_voltage, cal_shunt_voltage;

static esp_adc_cal_characteristics_t adc1_chars;

void read_adc(void) {
    input_voltage = adc1_get_raw(ADC1_EXAMPLE_CHAN0);
    shunt_voltage = adc1_get_raw(ADC2_EXAMPLE_CHAN0);

    cal_input_voltage = esp_adc_cal_raw_to_voltage(input_voltage, &adc1_chars);
    cal_shunt_voltage = esp_adc_cal_raw_to_voltage(shunt_voltage, &adc1_chars);
}

void setup_adc(void) {
    esp_err_t ret = ESP_OK;
    uint32_t  voltage = 0;
    ESP_ERROR_CHECK(esp_adc_cal_check_efuse(ADC_EXAMPLE_CALI_SCHEME));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_EXAMPLE_ATTEN, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

    // ADC config
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_EXAMPLE_CHAN0, ADC_EXAMPLE_ATTEN));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC2_EXAMPLE_CHAN0, ADC_EXAMPLE_ATTEN));
}
