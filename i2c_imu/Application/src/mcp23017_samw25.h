#ifndef MCP23017_H_
#define MCP23017_H_

#include "I2cDriver.h"
#include "SerialConsole.h"

// Register addresses for MCP23017
#define MCP23017_IODIRA 0x00  // I/O direction register for port A; 1=input, 0=output
#define MCP23017_GPPUA 0x0C   // GPIO pull-up resistor register for port A
#define MCP23017_GPIOA 0x12   // GPIO register for port A to read the pins state
#define MCP23017_ADDRESS 0x20 // Default I2C address with A2, A1, A0 set to GND

// Definitions for button pin mapping on GPIOA
#define SPI_BUTTON 1   // Bit position for SPI button (GPA1)
#define I2C_BUTTON 3   // Bit position for I2C button (GPA3)
#define UART_BUTTON 2  // Bit position for UART button (GPA2)
#define GPIO_BUTTON 0  // Bit position for GPIO button (GPA0)

// Function prototypes
void mcp23017_init(void);                   // Initialize MCP23017 settings
uint8_t mcp23017_read_gpio(void);           // Read current state of GPIOA pins
void mcp23017_print_button(uint8_t gpio_state); // Print which button is pressed

#endif /* MCP23017_H_ */
