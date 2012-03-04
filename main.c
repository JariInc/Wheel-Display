#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/pgmspace.h>   /* required by usbdrv.h */

#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */
#include "requests.h"       /* The custom request numbers we use */
#include "lcd.h"
#include "spi.h"
#include "mcp23s17.h"
#include "mcp3204.h"
#include "max3420.h"
#include "timer.h"

#define BAUD 57600
#include <util/setbaud.h>

void USART_Transmit(unsigned char data) {
	while (!( UCSRA & (1<<UDRE)));
	UDR = data;
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


volatile LCDtext speed;
volatile LCDtext rpm;
volatile uint8_t rpmleds;
volatile LCDtext gear;
volatile buttons btns;
volatile uint8_t usbready;

int main() {

	usbready = 0;

	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
	#if USE_2X
	UCSRA |= (1 << U2X);
	#else
	UCSRA &= ~(1 << U2X);
	#endif
	UCSRB = (1<<RXEN)|(1<<TXEN);

	// SPI
	SPI_MasterInit();
	
	// LCD
	LCDinit();

	// GPIO
	GPIOInit();

	// ADC
	ADCinit();

	// USB
	USBInit();

	// Timer
	TimerInit();
	TimerSet(TIMER0, 0xff);
	TimerSetFreq(TIMER1, 22500);

	char regnames[21][10] = {
		"EP0FIFO   ",
		"EP1OUTFIFO",
		"EP2INFIFO ",
		"EP3INFIFO ",
		"SUDFIFO   ",
		"EP0BC     ",
		"EP1OUTBC  ",
		"EP2INBC   ",
		"EP3INBC   ",
		"EPSTALLS  ",
		"CLRTOGS   ",
		"EPIRQ     ",
		"EPIEN     ",
		"USBIRQ    ",
		"USBIEN    ",
		"USBCTL    ",
		"CPUCTL    ",
		"PINCTL    ",
		"REVISION  ",
		"FNADDR    ",
		"IOPINS    "
	};

	int i, j;
	char val[5];

/*
	#include "usbdescriptors.h"
	for(i=0; i < DD[0]; i++) {
		USART_Transmit(DD[i]);
	}
*/
	while(1) {
	/*
		_delay_ms(1000);
		
		for(i = 0; i < 21; i++) {
			for(j = 0; j < 10; j++) {
				USART_Transmit(regnames[i][j]);
			}
			sprintf(val,"0x%02x",USBRead(i));
			USART_Transmit(' ');
			USART_Transmit(val[0]);
			USART_Transmit(val[1]);
			USART_Transmit(val[2]);
			USART_Transmit(val[3]);
			USART_Transmit(val[4]);
			USART_Transmit(0x0a);
		}
	*/
	}
}
