#include "ssd1306.h"
#include "I2cDriver.h"
#include "fonts.h"
#include "asf.h"
#include "CliThread/CliThread.h"

static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

typedef struct {
	uint16_t CurrentX;
	uint16_t CurrentY;
	uint8_t Inverted;
	uint8_t Initialized;
} SSD1306_t;

static SSD1306_t SSD1306;

static void SSD1306_WRITECOMMAND(uint8_t command) {
	I2C_Data data = {
		.address = SSD1306_I2C_ADDR,
		.msgOut = &command,
		.lenOut = 1,
		.msgIn = NULL,
		.lenIn = 0
	};

	SerialConsoleWriteString("Writing command to SSD1306.\n");
	if (I2cGetMutex(portMAX_DELAY) == ERROR_NONE) {
		if (I2cWriteDataWait(&data, portMAX_DELAY) != ERROR_NONE) {
			SerialConsoleWriteString("Error writing command to SSD1306.\n");
		}
		I2cFreeMutex();
		} else {
		SerialConsoleWriteString("Failed to get I2C mutex for command write.\n");
	}
}

uint8_t SSD1306_Init(void) {
	SerialConsoleWriteString("Initializing SSD1306.\n");
	SSD1306_WRITECOMMAND(0xAE); // Display off
	// Initialization sequence
	SSD1306_WRITECOMMAND(0xAF); // Display on
	SSD1306.Initialized = 1;
	return 1;
}

void SSD1306_UpdateScreen(void) {
	SerialConsoleWriteString("Updating SSD1306 screen.\n");
	for (uint8_t m = 0; m < 8; m++) {
		SSD1306_WRITECOMMAND(0xB0 + m);
		SSD1306_WRITECOMMAND(0x00);
		SSD1306_WRITECOMMAND(0x10);

		I2C_Data data = {
			.address = SSD1306_I2C_ADDR,
			.msgOut = &SSD1306_Buffer[SSD1306_WIDTH * m],
			.lenOut = SSD1306_WIDTH,
			.msgIn = NULL,
			.lenIn = 0
		};

		if (I2cGetMutex(portMAX_DELAY) == ERROR_NONE) {
			if (I2cWriteDataWait(&data, portMAX_DELAY) != ERROR_NONE) {
				SerialConsoleWriteString("Error updating screen buffer.\n");
			}
			I2cFreeMutex();
			} else {
			SerialConsoleWriteString("Failed to get I2C mutex for screen update.\n");
		}
	}
}

void SSD1306_Fill(SSD1306_COLOR_t color) {
	memset(SSD1306_Buffer, (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF, sizeof(SSD1306_Buffer));
	SerialConsoleWriteString("Screen buffer filled.\n");
}

void SSD1306_GotoXY(uint16_t x, uint16_t y) {
	SerialConsoleWriteString("Setting cursor position.\n");
	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
		SerialConsoleWriteString("Error: Cursor position out of bounds.\n");
		return;
	}
	SSD1306.CurrentX = x;
	SSD1306.CurrentY = y;
}

char SSD1306_Putc(char ch, FontDef_t* Font, SSD1306_COLOR_t color) {
	// Implement character drawing based on current position and font
	SerialConsoleWriteString("Putting character on screen.\n");
	return ch; // Return the character written for compatibility
}

char SSD1306_Puts(char* str, FontDef_t* Font, SSD1306_COLOR_t color) {
	SerialConsoleWriteString("Writing string to screen.\n");
	while (*str) {
		if (SSD1306_Putc(*str, Font, color) != *str) {
			SerialConsoleWriteString("Failed to put character.\n");
			return *str;
		}
		str++;
	}
	return 0; // Return 0 on success
}
