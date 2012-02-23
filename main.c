#include <avr/io.h>
#include "lcd.h"
#include "spi.h"
#include "mcp23s17.h"
#include "mcp3204.h"
#include "timer.h"

/*
#define BAUD 57600
#include <util/setbaud.h>

void USART_Transmit( unsigned char data ) {
	while (!( UCSRA & (1<<UDRE)));
	UDR = data;
}
*/

volatile uint8_t outputwidth = 0;
volatile uint16_t speed = 0;
volatile uint8_t speed_width = 0;
volatile uint8_t rpm_width = 0;
volatile uint16_t rpm = 0;
volatile uint8_t rpmleds = 0;
volatile uint8_t gear = 1;

int main() {

/*
	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
	#if USE_2X
	UCSRA |= (1 << U2X);
	#else
	UCSRA &= ~(1 << U2X);
	#endif
	UCSRB = (1<<RXEN)|(1<<TXEN);
	//USART_Transmit(0x01);
*/

	SPI_MasterInit();
	
	// LCD
	LCDinit();

	// GPIO
	GPIOInit();

	// ADC
	ADCinit();

	// Timer
	TimerInit();
	TimerSetFreq(22050);

	while(1)
		asm volatile ("nop");
	
}
