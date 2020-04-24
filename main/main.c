#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

#include "resources/SSD1306.h"

#define UART0_TX  	(GPIO_NUM_1)
#define UART0_RX  	(GPIO_NUM_3)

#define UART2_TX	(GPIO_NUM_17)
#define UART2_TX	(GPIO_NUM_16)

#define SDA_PIN GPIO_NUM_15
#define SCL_PIN GPIO_NUM_2

#define GPS_PWR_ON "AT+CGPSPWR=1\r"	// GPS powerup
#define GPS_GET_DT "AT+CGPSINF=0\r"	// GPS request info
#define MOD_RES_CK "AT\r"			// Module response check
#define MOD_RES_ER "ERROR"
#define MOD_RES_OK "OK"

#define BUF_SIZE (1024)	// Buffer size
#define DIS_INT 1000	// Display interval
#define REA_INT 500		// Read interval

void espDebug(const char *message);

void setup();

int i2cBegin();
int uart0Begin();
int uart2Begin();

int commBegin();
int gpsBegin();
static void gpsRequest();

static void speedDisplay();

void app_main()
{
	vTaskDelay(1500/portTICK_PERIOD_MS);

	setup();

    xTaskCreatePinnedToCore(gpsRequest, "GPS Request", 1024, NULL, 10, NULL, 0);
    xTaskCreatePinnedToCore(speedDisplay, "Speed Display", 1024, NULL, 10, NULL, 1);
}

void espDebug(const char *message)
{
	//printf(message);
}

void setup()
{
	espDebug("Starting I2C\r\n");
	while(i2cBegin() != 0)
	{
		espDebug("I2C not initialized...\r\n");
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
	espDebug("I2C started\r\n");

	espDebug("Starting UART0\r\n");
	while(uart0Begin() != 0)
	{
		espDebug("UART0 not initialized...\r\n");
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
	espDebug("UART0 started\r\n");

	espDebug("Starting UART2\r\n");
	while(uart2Begin() != 0)
	{
		espDebug("UART2 not initialized...\r\n");
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
	espDebug("UART2 started\r\n");

	espDebug("Starting module UART communication\r\n");
	while(commBegin() != 0)
	{
		espDebug("Module communication not initialized...\r\n");
		vTaskDelay(100/portTICK_PERIOD_MS);
	}
	espDebug("Module communication started\r\n");

	espDebug("Powering up GPS\r\n");
	while(gpsBegin() != 0)
	{
		espDebug("GPS not initialized...\r\n");
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
	espDebug("GPS initialized\r\n");
}

/* Request GPS information. The info comes in the shape:
 * +GPSINF: <mode>,<longitude>,<latitude>,<altitude>,
 * <UTC time>,<TTFF>,<num>,<speed>,<course>
 *
 * +CGPSINF: 0,2528.949000,4915.533500,850.800000,
 * 20200422074617.000,0,10,1.185280,18.020000
 *
 * <mode> 		- Package info mode				- 0
 * <longitude>	- Longitude						- 2528.949000
 * <latitude> 	- Latitude						- 4915.533500
 * <altitude>	- Altitude (m)					- 850.800000
 * <UTC time> 	- UTC time (yyyymmddHHMMSS)		- 20200422074617.000
 * <TTFF> 		- Time to first fix (seconds)	- 0
 * <num> 		- Satellites in view for fix	- 10
 * <speed> 		- Speed over ground (km/h)		- 1.185280
 * <course> 	- Course over ground			- 18.020000
 */
static void gpsRequest()
{
    while (1)
    {
    	uart_write_bytes(UART_NUM_2, GPS_GET_DT, sizeof(GPS_GET_DT));
    	vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

static void speedDisplay()
{
	uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

	while(1)
	{
    	int len = uart_read_bytes(UART_NUM_2, data, BUF_SIZE, 20 / portTICK_RATE_MS);
    	uart_write_bytes(UART_NUM_0, (const char *) data, len);
    	vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}

int i2cBegin()
{
	i2c_config_t i2c_config = {
			.mode = I2C_MODE_MASTER,
			.sda_io_num = SDA_PIN,
			.scl_io_num = SCL_PIN,
			.sda_pullup_en = GPIO_PULLUP_ENABLE,
			.scl_pullup_en = GPIO_PULLUP_ENABLE,
			.master.clk_speed = 1000000
	};

	i2c_param_config(I2C_NUM_0, &i2c_config);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

	vTaskDelay(100/portTICK_PERIOD_MS);

	return 0;
}

int uart0Begin()
{
	uart_config_t uart0_config = {
			.baud_rate = 115200,
			.data_bits = UART_DATA_8_BITS,
			.parity    = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};

	uart_param_config(UART_NUM_0, &uart0_config);
	uart_set_pin(UART_NUM_0, (UART0_TX), (UART0_RX),
			     UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);

	vTaskDelay(100/portTICK_PERIOD_MS);

	return 0;
}

int uart2Begin()
{
	uart_config_t uart2_config = {
			.baud_rate = 9600,
			.data_bits = UART_DATA_8_BITS,
			.parity    = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};

	uart_param_config(UART_NUM_2, &uart2_config);
	uart_set_pin(UART_NUM_2, (UART2_TX), (UART2_RX),
				 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	uart_driver_install(UART_NUM_2, BUF_SIZE * 2, 0, 0, NULL, 0);

	vTaskDelay(10/portTICK_PERIOD_MS);

	return 0;
}

int commBegin()
{
	int a = -1;
	uint8_t *response = (uint8_t *) malloc(BUF_SIZE);

	uart_write_bytes(UART_NUM_2, MOD_RES_CK, sizeof(MOD_RES_CK));
	vTaskDelay(10/portTICK_PERIOD_MS);

	int len = uart_read_bytes(UART_NUM_2, response, BUF_SIZE, 10/portTICK_RATE_MS);
	vTaskDelay(10/portTICK_PERIOD_MS);

	for(int i = 0; i <= len; i++)
	{
		if(response[i] == MOD_RES_OK[0])
		{
			if(response [i + 1] == MOD_RES_OK[1])
			{
				a = 0;
			}
		}
	}

	return a;
}

int gpsBegin()
{
    uart_write_bytes(UART_NUM_2, GPS_PWR_ON, sizeof(GPS_PWR_ON));
    vTaskDelay(100/portTICK_PERIOD_MS);

    return 0;
}
