#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "protocol_examples_common.h"

#include <esp_http_server.h>

#include "data_sample.h"

#include "cJSON.h"

/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */

static const char *TAG = "example";

/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Serialize data");
    
    cJSON *root;
    root = cJSON_CreateObject();
    
    cJSON_AddNumberToObject(root, "acc_ax",      last_sample.acc.ax);
    cJSON_AddNumberToObject(root, "acc_ay",      last_sample.acc.ay);
    cJSON_AddNumberToObject(root, "acc_az",      last_sample.acc.az);
    cJSON_AddNumberToObject(root, "acc_mag_val", last_sample.acc.mag_val);
    cJSON_AddNumberToObject(root, "acc_yaw",     last_sample.acc.yaw);
    cJSON_AddNumberToObject(root, "acc_pitch",   last_sample.acc.pitch);
    cJSON_AddNumberToObject(root, "acc_roll",    last_sample.acc.roll);
    cJSON_AddNumberToObject(root, "acc_time",    last_sample.acc.time);
    
    cJSON_AddNumberToObject(root, "gps_lat",         last_sample.gps.lat);
    cJSON_AddNumberToObject(root, "gps_lon",         last_sample.gps.lon);
    cJSON_AddNumberToObject(root, "gps_h",           last_sample.gps.h);
    cJSON_AddNumberToObject(root, "gps_have_signal", last_sample.gps.have_signal);
    cJSON_AddNumberToObject(root, "gps_spped_lat",   last_sample.gps.speed_lat);
    cJSON_AddNumberToObject(root, "gps_speed_lon",   last_sample.gps.speed_lon);
    cJSON_AddNumberToObject(root, "gps_speed_h",     last_sample.gps.speed_h);
    cJSON_AddNumberToObject(root, "gps_time",        last_sample.gps.time);
    
    /* Send response with custom headers and body set as the
     * string passed in user context*/
    char* resp_str = cJSON_Print(root);
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    
    free(resp_str);

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

static const httpd_uri_t hello = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "Hello World!"
};

/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /hello and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /hello or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /hello). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/hello", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    } else if (strcmp("/echo", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        /* Return ESP_FAIL to close underlying socket */
        return ESP_FAIL;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

void http_server_start(void) {
    static httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));   
}
