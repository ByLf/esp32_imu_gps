/* i2c - Example

   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples

   See README.md file to get detailed usage of this example.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "i2c_implement.h"
#include "mpu9250.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"

mpu9250_t mpu9250;
imu_raw_to_real_t real_data;
imu_sensor_data_t sensor_data;
static SemaphoreHandle_t print_mux;

void read_imu_data(void *arg)
{
    bool ret;
    uint32_t task_idx = (uint32_t)arg;
    int cnt = 0;
    while (1) {
        ret = mpu9250_read_all(&mpu9250, &sensor_data);
        xSemaphoreTake(print_mux, portMAX_DELAY);
        if (ret) {
            printf("TASK[%d] test cnt: %d, \n", task_idx, cnt++);
            printf("TASK[%d]  MASTER READ SENSOR(MPU9250)\n", task_idx);
            printf("*******************\n");
            printf("sensor val: %d [Lux]\n", sensor_data.accel[0]);
            printf("sensor val: %d [Lux]\n", sensor_data.accel[1]);
            printf("sensor val: %d [Lux]\n", sensor_data.accel[2]);
        } else {
            printf("%s: No ack, sensor not connected...skip...", esp_err_to_name(ret));
        }
        xSemaphoreGive(print_mux);
        vTaskDelay((DELAY_TIME_BETWEEN_ITEMS_MS * (task_idx + 1)) / portTICK_RATE_MS);
        
    }
    vSemaphoreDelete(print_mux);
    vTaskDelete(NULL);
}

void app_main(void)
{
    mpu9250_init(&mpu9250, MPU9250_Accelerometer_8G, MPU9250_Gyroscope_500s, &real_data);
    xTaskCreate(read_imu_data, "read_imu_data", 1024 * 2, (void *)0, 10, NULL);
}
