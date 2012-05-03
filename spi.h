#include <avr/io.h>

#define SPILCD 0
#define SPILCD_PORT PORTB
#define SPILCD_DDR DDRB
#define SPILCD_PIN PB0

#define SPIGPIO 1
#define SPIGPIO_PORT PORTB
#define SPIGPIO_DDR DDRB
#define SPIGPIO_PIN PB4

#define SPIADC 2
#define SPIADC_PORT PORTB
#define SPIADC_DDR DDRB
#define SPIADC_PIN PB5

void SPI_MasterInit(void);
char SPI_MasterTransmit(char cData);
void SPI_UnselectAllSlaves(void);
void SPI_SelectSlave(uint8_t slave);
