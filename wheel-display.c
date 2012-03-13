#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "lcd.h"
#include "spi.h"
#include "mcp23s17.h"
#include "mcp3204.h"
#include "timer.h"
#include "AudioInput.h"

/*
#define BAUD 38400
#include <util/setbaud.h>

void USART_Transmit(unsigned char data) {
	while (!( UCSR1A & (1<<UDRE1)));
	UDR1 = data;
}

void USART_Transmit_Int(uint8_t integer) {
	char data[7];
	sprintf(data, "%i", integer);
	USART_Transmit_Str(data);
}

void USART_Transmit_Str(const char *USART_String) { 
	uint8_t c; 
	while ((c=*USART_String++)) 
		USART_Transmit(c); 
	USART_Transmit(' '); 
}
*/

volatile LCDtext speed;
volatile LCDtext rpm;
volatile uint8_t rpmleds;
volatile LCDtext gear;
volatile buttons btns;

int main() {

	/*
	UBRR1H = UBRRH_VALUE;
	UBRR1L = UBRRL_VALUE;
	UCSR1B = (1<<RXEN1)|(1<<TXEN1);
	*/

	// USB
	SetupHardware();

	// LCD
	LCDinit();

	// SPI
	SPI_MasterInit();

	// ADC
	ADCinit();

	// GPIO
	GPIOInit();
	
	// Timer
	//TimerInit(TIMER1);
	//TimerSetFreq(TIMER0, 48000);
	//TimerSetFreq(TIMER1, 15);
	//TimerSet(TIMER1, 0xffff);
	
	//DDRB |= (1 << PB6);

	USBloop();
}
