#include "spi.h"

#define TYPE_RPM 			0x00
#define TYPE_SPEED 			0x01
#define TYPE_FUEL 			0x02
#define TYPE_FUEL_NEED 		0x03
#define TYPE_LAP 			0x04
#define TYPE_LAPS_REM 		0x05
#define TYPE_POS 			0x06
#define TYPE_LAPTIME 		0x10
#define TYPE_DELTA 			0x11

uint16_t debugges = 0;

void LCDUpdateValue(uint8_t pos, uint8_t type, uint16_t value) {
	SPI_SelectSlave(SPILCD);
	SPI_MasterTransmit((pos << 5)|type);
	//SPI_UnselectAllSlaves();
	//SPI_SelectSlave(SPILCD);
	SPI_MasterTransmit(value >> 8);
	//SPI_UnselectAllSlaves();
	//SPI_SelectSlave(SPILCD);
	SPI_MasterTransmit(value & 0xff);
	SPI_UnselectAllSlaves();
}

void LCDUpdate(void) {
	//LCDUpdateValue(0, 0, 2);
	SPI_SelectSlave(SPILCD);
	asm volatile ("nop");
	SPI_MasterTransmit(1);
	asm volatile ("nop");
	SPI_MasterTransmit(2);
	asm volatile ("nop");
	SPI_MasterTransmit(3);
	asm volatile ("nop");
	SPI_UnselectAllSlaves();
}
