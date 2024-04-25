#include <errno.h>
#include "CliThread/CliThread.h"
#include "FreeRTOS.h"
#include "I2cDriver/I2cDriver.h"
#include "SerialConsole.h"
#include "WifiHandlerThread/WifiHandler.h"
#include "asf.h"
#include "driver/include/m2m_wifi.h"
#include "main.h"
#include "stdio_serial.h"
#include "rtc.h"  
#include "SSD1306.h"  // Include the SSD1306 OLED driver

/****
 * Defines and Types
 ****/
#define APP_TASK_ID 0
#define CLI_TASK_ID 1

/****
 * Local Function Declaration
 ****/
void vApplicationIdleHook(void);
static void StartTasks(void);
void vApplicationDaemonTaskStartupHook(void);
void vApplicationMallocFailedHook(void);
void vApplicationStackOverflowHook(void);
void vApplicationTickHook(void);

void PrintTime(const TIME t);
static void DisplayTask(void *pvParameters);
static void RtcTask(void *pvParameters);  // Task for RTC operations

/****
 * Variables
 ****/
static TaskHandle_t cliTaskHandle = NULL;
static TaskHandle_t daemonTaskHandle = NULL;
static TaskHandle_t wifiTaskHandle = NULL;
static TaskHandle_t uiTaskHandle = NULL;
static TaskHandle_t controlTaskHandle = NULL;
static TaskHandle_t displayTaskHandle = NULL;
static TaskHandle_t rtcTaskHandle = NULL; // Handle for RTC task

char bufferPrint[64];

void PrintTime(const TIME t) {
    // Assuming 24-hour format is used and conversion to 12-hour format is needed for display
    char am_pm = 'A';
    int hour12 = t.hour;
    if (hour12 >= 12) {
        am_pm = 'P';
        hour12 -= 12;
    }
    if (hour12 == 0) hour12 = 12; // Convert '00' hour to '12'

    // Map day of week from number to string
    char* daysOfWeek[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    char* monthNames[] = {"January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"};

    // Format and print the time
    sprintf(bufferPrint, "Time: %02d:%02d:%02d %cM, %s, %s %02d, 20%02d\r\n",
    hour12, t.minutes, t.seconds, am_pm,
    daysOfWeek[t.dayofweek - 1], monthNames[t.month - 1],
    t.dayofmonth, t.year);

    SerialConsoleWriteString(bufferPrint);
}

int main(void) {
    system_init();
    InitializeSerialConsole();
    vTraceEnable(TRC_START);
    vTaskStartScheduler();
    return 0; // Will not get here
}

void vApplicationDaemonTaskStartupHook(void) {
    SerialConsoleWriteString("\r\n\r\n-----ESE516 Main Program-----\r\n");
    SerialConsoleWriteString("Initialize HW...\r\n");
    if (I2cInitializeDriver() != STATUS_OK) {
        SerialConsoleWriteString("Error initializing I2C Driver!\r\n");
    } else {
        SerialConsoleWriteString("I2C Driver Initialized!\r\n");
        if (SSD1306_Init()) {
            SSD1306_Fill(SSD1306_COLOR_BLACK);  // Clear the display
            SSD1306_GotoXY(10, 25);  // Position cursor
            SSD1306_Puts("Hi", &Font_11x18, SSD1306_COLOR_WHITE);  // Print "Hi"
            SSD1306_UpdateScreen();  // Refresh the display
            SerialConsoleWriteString("SSD1306 Successfully Initialized!\r\n");
        } else {
            SerialConsoleWriteString("SSD1306 Initialization failed!\r\n");
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Delay after initialization
        StartTasks();
    }
    vTaskSuspend(daemonTaskHandle);
}

static void StartTasks(void) {
    snprintf(bufferPrint, 64, "Heap before starting tasks: %d\r\n", xPortGetFreeHeapSize());
    SerialConsoleWriteString(bufferPrint);

    if (xTaskCreate(vCommandConsoleTask, "CLI_TASK", CLI_TASK_SIZE, NULL, CLI_PRIORITY, &cliTaskHandle) != pdPASS) {
        SerialConsoleWriteString("ERR: CLI task could not be initialized!\r\n");
    }
if (xTaskCreate(DisplayTask, "DisplayTask", 256, NULL, 5, &displayTaskHandle) != pdPASS) {
	SerialConsoleWriteString("Failed to create display task!\r\n");
	} else {
	SerialConsoleWriteString("Display task created successfully.\r\n");
}
    if (xTaskCreate(vWifiTask, "WIFI_TASK", WIFI_TASK_SIZE, NULL, WIFI_PRIORITY, &wifiTaskHandle) != pdPASS) {
        SerialConsoleWriteString("ERR: WIFI task could not be initialized!\r\n");
    }

    if (xTaskCreate(RtcTask, "RTC_TASK", 512, NULL, 3, &rtcTaskHandle) != pdPASS) {
        SerialConsoleWriteString("ERR: RTC task could not be initialized!\r\n");
    }

    snprintf(bufferPrint, 64, "Heap after starting tasks: %d\r\n", xPortGetFreeHeapSize());
    SerialConsoleWriteString(bufferPrint);
}

void RtcTask(void *pvParameters) {
    // Initialize the I2C and RTC on first run
    static bool rtcInitialized = false;
    if (!rtcInitialized) {
        // Set initial time
        SetTime(35, 54, 22, 4, 24, 4, 24); // sec, min, hour, dayofweek, dayofmonth, month, year
        rtcInitialized = true; // Mark the RTC as initialized
        SerialConsoleWriteString("Setting RTC time to: ");
        TIME setTime = {35, 54, 22, 4, 24, 4, 24};
        PrintTime(setTime);
    }

    // Main task loop
    while (1) {
        TIME currentTime;
        GetTime(&currentTime); // Fetch current time from RTC
        SerialConsoleWriteString("Interpreted Time: ");
        PrintTime(currentTime); // Print the complete date and time
        vTaskDelay(pdMS_TO_TICKS(1000)); // Update every second
    }
}

static void DisplayTask(void *pvParameters) {
	// Initialize the display
	SSD1306_Init();
	SSD1306_Fill(SSD1306_COLOR_BLACK); // Clear display
	SSD1306_GotoXY(10, 25); // Position cursor
	SSD1306_Puts("Hello, world!", &Font_11x18, SSD1306_COLOR_WHITE); // Display text
	SSD1306_UpdateScreen(); // Refresh the display

	for (;;) {
		// This could be replaced by actual dynamic updates
		SSD1306_Fill(SSD1306_COLOR_BLACK);
		SSD1306_GotoXY(10, 25);
		SSD1306_Puts("Updating...", &Font_11x18, SSD1306_COLOR_WHITE);
		SSD1306_UpdateScreen();
		vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
	}
}

void vApplicationMallocFailedHook(void) {
    SerialConsoleWriteString("Error on memory allocation on FREERTOS!\r\n");
    while (1);
}

void vApplicationStackOverflowHook(void) {
    SerialConsoleWriteString("Error on stack overflow on FREERTOS!\r\n");
    while (1);
}

void vApplicationTickHook(void) { SysTick_Handler_MQTT(); }
