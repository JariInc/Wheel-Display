#include "spi.h"

void ADCinit() {
	// just in case, push CS low for a while
	SPI_SelectSlave(SPIADC);
	SPI_UnselectAllSlaves();
}

uint16_t ADCGetValue(uint8_t ch) {
	uint16_t output = 0;
	SPI_SelectSlave(SPIADC);
	SPI_MasterTransmit(0b00000110);
	output = SPI_MasterTransmit(ch << 6) << 8;
	output |= SPI_MasterTransmit(0xff);
	SPI_UnselectAllSlaves();
	return output & 0xfff;
}
