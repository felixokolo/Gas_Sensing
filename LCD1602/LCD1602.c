#include "LCD1602.h"

char init_bytes[] = {
							0X3C, 0X3A, 0X3C, 0X3A, 0X3C, 0X3A, //Function set to 8 bits
							0X2C, 0X2A, 0X2C, 0X2A, 0X8C, 0X8A, //Function set to 4 bits
							0X0C, 0X0A, 0X9C, 0X9A, // Display OFF
							0X0C, 0X0A, 0X1C, 0X1A, // Clear display
							//Delay required after sending clear display bytes
							0X0C, 0X0A, 0X6C, 0X6A, // Set entry mode
							0X0C, 0X0A, 0XCC, 0XCA, //Display ON wit cursor OFF
							0X8C, 0X8A, 0XEC, 0XEA, //Set DDRAM address
							0X4D, 0X4B, 0X1D, 0X1B, //Write data to screen
                            0X4D, 0X4B, 0X2D, 0X2B //Write data to screen
						} ;

	char clear_display[] = {
								0X0C, 0X0A, 0X1C, 0X1A // Clear display
	};

    char set_ddram[] = {
								0X8C, 0X8A, 0XFC, 0XFA // Clear display
	};



esp_err_t lcd_init()
{
    esp_err_t ret;
	ret = pcf8574_write_bytes(init_bytes, 20);
	vTaskDelay(500 / portTICK_PERIOD_MS);
	ret = pcf8574_write_bytes(init_bytes + 20, 8);
    return ret;
}


esp_err_t lcd_set_ddram(char addr)
{
    esp_err_t ret;
    char upper = 0X8C + (addr & 0X70);
    char lower = 0X0C + ((addr & 0X0F) << 4);
    char byts[] = {upper, (upper | 0X02) & 0XFB, lower, (lower | 0X02) & 0XFB};
    ret = pcf8574_write_bytes(byts, 4);
    return ret;

}

esp_err_t lcd_clear_display()
{
    esp_err_t ret;
	ret = pcf8574_write_bytes(clear_display, 4);
    return ret;
}


esp_err_t lcd_write_bytes(char *b, int len, int line, int start)
{
    esp_err_t ret;
    int cnt;
    char *byts = (char*) malloc(sizeof(char) * len * 4);
    lcd_set_ddram(start + (line * 0X40));
    for (cnt = 0; cnt < len; cnt++)
    {
        *(byts + (cnt * 4)) = 0X0D + (*(b + cnt) & 0XF0);
        *(byts + (cnt * 4) + 1) = (*(byts + (cnt * 4)) | 0X02) & 0XFB;
        *(byts + (cnt * 4) + 2) = 0X0D + ((*(b + cnt) & 0X0F) << 4);
        *(byts + (cnt * 4) + 3) = (*(byts + (cnt * 4) + 2) | 0X02) & 0XFB;
    }
    ret = pcf8574_write_bytes(byts, len * 4);
    free(byts);
    return ret;
    

}
