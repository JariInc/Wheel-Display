#include <avr/io.h>
#include <avr/interrupt.h>

#define SPI_BUFFER_SIZE 32

#define SPIUSB 0
#define SPIUSB_PORT PORTB
#define SPIUSB_DDR DDRB
#define SPIUSB_PIN PB4

#define SPIGPIO 1
#define SPIGPIO_PORT PORTB
#define SPIGPIO_DDR DDRB
#define SPIGPIO_PIN PB3

#define SPIADC 2
#define SPIADC_PORT PORTB
#define SPIADC_DDR DDRB
#define SPIADC_PIN PB1

#define IN 0
#define OUT 1

void SPI_MasterInit(void);
char SPI_MasterTransmit(char cData);
//void SPI_MasterTransmit(char cData);
void SPI_UnselectAllSlaves();
void SPI_SelectSlave(uint8_t slave);
