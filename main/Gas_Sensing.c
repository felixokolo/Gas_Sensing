#include "includes.h"

static QueueHandle_t gpio_evt_queue = NULL;
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
const adc_channel_t MQ6_ADC = ADC_INPUT_MQ6;
const adc_channel_t MQ135_ADC = ADC_INPUT_MQ135;
adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t adc_cali_handle = NULL;

float humidity = 0, temp = 0;
float methane_ppm = 0, benzene_ppm = 0;
int current_display = 0, methane_alarm = 0, benzene_alarm = 0;
char me_message[20] = "";
char ben_message[20] = "";


const float MQ6_SLOPE = -0.57460993930210461379181967463889;
const float MQ6_CONST = 0.90049974853841845516727869855557;

const float MQ135_SLOPE = -0.47712125472;
const float MQ135_CONST = 0.43136376416;

const static char *TAG = "Gas sensing";


static esp_err_t i2c_master_init()
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
	conf.clk_flags = 0;
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode,
                              0,
                              0, 0);
}

static void adc_config()
{
	//-------------ADC1 Init---------------//
    
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = atten,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, MQ6_ADC, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, MQ135_ADC, &config));

	//-------------ADC1 Calibration Init---------------//
    
    adc_cali_line_fitting_config_t cali_config = {
            .unit_id = ADC_UNIT_1,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
    adc_cali_create_scheme_line_fitting(&cali_config, &adc_cali_handle);

}

int get_adc_reading(adc_channel_t channel)
{
	uint32_t readings = 0;
	int adc_voltage = 0;
	//readings += adc1_get_raw((adc1_channel_t)channel);
	ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, channel, &readings));
	//ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, MQ6_ADC, &readings);

	//readings /= NO_OF_ADC_SAMPLES;
	//adc_voltage = esp_adc_cal_raw_to_voltage(readings, adc_chars);
	ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, readings, &adc_voltage));
	printf("ADC voltage reading %d\n", adc_voltage);
	return adc_voltage * 2;
}

float get_MQ6_level()
{
	int reading =  get_adc_reading(MQ6_ADC);
	float resistance = (float) (5000/ (float) reading) * MQ6_LOAD_RESISTANCE;
	float y = log10(resistance/MQ6_AIR_RESISTANCE);
	float x = (y - MQ6_CONST) / MQ6_SLOPE;
	return pow(10, x);

}

float get_MQ135_level()
{
	int reading =  get_adc_reading(MQ135_ADC);
	float resistance = (float) (5000/ (float) reading) * MQ135_LOAD_RESISTANCE;
	float y = log10(resistance/MQ135_AIR_RESISTANCE);
	float x = (y - MQ135_CONST) / MQ135_SLOPE;
	return pow(10, x);
}



void gcvt(float num, int decimal_points, char *buff)
{
	int tmp = num, len = 0;
	itoa(tmp, buff, 10);
	len = strlen(buff);
	if (decimal_points > 0)
	{
		*(buff + len) = '.';
		tmp = (num - tmp) * 10;
		itoa(tmp, (buff + len + 1), 10);
	}
}


void gpio_init()
{
	gpio_config_t io_conf = {};

	// Output gpio pins configuration
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
	io_conf.pin_bit_mask = OUTPUT_PIN_SEL;
	gpio_config(&io_conf);
}


void display_current(enum display_t current)
{
    switch (current)
    {
        case TEMP_HUM:
			char humidity_string[10], temp_string[10];
			gcvt(temp, 1, temp_string);
			gcvt(humidity, 1, humidity_string);
            show_temp_hum(temp_string, humidity_string);
        break;

		case GAS_PPM:
			char methane_ppm_str[10], benzene_ppm_str[10];
			gcvt(methane_ppm, 0, methane_ppm_str);
			gcvt(benzene_ppm, 0, benzene_ppm_str);
			show_gas_ppm(methane_ppm_str, benzene_ppm_str);
		break;

		case ALARMS:
			show_alarm(me_message, ben_message);
		break;
    }
}

void app_main(void)
{
	esp_err_t ret;
	uint8_t status;
	int count = 0;
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	ESP_ERROR_CHECK(i2c_master_init());
	ret = lcd_init();
	gpio_init();
	AHT10_init();
	adc_config();
	get_adc_reading(MQ6_ADC);

	while (1)
	{
		AHT10_read(&status, &humidity, &temp);
		printf("Status - %X  Humidity - %f  Temperature - %f\n", status, humidity, temp);
		methane_ppm = get_MQ6_level();
		benzene_ppm = get_MQ135_level();
		if (methane_ppm > METHANE_PPM_THRESHOLD)
		{
			strcpy(me_message, "High gas level");
			gpio_set_level(GPIO_OUTPUT_BUZZER, 1);
			methane_alarm = 1;
			current_display = 2;
		}
		else if (methane_alarm == 1)
		{
			strcpy(me_message, "");
			gpio_set_level(GPIO_OUTPUT_BUZZER, 0);
			methane_alarm = 0;
			current_display = 1;
		}

		if (benzene_ppm > BENZENE_PPM_THRESHOLD)
		{
			strcpy(ben_message, "Poor air quality");
			gpio_set_level(GPIO_OUTPUT_BUZZER, 1);
			benzene_alarm = 1;
			current_display = 2;
		}
		else if (benzene_alarm == 1)
		{
			strcpy(ben_message, "");
			gpio_set_level(GPIO_OUTPUT_BUZZER, 0);
			benzene_alarm = 0;
			current_display = 1;
		}
		display_current(current_display);
		if (benzene_alarm == 1 || methane_alarm == 1)
			continue;
		//gcvt(humidity, 1, humidity_string);
		//gcvt(temp, 1, temp_string);
		//lcd_write_bytes(humidity_string, strlen(humidity_string), TEMP_HUMIDITY_LINE, HUMIDITY_LCD_START);
		//lcd_write_bytes(temp_string, strlen(temp_string), TEMP_HUMIDITY_LINE, TEMP_LCD_START);
		printf("Done returned %s\n", esp_err_to_name(ret));
		count ++;
		if (count == 10)
		{
			count = 0;
			current_display++;
			if (current_display >= 2)
				current_display = 0;
		}
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}

}
