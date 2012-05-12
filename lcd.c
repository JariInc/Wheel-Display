#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "font8.h"
#include "font16.h"
#include "font40.h"
#include "lcd.h"
#include "ks0108.h"
#include "message.h"
#include "headers.h"

volatile LCDtext labels[14];

extern volatile uint16_t data[32];
extern volatile uint8_t lcdtypes[7];

void LCDupdate(void) {
	uint8_t i;

	if(GET_REDRAW(labels[0]))
		LCD_writegear(labels[0].number);

	for(i = 2; i < sizeof(labels)/sizeof(LCDtext); i++) {
		if(GET_REDRAW(labels[i])) {
			LCD_writestring(&labels[i]);
		}
	}
}

void LCDProcessMessage(uint8_t pos, uint8_t type, uint16_t value) {

	// gear is special case
	if(pos == POS_GEAR) {
		if(labels[POS_GEAR].number != value && value < 9) {
			labels[POS_GEAR].number = value;
			SET_TYPE(labels[POS_GEAR], TYPE_NULL);
			SET_REDRAW(labels[POS_GEAR]);
		}
	}
	else {
		// type change
		if(GET_TYPE(labels[pos]) != type || type == TYPE_RPM) {
			// header
			uint8_t i = 0;
			uint8_t index = type & 0b1111;
		
			// time value headers
			if(type & 0b10000) {
				do {
					labels[pos].text[i] = pgm_read_byte(&(headersTime[index][i]));
				} while(labels[pos].text[i++] != '\0');
			}
			// integer value headers
			else {
				do {
					labels[pos].text[i] = pgm_read_byte(&(headersInt[index][i]));
				} while(labels[pos].text[i++] != '\0');
			}

			labels[pos].len = i;
			SET_TYPE(labels[pos], type);
			SET_REDRAW(labels[pos]);
		}

		// value change
		if(labels[pos].number != value) {
			// Special formatting (5th byte on type is set)
			if(type & 0b10000) {
				uint8_t tenth = value & 0b111;
				uint8_t seconds = (value >> 3) & 0b111111;
				uint8_t minutes = (value >> 9) & 0b11111;
				uint8_t sign = value >> 15;
				char ret[STRING_MAX_LEN];
				char* retPtr = &ret;

				// sign
				
				if(sign) {
					*retPtr = '-';
					retPtr++;
				}

				// minutes
				if(minutes) {
					itoa(minutes, retPtr, 10);
					retPtr += strlen(retPtr);

					*retPtr = ':';
					retPtr++;

					if(seconds < 10) {
						*retPtr = '0';
						retPtr++;
					}
				}
					
				itoa(seconds, retPtr, 10);
				retPtr += strlen(retPtr);

				*retPtr = '.';
				retPtr++;

				itoa(tenth, retPtr, 10);

				strncpy(&labels[pos+1].text, &ret, STRING_MAX_LEN);
				labels[pos+1].len = strlen(&labels[pos+1].text);
			}
			else {
				labels[pos+1].number = value;
			}

			SET_REDRAW(labels[pos+1]);
			SET_TYPE(labels[pos+1], type);
		}
	}
}

#define INITLABEL(l,f,xc,yc) \
	labels[(l)].font = (f);\
	labels[(l)].x = (xc);\
	labels[(l)].y = (yc);\
	SET_REDRAW(labels[(l)]);

void LCDinit(void) {
	ks0108_init();

	labels[POS_GEAR].font = FONT_HUGE;
	labels[POS_GEAR].number = 8;
	SET_REDRAW(labels[POS_GEAR]);

	INITLABEL(POS_R1LH, FONT_SMALL, LEFTCOL, 0);
	INITLABEL(POS_R1LV, FONT_LARGE, LEFTCOL, 1);

	INITLABEL(POS_R1RH, FONT_SMALL, RIGHTCOL, 0);
	INITLABEL(POS_R1RV, FONT_LARGE, RIGHTCOL, 1);

	INITLABEL(POS_R2LH, FONT_SMALL, LEFTCOL, 3);
	INITLABEL(POS_R2LV, FONT_LARGE, LEFTCOL, 4);

	INITLABEL(POS_R2RH, FONT_SMALL, RIGHTCOL, 3);
	INITLABEL(POS_R2RV, FONT_LARGE, RIGHTCOL, 4);

	INITLABEL(POS_R3LH, FONT_SMALL, 0, 6);
	INITLABEL(POS_R3LV, FONT_SMALL, 0, 7);

	INITLABEL(POS_R3RH, FONT_SMALL, 64, 6);
	INITLABEL(POS_R3RV, FONT_SMALL, 64, 7);

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
	uint8_t i = 0;
	uint8_t isnum = FALSE;
	uint8_t x = t->x;

	if(t->len < 1) {
		itoa(t->number, t->text, 10);
		if(t->number < 10 && t->number > -10)
			t->len = 1;
		else if(t->number < 100 && t->number > -100)
			t->len = 2;
		else if(t->number < 1000 && t->number > -1000)
			t->len = 3;
		else if(t->number < 10000 && t->number > -10000)
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
				else if(t->text[i] == 0x2d)
					x = LCD_writechar_16(x, t->y, 0x0a);
				break;
			case FONT_SMALL:
				if((t->text[i] > 0x60) && (t->text[i] < 0x7b)) // a-z
					x = LCD_writechar_8(x, t->y, (t->text[i] - 0x61));
				else if((t->text[i] > 0x2f) && (t->text[i] < 0x3a)) // 0-9
					x = LCD_writechar_8(x, t->y, (t->text[i] - 0x16));
				else if(t->text[i] == 0x20) // space
					x++;
				else if(t->text[i] == 0x2e) // .
					x = LCD_writechar_8(x, t->y, 36);
				else if(t->text[i] == 0x3a) // :
					x = LCD_writechar_8(x, t->y, 37);
				else if(t->text[i] == 0x2d) // -
					x = LCD_writechar_8(x, t->y, 38);
				break;
			case FONT_HUGE:
				if(t->text[i] > 0x2f && t->text[i] < 0x3a)
					LCD_writegear(t->number);
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
	t->meta &= ~(1 << REDRAW);
}

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
