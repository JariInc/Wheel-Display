#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>

/* Control: PORTD */
#define LCD_CTRL PORTD
#define LCD_CTRL_DIR DDRD
#define RS (1 << 0)
#define RW (1 << 1)
#define CS1 (1 << 3)
#define CS2 (1 << 2)
#define EN (1 << 5)

/* Data: PORTA */
#define LCD_DATA PORTA
#define LCD_DATA_DIR DDRA

void LCD_controller_disable(uint8_t i);
void LCD_controller_enable(uint8_t i);
uint8_t LCD_readdata(uint8_t controller, uint8_t status);
void LCD_instruction(uint8_t controller, uint8_t instruction, uint8_t data);
void LCD_initialize();
void LCD_clear();
void LCD_clear_range(uint8_t lpage, uint8_t hpage, uint8_t laddress, uint8_t haddress);
uint8_t LCD_writechar_8(uint8_t x, uint8_t page, uint8_t character);
uint8_t LCD_writechar_16(uint8_t x, uint8_t page, uint8_t character);
uint8_t LCD_writestring(uint8_t x, uint8_t page, char *string, uint8_t len, uint8_t large /*, uint8_t raling */);
uint8_t LCD_writeint(uint8_t x, uint8_t page, uint16_t value, uint8_t len, uint8_t large);
void LCD_writegear(uint8_t gear);
