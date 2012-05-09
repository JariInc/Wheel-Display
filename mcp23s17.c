#include <avr/io.h>
#include <util/delay.h>
#include "mcp23s17.h"
#include "spi.h"

void GPIOWrite(unsigned char a, unsigned char b) {
	SPI_SelectSlave(SPIGPIO);
	SPI_MasterTransmit(0x40);	// chip select, write
	SPI_MasterTransmit(0x12);	// addr
	SPI_MasterTransmit(a);		// GPIOA
	SPI_MasterTransmit(b); 		// GPIOB
	SPI_UnselectAllSlaves();
}


char GPIORead(uint8_t bank) { // 0=bank a, 1=bank b
	char input;
	SPI_SelectSlave(SPIGPIO);
	SPI_MasterTransmit(0x41);	// chip select, read
	SPI_MasterTransmit(0x12 + (bank & 0x01)); // addr
	input = SPI_MasterTransmit(0x00);
	SPI_UnselectAllSlaves();
	return input;
}


void GPIOInit(void) {
	GPIOSetDir(0xff, 0x00);
}

void GPIOSetDir(char banka, char bankb) { // 0 = output, 1 = input
	SPI_SelectSlave(SPIGPIO);
	SPI_MasterTransmit(0x40); // chip select, write
	SPI_MasterTransmit(0x00); // addr: 0x00
	SPI_MasterTransmit(banka); // IODIRA = 0x00 (out)
	SPI_MasterTransmit(bankb); // IODIRB = 0x00
	SPI_MasterTransmit(0xff); // IPOLA = 0xff (reverse polarity)
	SPI_MasterTransmit(0xff); // IPOLB = 0xff
	SPI_UnselectAllSlaves();

	SPI_SelectSlave(SPIGPIO);
	SPI_MasterTransmit(0x40); // chip select, write
	SPI_MasterTransmit(0x0c); // addr: 0x0c
	SPI_MasterTransmit(0xff); // GPPUA: 0 = no pu, 1 = 100k pullup
	SPI_MasterTransmit(0xff); // GPPUB 
	SPI_UnselectAllSlaves();

}
