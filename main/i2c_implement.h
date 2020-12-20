
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include <stdio.h>

#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)

#define DATA_LENGTH 512                  /*!< Data buffer length of test buffer */
#define RW_TEST_LENGTH 128               /*!< Data length for r/w test, [0,DATA_LENGTH] */
#define DELAY_TIME_BETWEEN_ITEMS_MS 1000 /*!< delay time between different test items */

#define I2C_SLAVE_SCL_IO CONFIG_I2C_SLAVE_SCL               /*!< gpio number for i2c slave clock */
#define I2C_SLAVE_SDA_IO CONFIG_I2C_SLAVE_SDA               /*!< gpio number for i2c slave data */
#define I2C_SLAVE_NUM I2C_NUMBER(CONFIG_I2C_SLAVE_PORT_NUM) /*!< I2C port number for slave dev */
#define I2C_SLAVE_TX_BUF_LEN (2 * DATA_LENGTH)              /*!< I2C slave tx buffer size */
#define I2C_SLAVE_RX_BUF_LEN (2 * DATA_LENGTH)              /*!< I2C slave rx buffer size */

#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUMBER(CONFIG_I2C_MASTER_PORT_NUM) /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY        /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

#define BH1750_SENSOR_ADDR CONFIG_BH1750_ADDR   /*!< slave address for BH1750 sensor */
#define BH1750_CMD_START CONFIG_BH1750_OPMODE   /*!< Operation mode */
#define ESP_SLAVE_ADDR CONFIG_I2C_SLAVE_ADDRESS /*!< ESP32 slave address, you can set any 7bit value */
#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */


////////////////////////////////////////////////////////////////////////////////
//
// XXX refer to page 38 of MPU9250 Product specification
// for accel/gyro/mag orientation
//
//
//      chip orientation mark: top left
//
//      accel/gyro
//      X : left positive, counter clock
//      y : forward positive, counter clock
//      Z : up positive, counter clock
//
//      mag
//      x : forward positive
//      y : left positive
//      z : down positive
//
////////////////////////////////////////////////////////////////////////////////

#define MPU9250_I2C_ADDR        0x68      // or 110 100x. 400Khz interface

/* MPU9250 registers */
#define MPU9250_AUX_VDDIO             0x01
#define MPU9250_SMPLRT_DIV            0x19
#define MPU9250_REG_ACCEL_XOFFS_H     (0x06)
#define MPU9250_REG_ACCEL_XOFFS_L     (0x07)
#define MPU9250_REG_ACCEL_YOFFS_H     (0x08)
#define MPU9250_REG_ACCEL_YOFFS_L     (0x09)
#define MPU9250_REG_ACCEL_ZOFFS_H     (0x0A)
#define MPU9250_REG_ACCEL_ZOFFS_L     (0x0B)
#define MPU9250_REG_GYRO_XOFFS_H      (0x13)
#define MPU9250_REG_GYRO_XOFFS_L      (0x14)
#define MPU9250_REG_GYRO_YOFFS_H      (0x15)
#define MPU9250_REG_GYRO_YOFFS_L      (0x16)
#define MPU9250_REG_GYRO_ZOFFS_H      (0x17)
#define MPU9250_REG_GYRO_ZOFFS_L      (0x18)
#define MPU9250_CONFIG                0x1A
#define MPU9250_GYRO_CONFIG           0x1B
#define MPU9250_ACCEL_CONFIG          0x1C
#define MPU9250_MOTION_THRESH         0x1F
#define MPU9250_INT_PIN_CFG           0x37
#define MPU9250_INT_ENABLE            0x38
#define MPU9250_INT_STATUS            0x3A
#define MPU9250_ACCEL_XOUT_H          0x3B
#define MPU9250_ACCEL_XOUT_L          0x3C
#define MPU9250_ACCEL_YOUT_H          0x3D
#define MPU9250_ACCEL_YOUT_L          0x3E
#define MPU9250_ACCEL_ZOUT_H          0x3F
#define MPU9250_ACCEL_ZOUT_L          0x40
#define MPU9250_TEMP_OUT_H            0x41
#define MPU9250_TEMP_OUT_L            0x42
#define MPU9250_GYRO_XOUT_H           0x43
#define MPU9250_GYRO_XOUT_L           0x44
#define MPU9250_GYRO_YOUT_H           0x45
#define MPU9250_GYRO_YOUT_L           0x46
#define MPU9250_GYRO_ZOUT_H           0x47
#define MPU9250_GYRO_ZOUT_L           0x48
#define MPU9250_MOT_DETECT_STATUS     0x61
#define MPU9250_SIGNAL_PATH_RESET     0x68
#define MPU9250_MOT_DETECT_CTRL       0x69
#define MPU9250_USER_CTRL             0x6A
#define MPU9250_PWR_MGMT_1            0x6B
#define MPU9250_PWR_MGMT_2            0x6C
#define MPU9250_FIFO_COUNTH           0x72
#define MPU9250_FIFO_COUNTL           0x73
#define MPU9250_FIFO_R_W              0x74
#define MPU9250_WHO_AM_I              0x75
    
/* Gyro sensitivities in °/s */
#define MPU9250_GYRO_SENS_250       ((float) 131)
#define MPU9250_GYRO_SENS_500       ((float) 65.5)
#define MPU9250_GYRO_SENS_1000      ((float) 32.8)
#define MPU9250_GYRO_SENS_2000      ((float) 16.4)
    
/* Acce sensitivities in g */
#define MPU9250_ACCE_SENS_2         ((float) 16384)
#define MPU9250_ACCE_SENS_4         ((float) 8192)
#define MPU9250_ACCE_SENS_8         ((float) 4096)
#define MPU9250_ACCE_SENS_16        ((float) 2048)

#define AK8963_WIA                  0x00
#define AK8963_INFO                 0x01
#define AK8963_ST1                  0x02
#define AK8963_HXL                  0x03
#define AK8963_HXH                  0x04
#define AK8963_HYL                  0x05
#define AK8963_HYH                  0x06
#define AK8963_HZL                  0x07
#define AK8963_HZH                  0x08
#define AK8963_ST2                  0x09
#define AK8963_CNTL1                0x0a
#define AK8963_ASTC                 0x0c

#define AK8963_I2C_ADDR             0x0c
#define AK8963_MAG_LSB              0.15f     // in 16 bit mode, 0.15uT per LSB

extern esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t *data_rd, size_t size);
extern esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr, size_t size);
extern esp_err_t i2c_master_init(void);
extern void disp_buf(uint8_t *buf, int len);