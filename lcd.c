#include <stdlib.h> 
#include "font8.h"
#include "font16.h"
#include "font40.h"
#include "lcd.h"
#include "ks0108.h"
#include "spi.h"
#include "mcp3204.h"
#include "mcp23s17.h"

extern volatile uint8_t outputwidth;
extern volatile uint16_t speed;
extern volatile uint8_t speed_width;
extern volatile uint8_t rpm_width;
extern volatile uint16_t rpm;
extern volatile uint8_t rpmleds;
extern volatile uint8_t gear;

void LCDinit() {
	ks0108_init();

	// 1st line
	LCD_writestring(28, 0, "rpm", 3, 0);
	LCD_writestring(80, 0, "speed", 5, 0);
	// 2nd line
	LCD_writestring(28, 3, "fuel", 4, 0);
	LCD_writestring(80, 3, "lap", 3, 0);

	// 3rd line
	LCD_writestring(0, 6, "laptime", 7, 0);

	// data
	LCD_writeint(80, 1, speed, 3, 1); // speed
	LCD_writeint(28, 4, 90, 4, 1); // fuel
	LCD_writeint(80, 4, 42, 4, 1); // lap

	LCD_writegear(gear);
	//LCDdrawGraticule(0);

}

void LCDloop() {
	uint16_t inc = ADCGetValue(0);
	
	//USART_Transmit((char)ADCGetValue(0));
	//GPIOWrite(0x00, (char)ADCGetValue(0));

	rpmleds = 0xff >> (7 - (rpm >> 10));
	GPIOWrite(0x00, rpmleds);

	// speed
	outputwidth = LCD_writeint(80, 1, speed, 3, 1);
	if(speed_width > outputwidth) {
		ks0108_clear_range(1, 2, outputwidth, speed_width);
	}
	speed_width = outputwidth;

	// gear
	if(rpm > (1 << 13)) {
		LCD_writegear(gear++ & 8);
		rpm = 3000;
	}
	
	// rpm
	outputwidth = LCD_writeint(28, 1, rpm, 5, 1);
	// clear the remainder
	if(rpm_width > outputwidth) {
		ks0108_clear_range(1, 2, outputwidth, rpm_width);
	}

	rpm_width = outputwidth;
	rpm += (inc >> 4);

	ks0108_clear_range(7, 7, 0, 4*6);
	LCD_writeint(0, 7, inc, 4, 0);

	if(speed < 300)
		speed++;
	else
		speed = 0;
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

uint8_t LCD_writestring(uint8_t x, uint8_t page, char *string, uint8_t len, uint8_t large /*, uint8_t raling */) {

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

	char str[6];

	utoa(value, str, 0x0a);

	return LCD_writestring(x, page, str, len, large);

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
