#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <util/delay.h> 

#include "spi.h"

void SPI_MasterInit(void) {
	/* Set MOSI, CS and SCK output, MISO input */
	DDRB |= (1<<PB1)|(1<<PB2)|(1<<PB0);
	DDRB &= ~(1<<PB3);
	
	SPCR |= (1<<MSTR)|(1<<SPE)|(0<<SPR1)|(1<<SPR0);
	SPSR |= (0<<SPI2X);

	char dump = SPSR; // clear SPIF bit in SPSR
    dump = SPDR;

	/* Set all slave selects output and high */
	SPIGPIO_DDR |= (1<<SPIGPIO_PIN);
	SPIADC_DDR |= (1<<SPIADC_PIN);
	SPI_UnselectAllSlaves();
}

char SPI_MasterTransmit(char cData){ 
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
	    SPDR  = cData;          		// send Character
	    while (!(SPSR & (1<<SPIF)));   // wait until Char is sent
	}
	return SPDR;
}


void SPI_UnselectAllSlaves(void) {
	SPIGPIO_PORT |= (1<<SPIGPIO_PIN);
	SPIADC_PORT |= (1<<SPIADC_PIN);
}

void SPI_SelectSlave(uint8_t slave) {
	// clear all
	SPI_UnselectAllSlaves();
	// set one
	switch(slave) {
		case SPIGPIO:
			SPIGPIO_PORT &= ~(1<<SPIGPIO_PIN);
			break;
		case SPIADC:
			SPIADC_PORT &= ~(1<<SPIADC_PIN);
			break;
		default:
			break;
	}
}
