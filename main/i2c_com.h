#include "esp_system.h"
#include "driver/i2c.h"
#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */
esp_err_t i2c_master_init(void);
esp_err_t i2c_master_wr(i2c_port_t i2c_num, uint8_t *data_wr, size_t size);
esp_err_t i2c_master_rd(i2c_port_t i2c_num, uint8_t *data_rd, size_t size);
