/**
 * @file      BootMain.c
 * @brief     Main file for the ESE5160 bootloader. Handles updating the main application
 * @details   Main file for the ESE5160 bootloader. Handles updating the main application
 * @author    Eduardo Garcia
 * @author    Nick M-G
 * @date      2024-03-03
 * @version   2.0
 * @copyright Copyright University of Pennsylvania
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "conf_example.h"
#include "sd_mmc_spi.h"
#include <asf.h>
#include <string.h>

#include "ASF/sam0/drivers/dsu/crc32/crc32.h"
#include "SD Card/SdCard.h"
#include "SerialConsole/SerialConsole.h"
#include "Systick/Systick.h"
#include "status_codes.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
#define APP_START_ADDRESS           ((uint32_t) 0x12000)                    ///< Start of main application. Must be address of start of main application
#define APP_START_RESET_VEC_ADDRESS (APP_START_ADDRESS + (uint32_t) 0x04)   ///< Main application reset vector address
#define STATUS_OK		0
#define STATUS_ERR		1

/******************************************************************************
 * Structures and Enumerations
 ******************************************************************************/

struct usart_module cdc_uart_module;   ///< Structure for UART module connected to EDBG (used for unit test output)

/******************************************************************************
 * Local Function Declaration
 ******************************************************************************/
static void jumpToApplication(void);
static bool StartFilesystemAndTest(void);
static void configure_nvm(void);

/******************************************************************************
 * Global Variables
 ******************************************************************************/
// INITIALIZE VARIABLES
char test_file_name[] = "0:sd_mmc_test.txt";   ///< Test TEXT File name
char test_bin_file[] = "0:sd_binary.bin";      ///< Test BINARY File name
Ctrl_status status;                            ///< Holds the status of a system initialization
FRESULT res;                                   // Holds the result of the FATFS functions done on the SD CARD TEST
FATFS fs;                                      // Holds the File System of the SD CARD
FIL file_object;                               // FILE OBJECT used on main for the SD Card Test

/******************************************************************************
 * Global Functions
 ******************************************************************************/

/**
* @fn		int main(void)
* @brief	Main function for ESE5160 Bootloader Application

* @return	Unused (ANSI-C compatibility).
* @note		Bootloader code initiates here.
*****************************************************************************/

