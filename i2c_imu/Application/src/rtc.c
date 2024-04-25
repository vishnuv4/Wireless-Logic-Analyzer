#include "RTC.h"
#include "SerialConsole.h"
#include "I2cDriver.h"
#include "asf.h"

#define I2C_SLAVE_ADDR 0x68 // DS3231 I2C address
uint8_t msg_in[16];
// Global variables
struct i2c_master_module i2cMasterModule;
char buff[20]; // Buffer for serial output
TIME time;     // Instance for storing time
I2C_Data i2cData;

// Decimal to BCD conversion
uint8_t decToBcd(int val) {
	return (uint8_t)((val / 10 * 16) + (val % 10));
}

// BCD to Decimal conversion
int bcdToDec(uint8_t val) {
	return ((val >> 4) * 10) + (val & 0x0F);
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
void GetTime(TIME *current_time) {
	uint8_t reg_address = 0x00;
	uint8_t received_time[7];

	i2cData.address = I2C_SLAVE_ADDR;
	i2cData.msgOut = &reg_address;
	i2cData.lenOut = 1;
	i2cData.msgIn = received_time;
	i2cData.lenIn = 7;

	int32_t status = I2cReadDataWait(&i2cData, 500, 1000);
	if (status == ERROR_NONE) {
		current_time->seconds = bcdToDec(received_time[0]);
		current_time->minutes = bcdToDec(received_time[1]);
		current_time->hour = bcdToDec(received_time[2]);
		current_time->dayofweek = bcdToDec(received_time[3]);
		current_time->dayofmonth = bcdToDec(received_time[4]);
		current_time->month = bcdToDec(received_time[5]); // Directly read month, no century manipulation
		current_time->year = bcdToDec(received_time[6]) + 2000; // Directly adding 2000 for simplicity
		SerialConsoleWriteString("Time read successfully.\r\n");
		} else {
		SerialConsoleWriteString("Error reading time.\r\n");
	}
}


void setupI2CMaster(void) {
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);
	config_i2c_master.buffer_timeout = 20000;
	config_i2c_master.pinmux_pad0 = PINMUX_PA08C_SERCOM0_PAD0;
	config_i2c_master.pinmux_pad1 = PINMUX_PA09C_SERCOM0_PAD1;
	i2c_master_init(&i2cMasterModule, SERCOM0, &config_i2c_master);
	i2c_master_enable(&i2cMasterModule);
}
