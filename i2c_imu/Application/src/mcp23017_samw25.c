#include "mcp23017_samw25.h"

void mcp23017_init(void) {
	uint8_t buffer[2];
	// Set IODIRA to input (0xFF)
	buffer[0] = MCP23017_IODIRA;
	buffer[1] = 0xFF;
	I2C_Data data = {
		.address = MCP23017_ADDRESS,
		.msgOut = buffer,
		.lenOut = 2,
		.msgIn = NULL,
		.lenIn = 0
	};
	I2cWriteDataWait(&data, pdMS_TO_TICKS(10));

	// Enable GPPUA pull-up resistors (0xFF)
	buffer[0] = MCP23017_GPPUA;
	buffer[1] = 0xFF;
	I2cWriteDataWait(&data, pdMS_TO_TICKS(10));
}

uint8_t mcp23017_read_gpio(void) {
	uint8_t buffer[1] = {MCP23017_GPIOA};
	uint8_t receivedData;
	I2C_Data data = {
		.address = MCP23017_ADDRESS,
		.msgOut = buffer,
		.lenOut = 1,
		.msgIn = &receivedData,
		.lenIn = 1
	};
	I2cReadDataWait(&data, pdMS_TO_TICKS(10), pdMS_TO_TICKS(100));
	return receivedData;
}

void mcp23017_print_button(uint8_t gpio_state) {
	char message[50];

	if (!(gpio_state & (1 << SPI_BUTTON))) {
		snprintf(message, sizeof(message), "SPI Button pressed\r\n");
		SerialConsoleWriteString(message);
	}
	if (!(gpio_state & (1 << UART_BUTTON))) {
		snprintf(message, sizeof(message), "UART Button pressed\r\n");
		SerialConsoleWriteString(message);
	}
	if (!(gpio_state & (1 << I2C_BUTTON))) {
		snprintf(message, sizeof(message), "I2C Button pressed\r\n");
		SerialConsoleWriteString(message);
	}
	if (!(gpio_state & (1 << GPIO_BUTTON))) {
		snprintf(message, sizeof(message), "GPIO Button pressed\r\n");
		SerialConsoleWriteString(message);
	}
}
