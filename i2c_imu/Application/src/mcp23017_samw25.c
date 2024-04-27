#include "mcp23017_samw25.h"

// Initialize MCP23017 general purpose IO expander
void mcp23017_init(void) {
    uint8_t buffer[2];

    // Configure all pins on GPIOA (IODIRA register) as inputs
    buffer[0] = MCP23017_IODIRA;  // Register address for IODIRA
    buffer[1] = 0xFF;             // Set all bits to 1 for input mode
    I2C_Data data = {
        .address = MCP23017_ADDRESS,  // I2C address of MCP23017
        .msgOut = buffer,
        .lenOut = 2,
        .msgIn = NULL,
        .lenIn = 0
    };
    I2cWriteDataWait(&data, pdMS_TO_TICKS(10));  // Write data with a timeout

    // Enable internal pull-up resistors for the buttons on GPIOA (GPPUA register)
    buffer[0] = MCP23017_GPPUA;   // Register address for GPPUA
    buffer[1] = 0xFF;             // Set all bits to 1 to enable pull-ups
    I2cWriteDataWait(&data, pdMS_TO_TICKS(10));  // Write data with a timeout
}

// Read the current state of GPIOA pins
uint8_t mcp23017_read_gpio(void) {
    uint8_t buffer[1] = {MCP23017_GPIOA};  // Register address to read from
    uint8_t receivedData;
    I2C_Data data = {
        .address = MCP23017_ADDRESS,
        .msgOut = buffer,
        .lenOut = 1,
        .msgIn = &receivedData,
        .lenIn = 1
    };
    I2cReadDataWait(&data, pdMS_TO_TICKS(10), pdMS_TO_TICKS(100));  // Read data with a timeout
    return receivedData;  // Return the state of GPIOA pins
}

// Print messages for button presses
void mcp23017_print_button(uint8_t gpio_state) {
    char message[50];

    // Check and print for each button; assume active low configuration
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
