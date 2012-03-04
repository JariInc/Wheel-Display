#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "spi.h"
// ATMega16 datasheet, page 138 

void SPI_MasterInit(void) {
	/* Set MOSI and SCK output, all others input */
	DDRB |= (1<<PB5)|(1<<PB7);
	DDRB &= ~(1<<PB6);

	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (0<<SPIE)|(1<<SPE)|(0<<DORD)|(1<<MSTR)|(0<<CPOL)|(0<<CPHA)|(0<<SPR1)|(0<<SPR0);
	SPSR |= (1<<SPI2X);

	char dump = SPSR; // clear SPIF bit in SPSR
    dump = SPDR;

	/* Set all slave selects HIGH */
	SPIUSB_DDR |= (1<<SPIUSB_PIN);
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

void SPI_UnselectAllSlaves() {
	SPIUSB_PORT |= (1<<SPIUSB_PIN);
	SPIGPIO_PORT |= (1<<SPIGPIO_PIN);
	SPIADC_PORT |= (1<<SPIADC_PIN);
}

void SPI_SelectSlave(uint8_t slave) {
	// clear all
	SPI_UnselectAllSlaves();
	// set one
	switch(slave) {
		case SPIUSB:
			SPIUSB_PORT &= ~(1<<SPIUSB_PIN);
			break;
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
