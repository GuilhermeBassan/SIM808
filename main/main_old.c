#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#define ECHO_TEST_TXD  (GPIO_NUM_1)
#define ECHO_TEST_RXD  (GPIO_NUM_3)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)

#define BUF_SIZE (1024)

#define GPS_PWR_ON "AT+CGPSPWR=1\r"	// GPS powerup
#define GPS_GET_DT "AT+CGPSINF=0\r"	// GPS request info

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
    	vTaskDelay(50/portTICK_PERIOD_MS);
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

void app_main()
{
	vTaskDelay(3000/portTICK_PERIOD_MS);

	uart_config_t uart0_config = {
			.baud_rate = 115200,
			.data_bits = UART_DATA_8_BITS,
			.parity    = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};
	uart_param_config(UART_NUM_0, &uart0_config);
	uart_set_pin(UART_NUM_0, (GPIO_NUM_1), (GPIO_NUM_3),
				 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);

	vTaskDelay(1000/portTICK_PERIOD_MS);

	uart_config_t uart2_config = {
			.baud_rate = 9600,
			.data_bits = UART_DATA_8_BITS,
			.parity    = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};
	uart_param_config(UART_NUM_2, &uart2_config);
	uart_set_pin(UART_NUM_2, (GPIO_NUM_17), (GPIO_NUM_16),
				 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	uart_driver_install(UART_NUM_2, BUF_SIZE * 2, 0, 0, NULL, 0);
	uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);

	vTaskDelay(1000/portTICK_PERIOD_MS);

    uart_write_bytes(UART_NUM_2, GPS_PWR_ON, sizeof(GPS_PWR_ON));

    vTaskDelay(1000/portTICK_PERIOD_MS);

    xTaskCreatePinnedToCore(gpsRequest, "GPS Request", 1024, NULL, 10, NULL,0);
    xTaskCreatePinnedToCore(speedDisplay, "Speed Display", 1024, NULL, 10, NULL,1);
}
