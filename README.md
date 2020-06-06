# ESP32 - SIM808 DevBoard
This is project using the ESP32 uC and the SIM808 GSM/GPRS/Bluetooth/GPS board.

## Hardware Wiring

| ESP32 Interface | #define | ESP32 Pin | Wired to |
| :---: | :---: | :---: | :---: |
| UART0 (TxD) | UART0_TX | GPIO1 | DEBUG |
| UART0 (RxD) | UART0_RX | GPIO3 | DEBUG |
| UART2 (TxD) | UART2_TX | GPIO17 | SIM808 RX |
| UART2 (RxD) | UART2_RX | GPIO16 | SIM808 TX |
| I2C (SCL) | SCL_PIN | GPIO2 | OLED 1306 SCL |
| I2C (SDA) | SDA_PIN | GPIO15 | OLED 1306 SDA |

## Changes:

- 20202004 - Project started
- 20200423 - Add OLED display

## Output

It outputs the complete GPS package at UART0. The package is:

+GPSINF: mode,longitude,latitude,altitude,UTC time,TTFF,num,speed,course

On the OLED, it displays only the speed.
