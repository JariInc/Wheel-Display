#include <avr/io.h>
#include <avr/interrupt.h>

void PWMinit(void) {
	DDRC |= (1 << PC6);

    // set up timer with prescaler = 64 and CTC mode
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

void PWMset(uint16_t val) {
	OCR1A = val;
}

void PWMsetFreq(uint8_t freq) {
	OCR1A = 19532/freq;
}

ISR(TIMER1_COMPA_vect) {
    // toggle led here
    PORTC ^= (1 << PC6);
}
