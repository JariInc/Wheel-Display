#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#include "liblcd.h"
#include "spi.h"
#include "mcp23s17.h"
#include "mcp3204.h"
#include "pwm.h"

#define BAUD 57600
#include <util/setbaud.h>

void USART_Transmit( unsigned char data ) {
	while (!( UCSRA & (1<<UDRE)));
	UDR = data;
}

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

	uint8_t i;
	uint8_t outputwidth = 0;
	uint8_t speed_width = 0;
	uint8_t rpm_width = 0;
	uint16_t rpm = 0;
	uint8_t rpmleds = 0;
	uint8_t gear = 1;

	uint16_t inc = 0;
	uint16_t j;
	
	// LCD
	LCD_initialize();

	// 1st line
	LCD_writestring(28, 0, "rpm", 3, 0);
	LCD_writestring(80, 0, "speed", 5, 0);
	// 2nd line
	LCD_writestring(28, 3, "fuel", 4, 0);
	LCD_writestring(80, 3, "lap", 3, 0);

	// 3rd line
	LCD_writestring(0, 6, "laptime", 7, 0);

	// data
	//LCD_writeint(80, 1, 107, 3, 1); // speed
	LCD_writeint(28, 4, 90, 4, 1); // fuel
	LCD_writeint(80, 4, 42, 4, 1); // lap

	i=0;
	LCD_writegear(gear);
	LCD_writegear(8);
	//LCDdrawGraticule(0);

	// GPIO
	GPIOInit();

	// ADC
	ADCinit();

	// PWM
	PWMinit();
	//PWMset(19532);
	PWMsetFreq(200);

	while(1) {
		_delay_ms(16);

		inc = ADCGetValue(0);
		
		//USART_Transmit((char)ADCGetValue(0));
		//GPIOWrite(0x00, (char)ADCGetValue(0));

		rpmleds = 0xff >> (7 - (rpm >> 10));
		GPIOWrite(0x00, rpmleds);

		// speed
		outputwidth = LCD_writeint(80, 1, i, 3, 1);
		if(speed_width > outputwidth) {
			LCD_clear_range(1, 2, outputwidth, speed_width);
		}
		speed_width = outputwidth;

		// gear
		if(rpm > (1 << 13)) {
			LCD_writegear(gear & 7);
			rpm = 3000;
		}
		
		// rpm
		outputwidth = LCD_writeint(28, 1, rpm, 5, 1);
		// clear the remainder
		if(rpm_width > outputwidth) {
			LCD_clear_range(1, 2, outputwidth, rpm_width);
		}

		rpm_width = outputwidth;
		rpm += (inc >> 4);

		LCD_clear_range(7, 7, 0, 4*6);
		LCD_writeint(0, 7, inc, 4, 0);
		
		i++;
	}
	
}
