#include <avr/io.h>
#include <avr/interrupt.h>
#include "spilcd.h"
#include "timer.h"

#define ONEHZ_T0 (F_CPU >> 8)
#define ONEHZ_T1 (F_CPU >> 10)

extern volatile uint8_t usbconnected;

void TimerInit(uint8_t timer) {
	switch(timer) {
		case 0:
			// Timer 0
		    // set up timer with prescaler
		    TCCR0B |= (1 << CS02)|(0 << CS01)|(0 << CS00); // prescaling 256
		    // initialize counter
		    TCNT0 = 0;
		    // initialize compare value
		    OCR0A = 0xff;
		    // enable compare interrupt
		    TIMSK0 |= (1 << OCIE0A);
			break;
		case 1:
			// Timer 1
		    // set up timer with prescaler and CTC mode
		    TCCR1B |= (1 << WGM12)|(1 << CS12)|(0 << CS11)|(1 << CS10); // prescaling 1024
		    // initialize counter
		    TCNT1 = 0;
		    // initialize compare value
		    OCR1A = 0xffff;
		    // enable compare interrupt
		    TIMSK1 |= (1 << OCIE1A);
			break;
	}

    // enable global interrupts
    sei();
}

void TimerSet(uint8_t timer, uint16_t val) {
	switch(timer) {
		case 0:
			OCR0A = val;
			break;
		case 1:
			OCR1A = val;
			break;
	}
}

void TimerSetFreq(uint8_t timer, uint16_t freq) {
	switch(timer) {
		case 0:
			OCR0A = ONEHZ_T0/freq;
			break;
		case 1:
			OCR1A = ONEHZ_T1/freq;
			break;
	}
	
}



// Timer 1

ISR(TIMER1_COMPA_vect) {
	LCDUpdate();
	/*
	// Button states
	if(!btns.interrupt) {
		btns.state = GPIORead(0x00);
		if(btns.state != btns.prevstate)
			btns.interrupt = 0x01;
	}
	*/
}
