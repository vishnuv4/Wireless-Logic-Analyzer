/**
 * @file      main.c
 * @brief     Main application entry point
 * @author    Eduardo Garcia
 * @author    Nick M-G
 * @date      2022-04-14
 ******************************************************************************/

/****
 * Includes
 ******************************************************************************/
#include <errno.h>

#include "CliThread/CliThread.h"
#include "FreeRTOS.h"
#include "I2cDriver\I2cDriver.h"
#include "SerialConsole.h"
#include "WifiHandlerThread/WifiHandler.h"
#include "asf.h"
#include "driver/include/m2m_wifi.h"
#include "main.h"
#include "stdio_serial.h"
#include "rtc.h"
#include "adc_spi.h"

/****
 * Defines and Types
 ******************************************************************************/
#define APP_TASK_ID 0 /**< @brief ID for the application task */
#define CLI_TASK_ID 1 /**< @brief ID for the command line interface task */

#define RTC_TASK_SIZE 100
#define RTC_PRIORITY 3
/****
 * Local Function Declaration
 ******************************************************************************/
void vApplicationIdleHook(void);
//!< Initial task used to initialize HW before other tasks are initialized
static void StartTasks(void);
void vApplicationDaemonTaskStartupHook(void);

void vApplicationStackOverflowHook(void);
void vApplicationMallocFailedHook(void);
void vApplicationTickHook(void);

/****
 * Variables
 ******************************************************************************/
static TaskHandle_t cliTaskHandle = NULL;       //!< CLI task handle
static TaskHandle_t daemonTaskHandle = NULL;    //!< Daemon task handle
static TaskHandle_t wifiTaskHandle = NULL;      //!< Wifi task handle
static TaskHandle_t uiTaskHandle = NULL;        //!< UI task handle
static TaskHandle_t controlTaskHandle = NULL;   //!< Control task handle
static TaskHandle_t rtcTaskHandle = NULL;
static TaskHandle_t adcSpiTaskHandle = NULL;

char bufferPrint[64];   ///< Buffer for daemon task

/**
 * @brief Main application function.
 * Application entry point.
 * @return int
 */
int main(void) {
    /* Initialize the board. */
    system_init();

    /* Initialize the UART console. */
    InitializeSerialConsole();

    // Initialize trace capabilities
    vTraceEnable(TRC_START);
    // Start FreeRTOS scheduler
    vTaskStartScheduler();

    return 0;   // Will not get here
}

/**
 * function          vApplicationDaemonTaskStartupHook
 * @brief            Initialization code for all subsystems that require FreeRToS
 * @details			This function is called from the FreeRToS timer task. Any code
 *					here will be called before other tasks are initilized.
 * @param[in]        None
 * @return           None
 */
void vApplicationDaemonTaskStartupHook(void) {
    SerialConsoleWriteString("\r\n\r\n-----ESE516 Main Program-----\r\n");

    // Initialize HW that needs FreeRTOS Initialization
    SerialConsoleWriteString("\r\n\r\nInitialize HW...\r\n");
    if (I2cInitializeDriver() != STATUS_OK) {
        SerialConsoleWriteString("Error initializing I2C Driver!\r\n");
    } else {
        SerialConsoleWriteString("Initialized I2C Driver!\r\n");
    }

    StartTasks();

    vTaskSuspend(daemonTaskHandle);
}

/**
 * function          StartTasks
 * @brief            Initialize application tasks
 * @details
 * @param[in]        None
 * @return           None
 */
static void StartTasks(void) {
    snprintf(bufferPrint, 64, "Heap before starting tasks: %d\r\n", xPortGetFreeHeapSize());
    SerialConsoleWriteString(bufferPrint);

    // Initialize Tasks here

    if (xTaskCreate(vCommandConsoleTask, "CLI_TASK", CLI_TASK_SIZE, NULL, CLI_PRIORITY, &cliTaskHandle) != pdPASS) {
        SerialConsoleWriteString("ERR: CLI task could not be initialized!\r\n");
    }

    snprintf(bufferPrint, 64, "Heap after starting CLI: %d\r\n", xPortGetFreeHeapSize());
    SerialConsoleWriteString(bufferPrint);
	
	/*if (xTaskCreate(vRtcTask, "Rtc_TASK", RTC_TASK_SIZE, NULL, RTC_PRIORITY, &rtcTaskHandle) != pdPASS) {
		SerialConsoleWriteString("ERR: CLI task could not be initialized!\r\n");
	}

	snprintf(bufferPrint, 64, "Heap after starting CLI: %d\r\n", xPortGetFreeHeapSize());
	SerialConsoleWriteString(bufferPrint);*/

    if (xTaskCreate(vWifiTask, "WIFI_TASK", WIFI_TASK_SIZE, NULL, WIFI_PRIORITY, &wifiTaskHandle) != pdPASS) {
        SerialConsoleWriteString("ERR: WIFI task could not be initialized!\r\n");
    }
    snprintf(bufferPrint, 64, "Heap after starting WIFI: %d\r\n", xPortGetFreeHeapSize());
    SerialConsoleWriteString(bufferPrint);
	
	if (xTaskCreate(vAdcSpiTask, "ADC_SPI_TASK", ADC_SPI_TASK_SIZE, NULL, ADC_SPI_PRIORITY, &adcSpiTaskHandle) != pdPASS) {
		SerialConsoleWriteString("ERR: ADC SPI task could not be initialized!\r\n");
	}
	snprintf(bufferPrint, 64, "Heap after starting SPI: %d\r\n", xPortGetFreeHeapSize());
	SerialConsoleWriteString(bufferPrint);
	
}

void vApplicationMallocFailedHook(void) {
    SerialConsoleWriteString("Error on memory allocation on FREERTOS!\r\n");
    while (1)
        ;
}

void vApplicationStackOverflowHook(void) {
    SerialConsoleWriteString("Error on stack overflow on FREERTOS!\r\n");
    while (1)
        ;
}

#include "MCHP_ATWx.h"
void vApplicationTickHook(void) { SysTick_Handler_MQTT(); }
