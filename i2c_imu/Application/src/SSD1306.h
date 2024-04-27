#ifndef SSD1306_H_
#define SSD1306_H_

#include "fonts.h"  // Assuming this contains the definition for FontDef_t

#ifdef __cplusplus
extern "C" {
	#endif

	/******************************************************************************
	* Defines and Typedefs
	******************************************************************************/
	#define SSD1306_I2C_ADDRESS 0x3D
	#define SSD1306_WIDTH 128
	#define SSD1306_HEIGHT 64

	// Color definition
	typedef enum {
		SSD1306_COLOR_BLACK = 0,
		SSD1306_COLOR_WHITE = 1
	} SSD1306_COLOR_t;

	// Structure to manage SSD1306 display settings
	typedef struct {
		uint16_t CurrentX;
		uint16_t CurrentY;
		uint8_t Inverted;
		uint8_t Initialized;
	} SSD1306_t;

	/******************************************************************************
	* Public function prototypes
	******************************************************************************/
	uint8_t SSD1306_Init(void);
	void SSD1306_UpdateScreen(void);
	void SSD1306_Clear(void);
	void SSD1306_Fill(SSD1306_COLOR_t color);
	void SSD1306_ON(void);
	void SSD1306_OFF(void);
	void SSD1306_ToggleInvert(void);
	void SSD1306_DrawPixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color);
	void SSD1306_GotoXY(uint16_t x, uint16_t y);
	char SSD1306_Putc(char ch, FontDef_t* Font, SSD1306_COLOR_t color);
	char SSD1306_Puts(char* str, FontDef_t* Font, SSD1306_COLOR_t color);
	void SSD1306_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t color);
	void SSD1306_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t color);
	void SSD1306_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t color);
	void SSD1306_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR_t color);
	void SSD1306_DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR_t color);
	void SSD1306_DrawCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t color);
	void SSD1306_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t color);

	#ifdef __cplusplus
}
#endif

#endif // SSD1306_H_
