#include <avr/io.h>
#include "testi.h"

void LCDdrawGraticule(uint8_t controller) {

	uint8_t page = 0;

	

	for(page = 0; page < 8; page++) {
	
		// set page
		LCD_instruction(controller, (0xb8 + page), 0);

		switch(page) {
			case 0:
				LCD_instruction(controller, (0x40 + 30), 0);
				LCD_instruction(controller, 0x08, 1);
				LCD_instruction(controller, 0x04, 1);
				LCD_instruction(controller, 0xfe, 1);
				LCD_instruction(controller, 0x04, 1);
				LCD_instruction(controller, 0x08, 1);
				break;
			case 1:
				LCD_instruction(controller, (0x40 + 15), 0);
				LCD_instruction(controller, 0x80, 1);
				LCD_instruction(controller, 0x40, 1);
				LCD_instruction(controller, 0x20, 1);
				LCD_instruction(controller, 0x30, 1);
				LCD_instruction(controller, 0x10, 1);
				LCD_instruction(controller, 0x08, 1);
				LCD_instruction(controller, 0x08, 1);
				LCD_instruction(controller, 0x04, 1);
				LCD_instruction(controller, 0x04, 1);
				LCD_instruction(controller, 0x02, 1);
				LCD_instruction(controller, 0x02, 1);
				LCD_instruction(controller, 0x02, 1);
				LCD_instruction(controller, 0x02, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0xff, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x02, 1);
				LCD_instruction(controller, 0x02, 1);
				LCD_instruction(controller, 0x02, 1);
				LCD_instruction(controller, 0x02, 1);
				LCD_instruction(controller, 0x04, 1);
				LCD_instruction(controller, 0x04, 1);
				LCD_instruction(controller, 0x08, 1);
				LCD_instruction(controller, 0x08, 1);
				LCD_instruction(controller, 0x10, 1);
				LCD_instruction(controller, 0x30, 1);
				LCD_instruction(controller, 0x20, 1);
				LCD_instruction(controller, 0x40, 1);
				LCD_instruction(controller, 0x80, 1);
				break;
			case 2:
				LCD_instruction(controller, (0x40 + 10), 0);
				LCD_instruction(controller, 0xe0, 1);
				LCD_instruction(controller, 0x30, 1);
				LCD_instruction(controller, 0x0c, 1);
				LCD_instruction(controller, 0x02, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, (0x40 + 24), 0);
				LCD_instruction(controller, 0x80, 1);
				LCD_instruction(controller, 0xc0, 1);
				LCD_instruction(controller, 0x40, 1);
				LCD_instruction(controller, 0x20, 1);
				LCD_instruction(controller, 0x20, 1);
				LCD_instruction(controller, 0x10, 1);
				LCD_instruction(controller, 0x10, 1);
				LCD_instruction(controller, 0x10, 1);
				LCD_instruction(controller, 0xff, 1);
				LCD_instruction(controller, 0x10, 1);
				LCD_instruction(controller, 0x10, 1);
				LCD_instruction(controller, 0x10, 1);
				LCD_instruction(controller, 0x20, 1);
				LCD_instruction(controller, 0x20, 1);
				LCD_instruction(controller, 0x40, 1);
				LCD_instruction(controller, 0xc0, 1);
				LCD_instruction(controller, 0x80, 1);
				LCD_instruction(controller, (0x40 + 50), 0);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x02, 1);
				LCD_instruction(controller, 0x0c, 1);
				LCD_instruction(controller, 0x30, 1);
				LCD_instruction(controller, 0xe0, 1);
				break;
			case 3:
				LCD_instruction(controller, (0x40 + 8), 0);
				LCD_instruction(controller, 0xf8, 1);
				LCD_instruction(controller, 0x07, 1);
				LCD_instruction(controller, (0x40 + 21), 0);
				LCD_instruction(controller, 0xe0, 1);
				LCD_instruction(controller, 0x1c, 1);
				LCD_instruction(controller, 0x02, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, (0x40 + 32), 0);
				LCD_instruction(controller, 0xff, 1);
				LCD_instruction(controller, (0x40 + 40), 0);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x02, 1);
				LCD_instruction(controller, 0x1c, 1);
				LCD_instruction(controller, 0xe0, 1);
				LCD_instruction(controller, (0x40 + 55), 0);
				LCD_instruction(controller, 0x07, 1);
				LCD_instruction(controller, 0xf8, 1);
				LCD_instruction(controller, (0x40 + 60), 0);
				LCD_instruction(controller, 0x40, 1);
				LCD_instruction(controller, 0x80, 1);
				break;
			case 4:
				LCD_instruction(controller, (0x40 + 3), 0);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x1f, 1);
				LCD_instruction(controller, 0xe1, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x07, 1);
				LCD_instruction(controller, 0x39, 1);
				LCD_instruction(controller, 0x41, 1);
				LCD_instruction(controller, 0x81, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0xff, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x81, 1);
				LCD_instruction(controller, 0x41, 1);
				LCD_instruction(controller, 0x39, 1);
				LCD_instruction(controller, 0x07, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0xe1, 1);
				LCD_instruction(controller, 0x1f, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x05, 1);
				LCD_instruction(controller, 0x03, 1);
				LCD_instruction(controller, 0x01, 1);
				break;
			case 5:
				LCD_instruction(controller, (0x40 + 10), 0);
				LCD_instruction(controller, 0x07, 1);
				LCD_instruction(controller, 0x0c, 1);
				LCD_instruction(controller, 0x30, 1);
				LCD_instruction(controller, 0x40, 1);
				LCD_instruction(controller, 0x80, 1);
				LCD_instruction(controller, (0x40 + 24), 0);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, 0x03, 1);
				LCD_instruction(controller, 0x02, 1);
				LCD_instruction(controller, 0x04, 1);
				LCD_instruction(controller, 0x04, 1);
				LCD_instruction(controller, 0x08, 1);
				LCD_instruction(controller, 0x08, 1);
				LCD_instruction(controller, 0x08, 1);
				LCD_instruction(controller, 0xff, 1);
				LCD_instruction(controller, 0x08, 1);
				LCD_instruction(controller, 0x08, 1);
				LCD_instruction(controller, 0x08, 1);
				LCD_instruction(controller, 0x04, 1);
				LCD_instruction(controller, 0x04, 1);
				LCD_instruction(controller, 0x02, 1);
				LCD_instruction(controller, 0x03, 1);
				LCD_instruction(controller, 0x01, 1);
				LCD_instruction(controller, (0x40 + 50), 0);
				LCD_instruction(controller, 0x80, 1);
				LCD_instruction(controller, 0x40, 1);
				LCD_instruction(controller, 0x30, 1);
				LCD_instruction(controller, 0x0c, 1);
				LCD_instruction(controller, 0x07, 1);
				break;
		}

	}

}
