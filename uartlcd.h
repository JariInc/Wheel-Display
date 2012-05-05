#define UPDATELCD(pos, type, value) \
	uartTx((pos<<5)|type);\
	uartTx(value >> 8);\
	uartTx(value & 0xff);\
	lcdReady = 1


void LCDUpdateValue(uint8_t pos, uint8_t type, uint16_t value);
void LCDUpdate(void);
