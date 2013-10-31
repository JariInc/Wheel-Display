/* Control: PORTC */
#define KS0108_CTRL PORTC
#define KS0108_CTRL_DIR DDRC

#define RS (1 << 2)
#define RW (1 << 4)
#define CS1 (1 << 6)
#define CS2 (1 << 5)
#define EN (1 << 7)

/* Data: PORTD */
#define KS0108_DATA PORTD
#define KS0108_DATA_DIR DDRD

void ks0108_controller_disable(uint8_t i);
void ks0108_controller_enable(uint8_t i);
uint8_t ks0108_readdata(uint8_t controller, uint8_t status);
void ks0108_instruction(uint8_t controller, uint8_t instruction, uint8_t data);
void ks0108_init(void);
void ks0108_clear(void);
void ks0108_clear_range(uint8_t lpage, uint8_t hpage, uint8_t laddress, uint8_t haddress);

