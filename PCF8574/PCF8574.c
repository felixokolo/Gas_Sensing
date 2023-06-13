#include <stdio.h>
#include "PCF8574.h"

esp_err_t pcf8574_write_bytes(char *byts, int len)
{
    i2c_cmd_handle_t cmd;
    int cnt;
    esp_err_t ret;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ESP_SLAVE_ADDR_PCF, ACK_CHECK_EN); //Enter slave address
    for (cnt = 0; cnt != len; cnt++)
    {
        i2c_master_write_byte(cmd, *(byts + cnt), ACK_CHECK_EN);
    }
    i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    printf("Done sending bytes\n");
    return ret;
    

}
