#define TIMER0 0
#define TIMER1 1

typedef struct {
	uint8_t state;
	uint8_t prevstate;
	uint8_t interrupt;
} buttons;

extern volatile buttons btns;

void TimerInit(uint8_t timer);
void TimerSet(uint8_t timer, uint16_t val);
void TimerSetFreq(uint8_t timer, uint16_t freq);