int main(void) {

    /*1.) INIT SYSTEM PERIPHERALS INITIALIZATION*/
    system_init();
    delay_init();
    InitializeSerialConsole();
    system_interrupt_enable_global();

    /* Initialize SD MMC stack */
    sd_mmc_init();

    // Initialize the NVM driver
    configure_nvm();

    irq_initialize_vectors();
    cpu_irq_enable();

    // Configure CRC32
    dsu_crc32_init();

    SerialConsoleWriteString("ESE5160 - ENTER BOOTLOADER");   // Order to add string to TX Buffer

    /*END SYSTEM PERIPHERALS INITIALIZATION*/

    /*2.) STARTS SIMPLE SD CARD MOUNTING AND TEST!*/

    // EXAMPLE CODE ON MOUNTING THE SD CARD AND WRITING TO A FILE
    // See function inside to see how to open a file
    SerialConsoleWriteString("\x0C\n\r-- SD/MMC Card Example on FatFs --\n\r");

    if (StartFilesystemAndTest() == false) {
        SerialConsoleWriteString("SD CARD failed! Check your connections. System will restart in 5 seconds...");
        delay_cycles_ms(5000);
        system_reset();
    } else {
        SerialConsoleWriteString("SD CARD mount success! Filesystem also mounted. \r\n");
    }

    /*END SIMPLE SD CARD MOUNTING AND TEST!*/

    /*3.) STARTS BOOTLOADER HERE!*/
	
	int flashEraseStatus;	// Status variable
	
	//Constant strings with the names of files
	char flagA[10] = "FlagA.txt";
	char flagB[10] = "FlagB.txt";
	char testA[10] = "TestA.bin";
	char testB[10] = "TestB.bin";
	
	// Check which flag file (FlagA.txt or FlagB.txt) is present in the SD card
	// Set a variable called firmwareFlag appropriately
	int firmwareFlag = 0;
	FRESULT fileFlagStatus;
	fileFlagStatus = f_open(&file_object, flagA, FA_OPEN_EXISTING);
	if(fileFlagStatus == FR_OK && firmwareFlag == 0)
	{
		firmwareFlag = 1;
		SerialConsoleWriteString("FlagA.txt found. Flashing firmware TestA.bin\r\n");
		
	}
	
	fileFlagStatus = f_open(&file_object, flagB, FA_OPEN_EXISTING);
	if(fileFlagStatus == FR_OK && firmwareFlag == 0)
	{
		firmwareFlag = 2;
		SerialConsoleWriteString("FlagB.txt found. Flashing firmware TestB.bin\r\n");
	}
	f_close(&file_object);
	
	if(firmwareFlag != 0)
	{
		// Erase the flash and check if it has been erased.
		// Runs in a do-while loop until all flash has been erased successfully
		do 
		{
			for(int i=0; i<128; i++)
			{
				int status = nvm_erase_row(APP_START_ADDRESS + i*256);
				if(status != STATUS_OK)
				{
					SerialConsoleWriteString("Erase error\r\n");
					flashEraseStatus = STATUS_ERR_BAD_DATA;
				}
				else
				{
					flashEraseStatus = STATUS_OK;
				}
			}
		
			if(status == STATUS_OK){
				SerialConsoleWriteString("Flash erased. Checking..\r\n");
			}
		
			if(flashEraseStatus == STATUS_OK)
			{
				for(int i=0; i<128*256; i++)
				{
					char *flash_char = (char*) (APP_START_ADDRESS+i);
					if(*flash_char != 0xFF)
					{
						SerialConsoleWriteString("Flash not erased successfully, rerunning flash\r\n");
						flashEraseStatus = STATUS_ERR;
					}
				}
			}
		} while (flashEraseStatus != STATUS_OK);
	
		SerialConsoleWriteString("Flash erased successfully.\r\n");
	
		if(firmwareFlag == 0)
		{
			SerialConsoleWriteString("Neither flag found\r\n");
		}
	
		// Read the file depending on the value of firmwareFlag
	
		uint8_t readbuffer[64];
		UINT bytesToRead;
		UINT bytesRead;
		DWORD filesize;
		unsigned int pageNumber = 0;
		FRESULT fileStatus;
	
		if(firmwareFlag == 1)
		{
			fileStatus = f_open(&file_object, testA, FA_READ);
		}
		else
		{
			fileStatus = f_open(&file_object, testB, FA_READ);
		}
		filesize = f_size(&file_object);
		bytesToRead = 64;
		bytesRead = 0;
		UINT totalBytesRead = 0;
	
		char consoleBuf[25];
	
		// While there are still bytes to be read, read from file and write to NVM
		while(totalBytesRead < filesize)
		{
			FRESULT res = f_read(&file_object, readbuffer, bytesToRead, &bytesRead);
			if(res != FR_OK)
			{
				SerialConsoleWriteString("Error in reading file\r\n");
				break;
			}
			nvm_write_buffer(APP_START_ADDRESS+(pageNumber*64), readbuffer, 64);
			pageNumber+=1;
			totalBytesRead += bytesRead;
			if(filesize - totalBytesRead < 64)
			{
				bytesToRead = filesize - totalBytesRead;
			}
			SerialConsoleWriteString("Wrote 1 page into NVM\r\n");
		}
	
		// Delete the flag file because the other one would have been written.
		if(firmwareFlag == 1)
		{
			f_unlink(flagA);
			SerialConsoleWriteString("FlagA.txt deleted.\r\n");
		}
		else if (firmwareFlag == 2)
		{
			f_unlink(flagB);
			SerialConsoleWriteString("FlagB.txt deleted.\r\n");
		}
	
		f_close(&file_object);
	
		SerialConsoleWriteString("Closed File");
		}
	
    /* END BOOTLOADER HERE!*/

    // 4.) DEINITIALIZE HW AND JUMP TO MAIN APPLICATION!
    SerialConsoleWriteString("ESE5160 - EXIT BOOTLOADER");   // Order to add string to TX Buffer
    delay_cycles_ms(100);                                    // Delay to allow print

    // Deinitialize HW - deinitialize started HW here!
    DeinitializeSerialConsole();   // Deinitializes UART
    sd_mmc_deinit();               // Deinitialize SD CARD

    // Jump to application
    jumpToApplication();

    // Should not reach here! The device should have jumped to the main FW.
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/

/**
 * function      static void StartFilesystemAndTest()
 * @brief        Starts the filesystem and tests it. Sets the filesystem to the global variable fs
 * @details      Jumps to the main application. Please turn off ALL PERIPHERALS that were turned on by the bootloader
 *				before performing the jump!
 * @return       Returns true is SD card and file system test passed. False otherwise.
 ******************************************************************************/
static bool StartFilesystemAndTest(void) {
    bool sdCardPass = true;
    uint8_t binbuff[256];

    // Before we begin - fill buffer for binary write test
    // Fill binbuff with values 0x00 - 0xFF
    for (int i = 0; i < 256; i++) {
        binbuff[i] = i;
    }

    // MOUNT SD CARD
    Ctrl_status sdStatus = SdCard_Initiate();
    if (sdStatus == CTRL_GOOD)   // If the SD card is good we continue mounting the system!
    {
        SerialConsoleWriteString("SD Card initiated correctly!\n\r");

        // Attempt to mount a FAT file system on the SD Card using FATFS
        SerialConsoleWriteString("Mount disk (f_mount)...\r\n");
        memset(&fs, 0, sizeof(FATFS));
        res = f_mount(LUN_ID_SD_MMC_0_MEM, &fs);   // Order FATFS Mount
        if (FR_INVALID_DRIVE == res) {
            LogMessage(LOG_INFO_LVL, "[FAIL] res %d\r\n", res);
            sdCardPass = false;
            goto main_end_of_test;
        }
        SerialConsoleWriteString("[OK]\r\n");

        // Create and open a file
        SerialConsoleWriteString("Create a file (f_open)...\r\n");

        test_file_name[0] = LUN_ID_SD_MMC_0_MEM + '0';
        res = f_open(&file_object, (char const *) test_file_name, FA_CREATE_ALWAYS | FA_WRITE);

        if (res != FR_OK) {
            LogMessage(LOG_INFO_LVL, "[FAIL] res %d\r\n", res);
            sdCardPass = false;
            goto main_end_of_test;
        }

        SerialConsoleWriteString("[OK]\r\n");

        // Write to a file
        SerialConsoleWriteString("Write to test file (f_puts)...\r\n");

        if (0 == f_puts("Test SD/MMC stack\n", &file_object)) {
            f_close(&file_object);
            LogMessage(LOG_INFO_LVL, "[FAIL]\r\n");
            sdCardPass = false;
            goto main_end_of_test;
        }

        SerialConsoleWriteString("[OK]\r\n");
        f_close(&file_object);   // Close file
        SerialConsoleWriteString("Test is successful.\n\r");

        // Write binary file
        // Read SD Card File
        test_bin_file[0] = LUN_ID_SD_MMC_0_MEM + '0';
        res = f_open(&file_object, (char const *) test_bin_file, FA_WRITE | FA_CREATE_ALWAYS);

        if (res != FR_OK) {
            SerialConsoleWriteString("Could not open binary file!\r\n");
            LogMessage(LOG_INFO_LVL, "[FAIL] res %d\r\n", res);
            sdCardPass = false;
            goto main_end_of_test;
        }

        // Write to a binaryfile
        SerialConsoleWriteString("Write to test file (f_write)...\r\n");
        uint32_t varWrite = 0;
        if (0 != f_write(&file_object, binbuff, 256, &varWrite)) {
            f_close(&file_object);
            LogMessage(LOG_INFO_LVL, "[FAIL]\r\n");
            sdCardPass = false;
            goto main_end_of_test;
        }

        SerialConsoleWriteString("[OK]\r\n");
        f_close(&file_object);   // Close file
        SerialConsoleWriteString("Test is successful.\n\r");

    main_end_of_test:
        SerialConsoleWriteString("End of Test.\n\r");

    } else {
        SerialConsoleWriteString("SD Card failed initiation! Check connections!\n\r");
        sdCardPass = false;
    }

    return sdCardPass;
}

/**
 * function      static void jumpToApplication(void)
 * @brief        Jumps to main application
 * @details      Jumps to the main application. Please turn off ALL PERIPHERALS that were turned on by the bootloader
 *				before performing the jump!
 * @return
 ******************************************************************************/
static void jumpToApplication(void) {
    // Function pointer to application section
    void (*applicationCodeEntry)(void);

    // Rebase stack pointer
    __set_MSP(*(uint32_t *) APP_START_ADDRESS);

    // Rebase vector table
    SCB->VTOR = ((uint32_t) APP_START_ADDRESS & SCB_VTOR_TBLOFF_Msk);

    // Set pointer to application section
    applicationCodeEntry = (void (*)(void))(unsigned *) (*(unsigned *) (APP_START_RESET_VEC_ADDRESS));

    // Jump to application. By calling applicationCodeEntry() as a function we move the PC to the point in memory pointed by applicationCodeEntry,
    // which should be the start of the main FW.
    applicationCodeEntry();
}

/**
 * function      static void configure_nvm(void)
 * @brief        Configures the NVM driver
 * @details
 * @return
 ******************************************************************************/
static void configure_nvm(void) {
    struct nvm_config config_nvm;
    nvm_get_config_defaults(&config_nvm);
    config_nvm.manual_page_write = false;
    nvm_set_config(&config_nvm);
}
