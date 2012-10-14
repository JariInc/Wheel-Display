#define N       7                    // LMS adaptive filter length
#define C       0x20                   // Safty factor
#define BETA    (int16_t)(0.99*32767)    // Beta for compute integrator
#define ONE_MINUS_BETA (32767-BETA)
#define SCALE   15
#define ROUND   (1 << SCALE-1)              // Rounding for fixed point arithmatic
#define ALPHA   (1 << SCALE)            // LMS adaptive filter step size
#define LEAK    4

int16_t nlms(int16_t voice, int16_t noise);
