#include <avr/io.h>

#define BAUD 38400
#include <util/setbaud.h>

void uartTx(char data) {
	/* Wait for empty transmit buffer */
	while ( !(UCSR1A & (1<<UDRE1)) );
	/* Put data into buffer, sends the data */
	UDR1 = data;
}

char uartRx(void) {
	/* Wait for data to be received */
	while ( !(UCSR1A & (1<<RXC1)) );
	/* Get and return received data from buffer */
	return UDR1;
}

void uartInit(void) {
	UBRR1H = UBRRH_VALUE;
	UBRR1L = UBRRL_VALUE;
	UCSR1B = (1<<RXEN1)|(1<<TXEN1);
}
