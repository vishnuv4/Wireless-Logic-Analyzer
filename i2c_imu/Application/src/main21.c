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
#include "ssd1306.h" 
#include "mcp23017_samw25.h"
#define DISPLAY_TASK_STACK_SIZE (512U)
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
/****
 * Defines and Types
 ****/
#define APP_TASK_ID 0
#define CLI_TASK_ID 1

#define LED_PIN PIN_PA23
/****
 * Local Function Declaration
 ****/
void vApplicationIdleHook(void);
static void StartTasks(void);
void vApplicationDaemonTaskStartupHook(void);
void vApplicationMallocFailedHook(void);
void vApplicationStackOverflowHook(void);
void vApplicationTickHook(void);
static void ButtonReadTask(void *pvParameters);
static void DisplayTask(void *pvParameters);  // Task to update SSD1306 display
void toggleLED(bool state);
/****
 * Variables
 ****/
static TaskHandle_t cliTaskHandle = NULL;
static TaskHandle_t daemonTaskHandle = NULL;
static TaskHandle_t wifiTaskHandle = NULL;
static TaskHandle_t uiTaskHandle = NULL;
static TaskHandle_t controlTaskHandle = NULL;
static TaskHandle_t rtcTaskHandle = NULL;
static TaskHandle_t displayTaskHandle = NULL; // Handle for display task

char bufferPrint[64];

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
		vTaskDelay(pdMS_TO_TICKS(100)); // Give some time after initializing the I2C
		if (SSD1306_Init() != 1) {
			SerialConsoleWriteString("SSD1306 Initialization failed!\r\n");
			} else {
			SerialConsoleWriteString("SSD1306 Successfully Initialized!\r\n");
			StartTasks();
		}
	}
	vTaskSuspend(NULL); // Suspend the daemon task as its job is done
}
static void StartTasks(void) {
	snprintf(bufferPrint, 64, "Heap before starting tasks: %d\r\n", xPortGetFreeHeapSize());
	SerialConsoleWriteString(bufferPrint);

	if (xTaskCreate(vCommandConsoleTask, "CLI_TASK", CLI_TASK_SIZE, NULL, CLI_PRIORITY, &cliTaskHandle) != pdPASS) {
		SerialConsoleWriteString("ERR: CLI task could not be initialized!\r\n");
	}
	
	 if (xTaskCreate(ButtonReadTask, "BUTTON_READ_TASK", 400, NULL, 5, NULL) != pdPASS) {
		 SerialConsoleWriteString("ERR: Button Read task could not be initialized!\r\n");
	 }


	if (xTaskCreate(DisplayTask, "DISPLAY_TASK", DISPLAY_TASK_STACK_SIZE, NULL, 3, &displayTaskHandle) != pdPASS) {
		SerialConsoleWriteString("ERR: Display task could not be initialized!\r\n");
	}

	if (xTaskCreate(vWifiTask, "WIFI_TASK", WIFI_TASK_SIZE, NULL, WIFI_PRIORITY, &wifiTaskHandle) != pdPASS) {
		SerialConsoleWriteString("ERR: WIFI task could not be initialized!\r\n");
	}
	snprintf(bufferPrint, 64, "Heap after starting tasks: %d\r\n", xPortGetFreeHeapSize());
	SerialConsoleWriteString(bufferPrint);
}

static void DisplayTask(void *pvParameters) {
	while (1) {
		SerialConsoleWriteString("Updating display...\r\n");
		SSD1306_Fill(SSD1306_COLOR_BLACK);
		SSD1306_GotoXY(10, 25);
		SSD1306_Puts("LogicNinjas", &Font_11x18, SSD1306_COLOR_WHITE);

		// Update the entire buffer for consistency
		SSD1306_UpdateScreen();

		vTaskDelay(pdMS_TO_TICKS(1000));  // Update every second
	}
}

static void ButtonReadTask(void *pvParameters) {
	// Initialize MCP23017 port expander
	mcp23017_init();

	while (1) {
		// Read button states
		uint8_t gpio_state = mcp23017_read_gpio();

		// Print messages if buttons are pressed
		mcp23017_print_button(gpio_state);

		// Toggle LED based on button press
		if (gpio_state != 0xFF) { // Assuming 0xFF means no button press
			toggleLED(true);
		} else {
			toggleLED(false);
		}

		// Add a delay to debounce the buttons and limit the read frequency
		vTaskDelay(pdMS_TO_TICKS(200));
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

void toggleLED(bool state) {
	port_pin_set_output_level(LED_PIN, state);
}
