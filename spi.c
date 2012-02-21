#include "spi.h"

volatile unsigned char spibuffer[2][SPI_BUFFER_SIZE];
volatile short int spirecv[2] = {0, 0};

// ATMega16 datasheet, page 138 

void SPI_MasterInit(void) {
	/* Set MOSI and SCK output, all others input */
	DDRB |= (1<<PB5)|(1<<PB7);
	DDRB &= ~(1<<PB6);

	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (0<<SPIE)|(1<<SPE)|(0<<DORD)|(1<<MSTR)|(0<<CPOL)|(0<<CPHA)|(0<<SPR1)|(1<<SPR0);
	SPSR |= (0<<SPI2X);

	char dump = SPSR; // clear SPIF bit in SPSR
    dump = SPDR;

	/* Set all slave selects HIGH */
	SPIUSB_DDR |= (1<<SPIUSB_PIN);
	SPIGPIO_DDR |= (1<<SPIGPIO_PIN);
	SPIADC_DDR |= (1<<SPIADC_PIN);
	SPI_UnselectAllSlaves();
}

/*
void SPI_MasterTransmit(char cData) {
	// Start transmission
	//SPDR = cData;
	// Wait for transmission complete
	//while(!(SPSR & (1<<SPIF)));
	//return SPDR;
	spibuffer[OUT][spirecv[OUT]++] = cData;
}
*/

char SPI_MasterTransmit(char cData){ 
    SPDR  = cData;          		// send Character
    while (!(SPSR & (1<<SPIF)));   // wait until Char is sent
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

/*
	Interrupt handling
*/


ISR(SPI_STC_vect) {
	// data in
	spibuffer[IN][spirecv[IN]++] = SPDR;
	if (spirecv[IN] >= SPI_BUFFER_SIZE || spibuffer[IN][spirecv[IN]-1] == 0x00) {
		// process data
		spirecv[IN] = 0;
	}
	// data out
	if(spirecv[OUT] > 0) {
		SPDR = spibuffer[OUT][0];
		spirecv[OUT]--;
	}
}
