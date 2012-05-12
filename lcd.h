#include <avr/pgmspace.h>

#define TRUE 1
#define FALSE 0

#define FONT_SMALL 0
#define FONT_LARGE 1
#define FONT_HUGE 2

#define STRING_MAX_LEN 9

#define LEFTCOL 28
#define RIGHTCOL 80

#define REDRAW 5
#define GET_REDRAW(x) ((x.meta >> 5) & 1)
#define SET_REDRAW(x) (x.meta |= (1 << 5))
#define RESET_REDRAW(x) (x.meta &= ~(1 << 5))

#define GET_TYPE(x) (x.meta & 0b11111)
#define SET_TYPE(x, y) (x.meta |= (y & 0b11111))

typedef struct {
	uint8_t x;
	uint8_t y;
	uint8_t len;
	uint8_t width;
	uint8_t font;
	uint16_t number;
	char text[STRING_MAX_LEN];
	uint8_t meta; // 0bRRrttttt, R=reserved
} LCDtext;

uint8_t LCD_writechar_8(uint8_t x, uint8_t page, uint8_t character);
uint8_t LCD_writechar_16(uint8_t x, uint8_t page, uint8_t character);
void LCD_writestring(LCDtext *t);
void LCD_writegear(uint8_t gear);
void LCDinit(void);
void LCDupdate(void);
void LCDProcessMessage(uint8_t pos, uint8_t type, uint16_t value);
