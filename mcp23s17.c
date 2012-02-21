#include <avr/io.h>
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

/*
char GPIOReadReg(unsigned char addr) {
	char input;
	SPI_SelectSlave(SPIGPIO);
	SPI_MasterTransmit(0x41);	// chip select, read
	input = SPI_MasterTransmit(addr);	// addr
	SPI_UnselectAllSlaves();
	return input;
}
*/

void GPIOInit() {
	SPI_SelectSlave(SPIGPIO);
	SPI_MasterTransmit(0x40); // chip select, write
	SPI_MasterTransmit(0x00); // addr: 0x00
	SPI_MasterTransmit(0x00); // IODIRA = 0x00 (out)
	SPI_MasterTransmit(0x00); // IODIRB = 0x00
	SPI_UnselectAllSlaves();
}
