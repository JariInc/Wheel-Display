#include <avr/io.h>
#include "types.h"
#include "uart.h"
#include "uartlcd.h"

extern volatile uint16_t data[32];
extern volatile uint8_t lcdtypes[7];
extern volatile uint8_t lcdupdate[32];
volatile uint8_t lcdReady = 0;
volatile uint8_t round = 0;

void LCDUpdateValue(uint8_t pos, uint8_t type, uint16_t value) {
	uartTx((pos << 5)|type);
	uartTx(value >> 8);
	uartTx(value & 0xff);
	//lcdReady = 1;
	
	uartTx(0xff);
	uartTx(0xff);
	uartTx(0xff);
}

void LCDUpdate(void) {
	uint8_t i;

	if((UCSR1A & (1<<RXC1)))
		lcdReady |= uartRx();

	if(lcdReady) {
		for(i = 0; i < sizeof(lcdtypes); i++) {
			if(lcdupdate[lcdtypes[i]]) {
				LCDUpdateValue(i, lcdtypes[i], data[lcdtypes[i]]);
				lcdupdate[lcdtypes[i]] = 0;
				lcdReady = 0;
				break;
			}
		}
	}
	else {
		uartTx(0xff);
		uartTx(0xff);
		uartTx(0xff);
	}
}

