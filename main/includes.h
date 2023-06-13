#ifndef CHECKER
#define CHECKER

#include "driver/i2c.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "esp_event.h"
#include <string.h>


#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_adc_cal.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include <stdlib.h>
#include "math.h"

#include "LCD1602.h"
#include "AHT10.h" 

#define ESP_INTR_FLAG_DEFAULT 0
#define ADC_INPUT_MQ6 ADC_CHANNEL_6
#define ADC_INPUT_MQ135 ADC_CHANNEL_7
#define GPIO_OUTPUT_BUZZER GPIO_NUM_23
#define INPUT_PIN_SEL ((1ULL << GPIO_INPUT_MQ6) | (1ULL << GPIO_INPUT_MQ135))
#define OUTPUT_PIN_SEL (1ULL << GPIO_OUTPUT_BUZZER)

#define DEFAULT_VREF    1100
#define NO_OF_ADC_SAMPLES 64

#define TEMP_LCD_START 6
#define HUMIDITY_LCD_START 10
#define METHANE_LEVEL_LCD_START 8
#define BENZENE_LEVEL_LCD_START 9
#define GAS_AIR_LINE 0
#define TEMP_HUMIDITY_LINE 1
#define MQ135_LOAD_RESISTANCE 10000
#define MQ6_LOAD_RESISTANCE 20000
#define MQ135_AIR_RESISTANCE 180000
#define MQ6_AIR_RESISTANCE 560000

#define METHANE_PPM_THRESHOLD 2000
#define BENZENE_PPM_THRESHOLD 1000

enum display_t {
	TEMP_HUM,
	GAS_PPM,
	ALARMS
};

static esp_err_t i2c_master_init();
void gpio_init();
static void gpio_task_example(void* arg);
void gcvt(float num, int decimal_points, char *buff);
static void adc_config();
int get_adc_reading(adc_channel_t channel);
void show_temp_hum();
void show_gas_ppm();
void show_current(enum display_t current);
float get_MQ6_level();
float get_MQ135_level();
void show_alarm(char *me_message, char *ben_message);
#endif