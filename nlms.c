#include <inttypes.h>
#include <limits.h>
#include "nlms.h"

int16_t nlms(int16_t voice, int16_t noise) {

	// static
	static int16_t x[N]; 		// input buffer
	static int16_t n[N];
	static int16_t w[N]; 		// s0.15
	static int16_t p = 1024;
	static int16_t c;
	static int16_t leak;

	// non static
	int32_t temp32;
	uint8_t i;					// indices
	
	// debug
	static int32_t max, min;

	// shift old samples
	for(i=N-1; i > 0; i--)
  {
    x[i] = x[i-1];
    n[i] = n[i-1];
  }

	// get new samples
	x[0] = voice;
	n[0] = noise;

  temp32 = (((int32_t)x[0] * x[0])+ROUND)>>SCALE;
  temp32 = ((temp32 * ONE_MINUS_BETA)+ROUND)>>SCALE;           
  p = (int16_t)(((p * (int32_t)BETA)+ROUND)>>SCALE);  
  
  temp32 += (p+c);
  temp32 >>= 1;             
  int16_t mu = ALPHA / (int32_t)temp32;
  
  // filter
  temp32 = (int32_t)w[0] * n[0];
	for(i=1; i < N; i++)
    temp32 += (int32_t)w[i] * n[i];

	// result
	int16_t e = x[0] - (int16_t)((temp32+ROUND)>>SCALE);

	// update filter coeffs
	int16_t ue = (int16_t)(((e * (int32_t)mu)+ROUND)>>SCALE);

	int32_t newleak = 0; // leak scaler

  for(i=0; i < N ; i++) {
  	temp32 = w[i] - leak + ((((int32_t)ue * x[i])+ROUND)>>SCALE);
  	
  	if(temp32 >= SHRT_MAX)
  		w[i] = SHRT_MAX;
  	else if(w[i] <= SHRT_MIN)
  		w[i] = SHRT_MIN;
  	else
	 		w[i] = temp32;
	 	
	 	newleak += w[i];
	}
	
	leak = newleak >> (N-LEAK);

	return e;
}
