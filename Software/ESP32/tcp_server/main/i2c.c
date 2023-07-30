/*************************************************************************
 * 
 * file: i2c.c
 * 
 * description:  Simple I2C driver for freeETarget
 * 
 **************************************************************************
 *
 * This file manges the I2C driver
 * 
 * See: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html
 * 
 ***************************************************************************/
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"

static const char *TAG = "i2c-simple-example";

#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

i2c_config_t i2c_configuratino = {
    .mode = I2C_MODE_MASTER,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = I2C_MASTER_FREQ_HZ,
};

/**
 * @brief Read a sequence of bytes from a MPU9250 sensor registers
 */
esp_err_t i2c_read
(
    uint8_t  reg_addr,              // I2C device address
    uint8_t* data,                  // Buffer to be sent
    size_t   length                 // Number of bytes to be sent
)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, MPU9250_SENSOR_ADDR, &reg_addr, 1, data, length, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
esp_err_t i2c_write(uint8_t reg_addr, uint8_t* data, size_t length)
{
    int i
    uint8_t write_buf[128];
    
    write_buf[0] = reg_addr;
    for ( i =0; i != length; i++)
    {
        write_buf[i+1] = data[i];
    }

    return i2c_master_write_to_device(I2C_MASTER_NUM, MPU9250_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief i2c master initialization
 */
esp_err_t i2c_master_init
(
    int    i2c_gpio_SDA       // GPIO SPI belongs to
    int    i2c_gpio_SCL       // GPIO SPI belongs to    
)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_configuration.sda_io_num = i2c_gpio_SDA,
    i2c_configuration.scl_io_num = i2c_gpio_SCL,

    i2c_param_config(i2c_master_port, &configuration);

    return i2c_driver_install(i2c_master_port, configuration.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}
