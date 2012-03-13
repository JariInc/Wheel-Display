#include <stdlib.h>
#include <string.h>
#include "font8.h"
#include "font16.h"
#include "font40.h"
#include "lcd.h"
#include "ks0108.h"
#include "spi.h"
#include "mcp3204.h"
#include "mcp23s17.h"

/* debug stuff */
#include "timer.h"
extern volatile buttons btns;

extern volatile LCDtext speed;
extern volatile LCDtext rpm;
extern volatile uint8_t rpmleds;
extern volatile LCDtext gear;

void LCDinit() {
	ks0108_init();

	LCDtext t;
	t.redraw = 1;
	t.font = FONT_SMALL;

	// 1st line
	t.x = 28;
	t.y = 0;
	t.redraw = TRUE;
	t.len = 3;
	strcpy(t.text, "rpm");
	LCD_writestring(&t);

	t.x = 80;
	t.redraw = TRUE;
	t.len = 5;
	strcpy(t.text, "speed\0");
	LCD_writestring(&t);


	// 2nd line
	t.x = 28;
	t.y = 3;
	t.redraw = TRUE;
	t.len = 4;
	strcpy(t.text, "fuel\0");
	LCD_writestring(&t);

	t.x = 80;
	t.redraw = TRUE;
	t.len = 3;
	strcpy(t.text, "lap\0");
	LCD_writestring(&t);

	// 3rd line
	t.x = 0;
	t.y = 6;
	t.redraw = TRUE;
	t.len = 7;
	strcpy(t.text, "laptime\0");
	LCD_writestring(&t);

	// data
	t.font = FONT_LARGE;
	t.x = 28;
	t.y = 4;
	t.redraw = TRUE;
	t.len = 0;
	t.number = 90;
	LCD_writestring(&t); // fuel

	t.number = 42;
	t.x = 80;
	t.redraw = TRUE;
	LCD_writestring(&t); // lap

	//LCDdrawGraticule(0);

	speed.x = 80;
	speed.y = 1;
	speed.redraw = TRUE;
	speed.font = FONT_LARGE;
	LCD_writestring(&speed);

	rpm.x = 28;
	rpm.y = 1;
	rpm.redraw = TRUE;
	rpm.font = FONT_LARGE;
	LCD_writestring(&rpm);

	gear.x = 0;
	gear.y = 0;
	gear.redraw = TRUE;
	gear.font = FONT_HUGE;
	LCD_writestring(&gear);
}

void LCDloop() {
/*
	uint16_t inc = 127; //ADCGetValue(0);

	rpmleds = 0xff >> (7 - ((rpm.number) >> 10));
	GPIOWrite(0x00, rpmleds);

	// speed
	if(speed.number < 300)
		speed.number++;
	else
		speed.number = 0;

	speed.redraw = TRUE;
	LCD_writestring(&speed);
	
	// rpm
	rpm.number += (inc >> 4);
	rpm.redraw = TRUE;
	LCD_writestring(&rpm);

	// gear
	if(rpm.number > 8250) {
		gear.number++;
		gear.number &= 7;
		gear.redraw = TRUE;
		LCD_writestring(&gear);
		rpm.number = 1000;
	}
*/
/*
	// adc
	ks0108_clear_range(7, 7, 0, 127);
	LCDtext dump = {0, 7, 0, 0, 1, FONT_SMALL, inc, ""};
	LCD_writestring(&dump);

	dump.x = 28;
	dump.number = 0; //ADCGetValue(1);
	dump.redraw = TRUE;
	LCD_writestring(&dump);
*/
}

uint8_t LCD_writechar_8(uint8_t x, uint8_t page, uint8_t character) {

	uint8_t i, controller = 0;

	if(x < 64)
		controller = 0;
	else
		controller = 1;

	ks0108_instruction(controller, (0xb8 + page), 0);					// set page (0-7)

	/*
		set address
		If x > 64 take remainder and use that as the address.
		If drawing is crosses to another controller address is zeroed (see below)
	*/
	ks0108_instruction(controller, 0x40 + (x % (controller * 64)), 0); // set address (0-63)

	for(i=0; i<7; i++) {
		if(pgm_read_byte(&(font8[character][i])) != 0x00) {			// skip empty lines

			if(x == 64) { 											// crossing the line from controller to another
				controller = 1;
				ks0108_instruction(controller, (0xb8 + page), 0);		// set page
				ks0108_instruction(controller, 0x40, 0);				// set address start from zero
			}

			if(x < 128) {											// check if we are still on screen
					ks0108_instruction(controller, pgm_read_byte(&(font8[character][i])), 1);
					x++;
			}
		}
	}
	// add space between letters
	ks0108_instruction(controller, 0x00, 1);
	return ++x;
		
}

