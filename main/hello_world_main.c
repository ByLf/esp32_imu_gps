/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "i2c_com.h"

#ifdef CONFIG_IDF_TARGET_ESP32
#define CHIP_NAME "ESP32"
#endif

#ifdef CONFIG_IDF_TARGET_ESP32S2BETA
#define CHIP_NAME "ESP32-S2 Beta"
#endif

void app_main(void)
{
    int16_t Ax,Ay,Az,Gx,Gy,Gz,temp;
    uint8_t dev_addr=0x68;
    uint8_t *data = (uint8_t *)malloc(14);
    data[0]=0x3B;
    printf("Hello world!\n");
    printf("%d \n",i2c_master_init());
    //printf("%d \n",i2c_master_wr(0,data,1));
    i2c_get(0,dev_addr,0x3B,data,14);
    Ax = (int16_t)(data[0] << 8 | data[1]);
    Ay = (int16_t)(data[2] << 8 | data[3]);
    Az = (int16_t)(data[4] << 8 | data[5]);
    temp = (int16_t)(data[6] << 8 | data[7]);
    Gx = (int16_t)(data[8] << 8 | data[9]);
    Gy = (int16_t)(data[10] << 8 | data[11]);
    Gz = (int16_t)(data[12] << 8 | data[13]);
//    for(uint8_t i=0;i<4;++i){
    printf("Ax = %i\n Ay = %i\n Az = %i\n Gx = %i\n Gy = %i\n Gz = %i\nt=%i",Ax,Ay,Az,Gx,Gy,Gz,temp);
//    }
    printf("\n");
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
            CHIP_NAME,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
