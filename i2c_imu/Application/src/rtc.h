#ifndef RTC_H
#define RTC_H

#include <asf.h>
#include <stdio.h>
#include <string.h>
#include "I2cDriver.h"

// Time structure for RTC data
typedef struct {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hour;
	uint8_t dayofweek;
	uint8_t dayofmonth;
	uint8_t month;
	uint8_t year;
} TIME;

void setupI2CMaster(void);
void GetTime(TIME *time);
void SetTime(uint8_t sec, uint8_t min, uint8_t hour, uint8_t dow, uint8_t dom, uint8_t month, uint8_t year);
uint8_t decToBcd(int val);
int bcdToDec(uint8_t val);
void vRtcTask(void *pvParameters);

#endif /* RTC_H */
