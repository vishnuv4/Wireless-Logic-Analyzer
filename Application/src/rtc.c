#include <asf.h>
#include <stdio.h>
#include <string.h>
#include <rtc.h>
#include "SerialConsole.h"
#include "I2cDriver.h"

#define I2C_SLAVE_ADDR 0x68 // Define RTC slave address

// Declare the i2c_master_module instance
struct i2c_master_module i2cMasterModule;

char buff[20]; // Buffer for serial output
TIME time;     // Instance for storing time

uint8_t msg_in[16], msg_out[16];

// Convert decimal to BCD representation
uint8_t decToBcd(int val) {
	return (uint8_t)((val / 10 * 16) + (val % 10));
}

// Convert BCD to decimal representation
uint8_t bcdToDec(uint8_t val) {
	return (uint8_t)((val / 16 * 10) + (val % 16));
}

// Function to set time on the RTC
void SetTime(uint8_t sec, uint8_t min, uint8_t hour, uint8_t dow, uint8_t dom, uint8_t month, uint8_t year) {
	uint8_t data_to_write[] = {
		decToBcd(sec),
		decToBcd(min),
		decToBcd(hour),
		decToBcd(dow),
		decToBcd(dom),
		decToBcd(month),
		decToBcd(year)
	};
	
	struct I2C_Data data = {
		.address = I2C_SLAVE_ADDR,
		.msgOut = data_to_write,
		.msgIn = msg_in,
		.lenIn = 0,
		.lenOut = 7,
	};

	// Write time data to RTC
	enum status_code status = I2cWriteData(&data);
	if (status == STATUS_OK) {
		SerialConsoleWriteString("Time successfully written.\r\n");
		} else {
		SerialConsoleWriteString("Error writing time.\r\n");
	}
}

// Function to get time from the RTC
void GetTime(TIME *current_time) {
	uint8_t received_time[7];
	uint8_t reg_address = 0x00;
	
	struct I2C_Data data1 = {
		.address = I2C_SLAVE_ADDR,
		.msgOut = &reg_address,
		.msgIn = msg_in,
		.lenIn = 0,
		.lenOut = 1,
		};
	
	enum status_code writeStatus = I2cWriteData(&data1);
	if (writeStatus != STATUS_OK) {
		sprintf(buff, "GetTime write data: %x\r\n", writeStatus);
		SerialConsoleWriteString(buff);
	}

	// Write the register address to read from (DS3231 seconds register address)
		struct I2C_Data data2 = {
			.address = I2C_SLAVE_ADDR,
			.msgOut = &reg_address,
			.msgIn = received_time,
			.lenIn = 7,
			.lenOut = 0,
		};
	enum status_code status = I2cReadData(&data2);
	if (status != STATUS_OK) {
		SerialConsoleWriteString("Error sending register address.\r\n");
		return;
	}

	// Read the current time from RTC
	
	
	if (status == STATUS_OK) {
		// Convert BCD to decimal and store in current_time struct
		current_time->seconds = bcdToDec(received_time[0]);
		current_time->minutes = bcdToDec(received_time[1]);
		current_time->hour = bcdToDec(received_time[2]);
		current_time->dayofweek = bcdToDec(received_time[3]);
		current_time->dayofmonth = bcdToDec(received_time[4]);
		current_time->month = bcdToDec(received_time[5]);
		current_time->year = bcdToDec(received_time[6]);
		SerialConsoleWriteString("Time read successfully.\r\n");
		} else {
		SerialConsoleWriteString("Error reading time.\r\n");
	}
}

// Configure I2C master settings
void setupI2CMaster(void) {
	// Initialize I2C master module with default configuration
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);

	// Customize I2C master settings
	config_i2c_master.buffer_timeout = 20000;
	config_i2c_master.pinmux_pad0 = PINMUX_PA08C_SERCOM0_PAD0;
	config_i2c_master.pinmux_pad1 = PINMUX_PA09C_SERCOM0_PAD1;

	// Initialize and enable I2C master module
	i2c_master_init(&i2cMasterModule, SERCOM0, &config_i2c_master);
	i2c_master_enable(&i2cMasterModule);
}

void vRtcTask(void *pvParameters)
{
	I2cInitializeDriver(); // Setup I2C master module
	SetTime(0, 37, 11, 1, 7, 4, 24); // Set initial time
	while (1) {
		GetTime(&time); // Get current time from RTC
		sprintf(buff, "%02d:%02d:%02d", time.hour, time.minutes, time.seconds); // Format time into buffer
		SerialConsoleWriteString(buff); // Print formatted time
		vTaskDelay(1000); // Delay to avoid spamming
	}

}
