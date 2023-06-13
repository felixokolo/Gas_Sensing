#include "AHT10.h"

	esp_err_t AHT10_init(){
		int ret;
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR_ATH | WRITE_BIT), ACK_CHECK_EN);
		i2c_master_write_byte(cmd, 0XE1, ACK_CHECK_EN);
		i2c_master_write_byte(cmd, 0X08, ACK_CHECK_EN);
		i2c_master_write_byte(cmd, 0X00, ACK_CHECK_EN);
		i2c_master_stop(cmd);
		ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
		return ret;
 }
	esp_err_t AHT10_trigger(){
		int ret;
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR_ATH | WRITE_BIT), ACK_CHECK_EN);
		i2c_master_write_byte(cmd, 0XAC, ACK_CHECK_EN);
		i2c_master_write_byte(cmd, 0X33, ACK_CHECK_EN);
		i2c_master_write_byte(cmd, 0X00, ACK_CHECK_EN);
		
		i2c_master_stop(cmd);
		ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
		return ret;
 }
 
	esp_err_t AHT10_read(uint8_t *status, float *humidity, float *temp){
		int ret;
		uint8_t dat[6];
		uint32_t humidity_data = 0, temp_data = 0;
		AHT10_trigger();
		vTaskDelay(100 / portTICK_PERIOD_MS);
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR_ATH | READ_BIT), ACK_CHECK_EN);
		i2c_master_read(cmd, dat, 5, ACK_VAL);
		i2c_master_read(cmd, dat+5, 1, NACK_VAL);
		i2c_master_stop(cmd);
		ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
		if (ret != ESP_OK)
		{
			printf("Error reading AHT10 sensor\n");
			return ret;
		}
		*status = dat[0];
		humidity_data = dat[1];
		humidity_data = humidity_data<<8;
		humidity_data += dat[2];
		humidity_data = humidity_data<<4;
		humidity_data += (dat[3]>>4);
		temp_data = (dat[3]&0X0F);
		temp_data = temp_data<<8;
		temp_data += dat[4];
		temp_data = temp_data<<8;
		temp_data += dat[5];
		*humidity = ((float)humidity_data/1048576)*100;
		*temp = ((float)temp_data/1048576)*200-50;
		return ret;
 }