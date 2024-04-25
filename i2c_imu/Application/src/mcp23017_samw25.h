#ifndef MCP23017_H_
#define MCP23017_H_

#include "I2cDriver.h"
#include "SerialConsole.h"

// MCP23017 registers
#define MCP23017_IODIRA 0x00
#define MCP23017_GPPUA 0x0C
#define MCP23017_GPIOA 0x12
#define MCP23017_ADDRESS 0x20 // Address with A2, A1, A0 to GND

// Button defines
#define SPI_BUTTON 5
#define UART_BUTTON 6
#define I2C_BUTTON 7
#define GPIO_BUTTON 4

void mcp23017_init(void);
uint8_t mcp23017_read_gpio(void);
void mcp23017_print_button(uint8_t gpio_state);

#endif /* MCP23017_H_ */
