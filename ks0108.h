/* Control: PORTD */
#define KS0108_CTRL PORTD
#define KS0108_CTRL_DIR DDRD
#define RS (1 << 0)
#define RW (1 << 1)
#define CS1 (1 << 3)
#define CS2 (1 << 2)
#define EN (1 << 5)

/* Data: PORTA */
#define ks0108_DATA PORTA
#define ks0108_DATA_DIR DDRA

void ks0108_controller_disable(uint8_t i);
void ks0108_controller_enable(uint8_t i);
uint8_t ks0108_readdata(uint8_t controller, uint8_t status);
void ks0108_instruction(uint8_t controller, uint8_t instruction, uint8_t data);
void ks0108_init();
void ks0108_clear();
void ks0108_clear_range(uint8_t lpage, uint8_t hpage, uint8_t laddress, uint8_t haddress);
