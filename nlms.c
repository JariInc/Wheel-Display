#include <inttypes.h>
#include "nlms.h"

#define N 16
#define WSCALE 8;

// static
static int16_t x[N]; 		// input buffer
static int16_t n_1[N];
static int32_t w[N]; 		// Q8.8
static uint8_t xPtr;

void NLMSinit(void) {
	uint8_t i;
	xPtr = 0;
	for(i = 0; i < N; i++) {
		x[i] = 0;
		n_1[i] = 0;
		w[i] = 0;
	}
	w[0] = 1 << WSCALE;
}

int16_t NLMS(int16_t voice, int16_t noise) {
	int32_t n_3;		// output
	int16_t n;			// n_1 estimate
	int16_t e;			// error
	int16_t mu;
	int32_t P_hat = 0;	// power
	uint8_t i, j;

	// get new samples
	x[xPtr] = voice;
	n_1[xPtr] = noise;
	
	// reset result
	n_3 = 0;
	
	// reset n2sum
	P_hat = 0;
	
	// filter
	for(j = 0; j < N-1; j++) {
		n_3 += (n_1[xPtr - j] * w[j]) >> 8;
		P_hat += x[xPtr - j] * x[xPtr - j];
	}
	
	// calculate error
	e = x[xPtr] - n_3;
	
	// calculate mu
	mu = P_hat << 16;
	
	// update w
	for(j = 0; j < N-1; j++) {
		w[j] = w[j] + ((mu*x[xPtr-j]*e) >> 8);
	}
	
	xPtr = (xPtr+1) & (N-1);

	return e;
}
