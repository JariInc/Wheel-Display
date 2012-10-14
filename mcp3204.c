#include <util/delay.h>
#include "spi.h"
#include "mcp3204.h"

extern volatile int16_t noisefloor[2];

void ADCinit(void) {
	// just in case, push CS low for a while
	SPI_SelectSlave(SPIADC);
	SPI_UnselectAllSlaves();

	// get noise floor
	uint8_t i;
	for(i = 0; i < (1 << 4); i++) {
		noisefloor[0] += ADCGetValue(0);
		noisefloor[1] += ADCGetValue(1);
		_delay_us(125);
	}

	// average samples
	noisefloor[0] >>= 4;
	noisefloor[1] >>= 4;
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
