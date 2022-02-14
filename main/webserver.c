#include <esp_http_server.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <string.h>

#include "adc.h"
#include "cJSON.h"
#include "dac.h"
#include "esp_err.h"
#include "esp_log.h"

#define TAG "WEBSERVER"

static httpd_handle_t server = NULL;

void startAP(void) {
    ESP_LOGI(TAG, "Setup Start AP");

    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "Electronic Load",
            .channel = 1,
            .max_connection = 8,
            .authmode = WIFI_AUTH_OPEN},
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void stop_webserver(httpd_handle_t server) {
    ESP_LOGI(TAG, "Stop Server");
    if (server) {
        httpd_stop(server);
        ESP_LOGI(TAG, "Server stopped");
    } else {
        ESP_LOGE(TAG, "Server is NULL");
    }
}

esp_err_t get_handler(httpd_req_t* req) {
    cJSON* root;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "current", cal_input_voltage);
    cJSON_AddNumberToObject(root, "voltage", cal_shunt_voltage);

    char* json_response = cJSON_Print(root);
    cJSON_Delete(root);
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, json_response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t post_handler(httpd_req_t* req) {
    ESP_LOGI(TAG, "Post Handler invoked");
    char   content[100];
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    const char resp[] = "OK";

    // TODO JSON Parsing
    cJSON* root = cJSON_Parse(&content);
    mode = cJSON_GetObjectItemCaseSensitive(root, "mode")->valueint;
    value = cJSON_GetObjectItemCaseSensitive(root, "value")->valuedouble;
    ESP_LOGE(TAG, "New mode: %i Value %f", mode, value);
    cJSON_Delete(root);

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, resp);
    return ESP_OK;
}

httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL};

httpd_uri_t uri_post = {
    .uri = "/",
    .method = HTTP_POST,
    .handler = post_handler,
    .user_ctx = NULL};
/* Function for starting the webserver */
static httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &uri_get));
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &uri_post));
        ESP_LOGI(TAG, "URI handler registred");
        return server;
    }
    /* If server failed to start, handle will be NULL */
    return NULL;
}

void setup_server(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    startAP();
    server = start_webserver();
}