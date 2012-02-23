uint8_t LCD_writechar_8(uint8_t x, uint8_t page, uint8_t character);
uint8_t LCD_writechar_16(uint8_t x, uint8_t page, uint8_t character);
uint8_t LCD_writestring(uint8_t x, uint8_t page, char *string, uint8_t len, uint8_t large /*, uint8_t raling */);
uint8_t LCD_writeint(uint8_t x, uint8_t page, uint16_t value, uint8_t len, uint8_t large);
void LCD_writegear(uint8_t gear);
void LCDloop();
void LCDinit();
