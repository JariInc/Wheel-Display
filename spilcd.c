#include "spi.h"
#include "types.h"

extern volatile uint16_t data[32];

void LCDUpdateValue(uint8_t pos, uint8_t type, uint16_t value) {

	while (!(UCSR1A & (1<<UDRE1)));
	UDR1 = (pos << 5)|type;
	while (!(UCSR1A & (1<<UDRE1)));
	UDR1 = value >> 8;
	while (!(UCSR1A & (1<<UDRE1)));
	UDR1 = value & 0xff;
	while (!(UCSR1A & (1<<UDRE1)));

	uint8_t i;
	for(i = 0; i < 6; i++) {
		while (!(UCSR1A & (1<<UDRE1)));
		UDR1 = 0xff;
	}
}


void LCDUpdate(void) {

	
	LCDUpdateValue(0, 0, data[GEAR]++ & 7);

	// row 1
	LCDUpdateValue(1, RPM, data[RPM]++);
	LCDUpdateValue(2, SPEED, 107);
	
	// row 2
	LCDUpdateValue(3, FUEL, data[FUEL]);
	LCDUpdateValue(4, LAP, data[LAP]);


	// row 3
	LCDUpdateValue(5, LAPTIME, data[LAPTIME]);
	LCDUpdateValue(6, DELTA, data[DELTA]);

}

