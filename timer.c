#include <avr/io.h>
#include <avr/interrupt.h>
#include "lcd.h"

#define ONEHZ (F_CPU >> 8)

volatile uint8_t ledDiv = 0;
volatile uint16_t lcdDiv = 0;

void TimerInit(void) {
	// LCD PWM control as output
	DDRC |= (1 << PC6);

    // set up timer with prescaler and CTC mode
    TCCR1B |= (1 << WGM12)|(1 << CS12)|(0 << CS11)|(0 << CS10);

    // initialize counter
    TCNT1 = 0;

    // initialize compare value
    OCR1A = 0xffff;

    // enable compare interrupt
    TIMSK |= (1 << OCIE1A);

    // enable global interrupts
    sei();
}

void TimerSet(uint16_t val) {
	OCR1A = val;
}

void TimerSetFreq(uint16_t freq) {
	OCR1A = ONEHZ/freq;
}

ISR(TIMER1_COMPA_vect) {
	// LCD backlight PWM
	// divide timer by 64
	// 22050Hz/63/2 = 175Hz
	if(ledDiv++ > (1 << 6)) {
    	PORTC ^= (1 << PC6);
		ledDiv = 0;
	}

	// LCD content update
	// divide time by 256
	if(lcdDiv++ > (1 << 10)) {
    	LCDloop();
		lcdDiv = 0;
	}
}