uint8_t LCD_writechar_16(uint8_t x, uint8_t page, uint8_t character) {

	uint8_t i, j, startx, controller = 0;

	startx = x;

	for(i=0; i<2; i++) {

		x = startx;

		if(x < 64)
			controller = 0;
		else
			controller = 1;

		ks0108_instruction(controller, (0xb8 + page + i), 0);
		ks0108_instruction(controller, 0x40 + (x % (controller * 64)), 0);

		for(j=0; j<9; j++) {
			if((pgm_read_byte(&(font16[character][0][j])) | pgm_read_byte(&(font16[character][1][j]))) != 0x00) {

				if(x == 64) { 											// crossing the line from controller to another
					controller = 1;
					ks0108_instruction(controller, (0xb8 + page + i), 0);		// set page
					ks0108_instruction(controller, 0x40, 0);					// set address start from zero
				}

				if(x < 128) {											// check if we are still on screen
					ks0108_instruction(controller, pgm_read_byte(&(font16[character][i][j])), 1);
					x++;
				}
			}
		}
	}

	
	if(x < 128) {
		for(i=0; i<2; i++) {
			ks0108_instruction(controller, (0xb8 + page + i), 0);
			ks0108_instruction(controller, 0x40 + (x % (controller * 64)), 0);
			ks0108_instruction(controller, 0x00, 1); 							// space between letters
		}
		x++;
	}
	
	return x;
		
}

void LCD_writestring(LCDtext *t) {

	if(t->redraw) {
		uint8_t i = 0;
		uint8_t isnum = FALSE;
		uint8_t x = t->x;

		if(t->len < 1) {
			utoa(t->number, t->text, 0x0a);
			if(t->number < 10)
				t->len = 1;
			else if(t->number < 100)
				t->len = 2;
			else if(t->number < 1000)
				t->len = 3;
			else if(t->number < 1000)
				t->len = 4;
			else
				t->len = 5;
		
			isnum = TRUE;
		}

		for(i=0; i < t->len; i++) {
			switch(t->font) {
				case FONT_LARGE:
					if(t->text[i] > 0x2f && t->text[i] < 0x3a)
						x = LCD_writechar_16(x, t->y, (t->text[i] - 0x30));
					break;
				case FONT_SMALL:
					if((t->text[i] > 0x60) && (t->text[i] < 0x7b))
						x = LCD_writechar_8(x, t->y, (t->text[i] - 0x61));
					else if((t->text[i] > 0x2f) && (t->text[i] < 0x3a))
						x = LCD_writechar_8(x, t->y, (t->text[i] - 0x16));
					else if(t->text[i] == 0x20)
						x++;
					else if(t->text[i] == 0x2e)
						x = LCD_writechar_8(x, t->y, 36);
					else if(t->text[i] == 0x3a)
						x = LCD_writechar_8(x, t->y, 37);
					break;
				case FONT_HUGE:
					if(t->text[i] > 0x2f && t->text[i] < 0x3a)
						/*x = */LCD_writegear(t->number);
					break;
			}
			 // space between chars
			//x++;
		}

		// keep number field as is
		if(isnum)
			t->len = 0;
	
		// check if clean up needed
		if(x < (t->x+t->width)) {
			switch(t->font) {
				case FONT_SMALL:
					ks0108_clear_range(t->y, t->y, x, (t->x+t->width));
					break;
				case FONT_LARGE:
					ks0108_clear_range(t->y, (t->y)+1, x, (t->x+t->width));
					break;
			}
		}

		// save
		t->width = x - t->x;
		t->redraw = FALSE;
	}
}

/*
uint8_t LCD_writestring(uint8_t x, uint8_t page, char *string, uint8_t len, uint8_t large) {

	uint8_t i = 0;

	for(i=0; i<len; i++) {
		if(large != 0) {
			if(string[i] > 0x2f && string[i] < 0x3a)
				x = LCD_writechar_16(x, page, (string[i] - 0x30));
		}
		else {
			if((string[i] > 0x60) && (string[i] < 0x7b))
				x = LCD_writechar_8(x, page, (string[i] - 0x61));
			else if((string[i] > 0x2f) && (string[i] < 0x3a))
				x = LCD_writechar_8(x, page, (string[i] - 0x16));
			else if(string[i] == 0x20)
				x++;
			else if(string[i] == 0x2e)
				x = LCD_writechar_8(x, page, 36);
			else if(string[i] == 0x3a)
				x = LCD_writechar_8(x, page, 37);
		}

		 // space between chars
		//x++;
	}

	return x;

}

uint8_t LCD_writeint(uint8_t x, uint8_t page, uint16_t value, uint8_t len, uint8_t large) {

	LCDtext t { 5, 1,  };

	utoa(value, str, 0x0a);

	return LCD_writestring(x, page, str, len, large);

}
*/
void LCD_writegear(uint8_t gear) {

	ks0108_clear_range(0, 5, 0, 25);

	uint8_t i, line = 0;

	for(line=0; line<5; line++) {

		ks0108_instruction(0, (0xb8 + line), 0);		// set page
		ks0108_instruction(0, 0x40, 0);					// set address (zero)

		for(i=0; i<25; i++)
			ks0108_instruction(0, pgm_read_byte(&(font40[gear][line][i])), 1);
	}
}
