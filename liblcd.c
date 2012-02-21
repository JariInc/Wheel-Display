#include "liblcd.h"
#include "font8.h"
#include "font16.h"
#include "font40.h"

void LCD_controller_disable(uint8_t i) {
	switch(i) {
		case 0:
			LCD_CTRL |= CS1;
			break;
		case 1:	
			LCD_CTRL |= CS2;
			break;
	}

}

void LCD_controller_enable(uint8_t i) {
	switch(i) {
		case 0:
			LCD_CTRL &= ~CS1;
			break;
		case 1:	
			LCD_CTRL &= ~CS2;
			break;
	}
}

uint8_t LCD_readdata(uint8_t controller, uint8_t status) {
	uint8_t data = 0x00;

	LCD_controller_enable(controller);

	LCD_DATA = 0x00;
	LCD_DATA_DIR = 0x00;
	LCD_CTRL |= RW;

	if(status != 0) { // read LCD status
		LCD_CTRL &= ~RS;	
		LCD_CTRL |= EN;
		asm volatile ("nop");
		data = LCD_DATA;
		LCD_CTRL &= ~EN;
	}
	else {			// read data RAM
		LCD_CTRL |= RS;

		// dummy read
		LCD_CTRL |= EN;
		asm volatile ("nop");
		LCD_CTRL &= ~EN;
		
		// wait data fetch
		//while(LCD_readdata(controller, 1) & 0x80);

		// actual read
		LCD_CTRL |= EN;
		asm volatile ("nop");
		LCD_CTRL &= ~EN;

		// wait data fetch
		while(LCD_readdata(controller, 1) & 0x80);

		data = LCD_DATA;
	}

	LCD_controller_disable(controller);

	return data;
}

void LCD_instruction(uint8_t controller, uint8_t instruction, uint8_t data) {
	// wait until pin7 goes low
	while(LCD_readdata(controller, 1) & 0x80);

	LCD_controller_enable(controller);

	LCD_DATA_DIR = 0xff;
	LCD_CTRL &= ~RW;

	if(data != 0)	// write data RAM
		LCD_CTRL |= RS;
	else			// write LCD instruction
		LCD_CTRL &= ~RS;

	LCD_DATA = instruction;

	LCD_CTRL |= EN;
	asm volatile ("nop");
	LCD_CTRL &= ~EN;

	LCD_controller_disable(controller);
}

void LCD_initialize() {

	LCD_CTRL_DIR |= (RS | RW | CS1 | CS2 | EN);	// ctrl direction (out)

	LCD_CTRL |= (CS1 | CS2);	// disable both controllers (logic high)

	LCD_clear();

}

void LCD_clear() {

	uint8_t controller = 0;
	uint8_t page = 0;
	uint8_t address = 0;

	for(controller = 0; controller < 2; controller++) {

		LCD_instruction(controller, 0x3f, 0); // turn display on
		LCD_instruction(controller, 0xc0, 0); // set start address

		for(page = 0; page <= 8; page++) {
			// set page
			LCD_instruction(controller, (0xb8 + page), 0);
			// set address to zero
			LCD_instruction(controller, 0x40, 0);
			for(address = 0; address < 64; address++)
				// write zeros (no need to specify address as the controller automatically increases it
				LCD_instruction(controller, 0x00, 1);
		}
	}
}

void LCD_clear_range(uint8_t lpage, uint8_t hpage, uint8_t laddress, uint8_t haddress) {

	uint8_t page, address, controller = 0;

	for(page = lpage; page <= hpage; page++) {

		if(laddress < 64)
			controller = 0;
		else
			controller = 1;

		// set page
		LCD_instruction(controller, (0xb8 + page), 0);
		// set address
		LCD_instruction(controller, 0x40 + (laddress % 64), 0);
		for(address = laddress; address < haddress; address++) {
		
			if(address == 64) {
				controller = 1;
				LCD_instruction(controller, (0xb8 + page), 0);		// set page
				LCD_instruction(controller, 0x40, 0);				// set address start from zero
			}
			// write zeros (no need to specify address as the controller automatically increases it
			LCD_instruction(controller, 0x00, 1);
		}
	}
}

uint8_t LCD_writechar_8(uint8_t x, uint8_t page, uint8_t character) {

	uint8_t i, controller = 0;

	if(x < 64)
		controller = 0;
	else
		controller = 1;

	LCD_instruction(controller, (0xb8 + page), 0);					// set page (0-7)

	/*
		set address
		If x > 64 take remainder and use that as the address.
		If drawing is crosses to another controller address is zeroed (see below)
	*/
	LCD_instruction(controller, 0x40 + (x % (controller * 64)), 0); // set address (0-63)

	for(i=0; i<7; i++) {
		if(pgm_read_byte(&(font8[character][i])) != 0x00) {			// skip empty lines

			if(x == 64) { 											// crossing the line from controller to another
				controller = 1;
				LCD_instruction(controller, (0xb8 + page), 0);		// set page
				LCD_instruction(controller, 0x40, 0);				// set address start from zero
			}

			if(x < 128) {											// check if we are still on screen
					LCD_instruction(controller, pgm_read_byte(&(font8[character][i])), 1);
					x++;
			}
		}
	}
	// add space between letters
	LCD_instruction(controller, 0x00, 1);
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

		LCD_instruction(controller, (0xb8 + page + i), 0);
		LCD_instruction(controller, 0x40 + (x % (controller * 64)), 0);

		for(j=0; j<9; j++) {
			if((pgm_read_byte(&(font16[character][0][j])) | pgm_read_byte(&(font16[character][1][j]))) != 0x00) {

				if(x == 64) { 											// crossing the line from controller to another
					controller = 1;
					LCD_instruction(controller, (0xb8 + page + i), 0);		// set page
					LCD_instruction(controller, 0x40, 0);					// set address start from zero
				}

				if(x < 128) {											// check if we are still on screen
					LCD_instruction(controller, pgm_read_byte(&(font16[character][i][j])), 1);
					x++;
				}
			}
		}
	}

	
	if(x < 128) {
		for(i=0; i<2; i++) {
			LCD_instruction(controller, (0xb8 + page + i), 0);
			LCD_instruction(controller, 0x40 + (x % (controller * 64)), 0);
			LCD_instruction(controller, 0x00, 1); 							// space between letters
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

	LCD_clear_range(0, 5, 0, 25);

	uint8_t i, line = 0;

	for(line=0; line<5; line++) {

		LCD_instruction(0, (0xb8 + line), 0);			// set page
		LCD_instruction(0, 0x40, 0);					// set address (zero)

		for(i=0; i<25; i++)
			LCD_instruction(0, pgm_read_byte(&(font40[gear][line][i])), 1);
	}
}
