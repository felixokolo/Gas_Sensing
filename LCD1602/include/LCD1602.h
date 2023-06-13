#ifndef CHECKLCD
#define CHECKLCD

#include "PCF8574.h"
#include <stdio.h>
#include "driver/i2c.h"

esp_err_t lcd_init();
esp_err_t lcd_write_bytes(char *b, int len, int line, int start);
esp_err_t lcd_set_ddram(char addr);
esp_err_t lcd_clear_display();

#endif