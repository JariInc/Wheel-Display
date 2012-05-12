#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "lcd.h"
#include "timer.h"

/*
int main() {

	// UART
	UBRR = 8; // 76800
	UCR = (1 << RXCIE) | (1 << RXEN) | (1 << TXEN);

	// LCD
	LCDinit();
	
	// Timer
	//TimerInit(TIMER0);
	TimerInit(TIMER1);
	TimerSetFreq(TIMER1, 10);

	// Watchdog
	wdt_enable(WDTO_250MS);

	sei();
	while(1) {
		wdt_reset();
	}
}
*/
