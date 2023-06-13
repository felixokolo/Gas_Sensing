#include "includes.h"


void show_temp_hum(char *temp, char *humidity)
{   lcd_clear_display();
    lcd_write_bytes("TEMP: --.- degC", 15, 0, 0);
    lcd_write_bytes("HUMIDITY: --.-%", 15, 1, 0);
    lcd_write_bytes(temp, strlen(temp), 0, TEMP_LCD_START);
    lcd_write_bytes(humidity, strlen(humidity), 1, HUMIDITY_LCD_START);
}
void show_gas_ppm(char *methane_ppm, char *benzene_ppm)
{
    lcd_clear_display();
    lcd_write_bytes("CH4 LEV:     PPM", 16, 0, 0);
    lcd_write_bytes("C6H6 LEV:    PPM", 16, 1, 0);
    lcd_write_bytes(methane_ppm, strlen(methane_ppm), 0, METHANE_LEVEL_LCD_START);
    lcd_write_bytes(benzene_ppm, strlen(benzene_ppm), 1, BENZENE_LEVEL_LCD_START);
}

void show_alarm(char *me_message, char *ben_message)
{
    lcd_clear_display();
    lcd_write_bytes(me_message, strlen(me_message), 0, 0);
    lcd_write_bytes(ben_message, strlen(ben_message), 1, 0);
}