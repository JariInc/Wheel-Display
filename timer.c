#include <avr/io.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "timer.h"
#include "max3420.h"

#define ONEHZ_T0 (F_CPU >> 10)
#define ONEHZ_T1 (F_CPU >> 8)

extern volatile uint8_t usbready;
extern volatile char Suspended;

//volatile uint8_t ledDiv = 0;
//volatile uint16_t lcdDiv = 0;

void TimerInit(void) {
	// LCD PWM control as output
	//DDRC |= (1 << PC6);

	// Timer 0: LCD content
    // set up timer with prescaler
    TCCR0 |= (1 << CS12)|(0 << CS11)|(1 << CS10); // prescaling 1024
    // initialize counter
    TCNT0 = 0;
    // initialize compare value
    OCR0 = 0xff;
    // enable compare interrupt
    TIMSK |= (1 << OCIE0);

	// Timer 1: ADC & LCD backlight
    // set up timer with prescaler and CTC mode
    TCCR1B |= (1 << WGM12)|(1 << CS12)|(0 << CS11)|(0 << CS10); // prescaling 256
    // initialize counter
    TCNT1 = 0;
    // initialize compare value
    OCR1A = 0xffff;
    // enable compare interrupt
    TIMSK |= (1 << OCIE1A);

    // enable global interrupts
    sei();
}

void TimerSet(uint8_t timer, uint16_t val) {
	switch(timer) {
		case 0:
			OCR0 = val;
			break;
		case 1:
			OCR1A = val;
			break;
	}
}

void TimerSetFreq(uint8_t timer, uint16_t freq) {
	switch(timer) {
		case 0:
			OCR0 = ONEHZ_T0/freq;
			break;
		case 1:
			OCR1A = ONEHZ_T1/freq;
			break;
	}
	
}

// Timer 0
ISR(TIMER0_COMP_vect) {
	// LCD content update 45Hz
    LCDloop(); 

	/*
	// Button states
	if(!btns.interrupt) {
		btns.state = GPIORead(0x00);
		if(btns.state != btns.prevstate)
			btns.interrupt = 0x01;
	}
	*/
	
	if(usbready) {
		if(Suspended)
			USBChkResume();
		//else if((PORTD & (1 << 2)) == 0x00)
		//	USBServeIRQ();
		//USART_Transmit(USBRead(0x0b));
		//USART_Transmit(USBRead(0x0c));
	}
}

// Timer 1
ISR(TIMER1_COMPA_vect) {
	// LCD backlight PWM
	// divide timer by 64
	// 22050Hz/63/2 = 175Hz
	//if(ledDiv++ > 32) {
    //	PORTC ^= (1 << PC6);
	//	ledDiv = 0;
	//}
	
}
