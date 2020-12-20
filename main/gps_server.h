#ifndef GPS_SERVER_H
#define GPS_SERVER_H

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

#include <driver/uart.h>
#include <string>

void gps_server_init(void);

extern std::string last_msg;

#endif
