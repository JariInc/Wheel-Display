#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>
#include "ks0108.h"

void ks0108_controller_disable(uint8_t i) {
	switch(i) {
		case 0:
			KS0108_CTRL |= CS1;
			break;
		case 1:	
			KS0108_CTRL |= CS2;
			break;
	}

}

void ks0108_controller_enable(uint8_t i) {
	switch(i) {
		case 0:
			KS0108_CTRL &= ~CS1;
			break;
		case 1:	
			KS0108_CTRL &= ~CS2;
			break;
	}
}

uint8_t ks0108_readdata(uint8_t controller, uint8_t status) {
	uint8_t data = 0x00;

	ks0108_controller_enable(controller);

	KS0108_DATA = 0x00;
	KS0108_DATA_DIR = 0x00;
	KS0108_CTRL |= RW;

	if(status != 0) { // read LCD status
		KS0108_CTRL &= ~RS;	
		KS0108_CTRL |= EN;
		asm volatile ("nop");
		data = KS0108_DATA;
		KS0108_CTRL &= ~EN;
	}
	else {			// read data RAM
		KS0108_CTRL |= RS;

		// dummy read
		KS0108_CTRL |= EN;
		asm volatile ("nop");
		KS0108_CTRL &= ~EN;
		
		// wait data fetch
		//while(ks0108_readdata(controller, 1) & 0x80);

		// actual read
		KS0108_CTRL |= EN;
		asm volatile ("nop");
		KS0108_CTRL &= ~EN;

		// wait data fetch
		while(ks0108_readdata(controller, 1) & 0x80);

		data = KS0108_DATA;
	}

	ks0108_controller_disable(controller);

	return data;
}

void ks0108_instruction(uint8_t controller, uint8_t instruction, uint8_t data) {
	// wait until pin7 goes low
	while(ks0108_readdata(controller, 1) & 0x80);

	ks0108_controller_enable(controller);

	KS0108_DATA_DIR = 0xff;
	KS0108_CTRL &= ~RW;

	if(data != 0)	// write data RAM
		KS0108_CTRL |= RS;
	else			// write LCD instruction
		KS0108_CTRL &= ~RS;

	KS0108_DATA = instruction;

	KS0108_CTRL |= EN;
	asm volatile ("nop");
	KS0108_CTRL &= ~EN;

	ks0108_controller_disable(controller);
}

void ks0108_init(void) {

	KS0108_CTRL_DIR |= (RS | RW | CS1 | CS2 | EN);	// ctrl direction (out)

	KS0108_CTRL |= (CS1 | CS2);	// disable both controllers (logic high)

	ks0108_clear();

}

void ks0108_clear(void) {

	uint8_t controller = 0;
	uint8_t page = 0;
	uint8_t address = 0;

	for(controller = 0; controller < 2; controller++) {

		ks0108_instruction(controller, 0x3f, 0); // turn display on
		ks0108_instruction(controller, 0xc0, 0); // set start address

		for(page = 0; page <= 8; page++) {
			// set page
			ks0108_instruction(controller, (0xb8 + page), 0);
			// set address to zero
			ks0108_instruction(controller, 0x40, 0);
			for(address = 0; address < 64; address++)
				// write zeros (no need to specify address as the controller automatically increases it
				ks0108_instruction(controller, 0x00, 1);
		}
	}
}

void ks0108_clear_range(uint8_t lpage, uint8_t hpage, uint8_t laddress, uint8_t haddress) {

	uint8_t page, address, controller = 0;

	for(page = lpage; page <= hpage; page++) {

		if(laddress < 64)
			controller = 0;
		else
			controller = 1;

		// set page
		ks0108_instruction(controller, (0xb8 + page), 0);
		// set address
		ks0108_instruction(controller, 0x40 + (laddress % 64), 0);
		for(address = laddress; address < haddress; address++) {
		
			if(address == 64) {
				controller = 1;
				ks0108_instruction(controller, (0xb8 + page), 0);		// set page
				ks0108_instruction(controller, 0x40, 0);				// set address start from zero
			}
			// write zeros (no need to specify address as the controller automatically increases it
			ks0108_instruction(controller, 0x00, 1);
		}
	}
}


