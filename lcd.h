#define TRUE 1
#define FALSE 0

#define FONT_SMALL 0
#define FONT_LARGE 1
#define FONT_HUGE 2

typedef struct {
	uint8_t x;
	uint8_t y;
	uint8_t len;
	uint8_t width;
	uint8_t redraw;
	uint8_t font;
	uint16_t number;
	char text[15];
} LCDtext;

uint8_t LCD_writechar_8(uint8_t x, uint8_t page, uint8_t character);
uint8_t LCD_writechar_16(uint8_t x, uint8_t page, uint8_t character);
//uint8_t LCD_writestring(uint8_t x, uint8_t page, char *string, uint8_t len, uint8_t large /*, uint8_t raling */);
void LCD_writestring(LCDtext *t);
//uint8_t LCD_writeint(uint8_t x, uint8_t page, uint16_t value, uint8_t len, uint8_t large);

void LCD_writegear(uint8_t gear);
void LCDloop();
void LCDinit();

