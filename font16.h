#include <avr/pgmspace.h>
/*

	04b_19, 18px

*/

uint8_t font16[10][2][9] PROGMEM = {
	{		// 0
		{0xfe, 0xfe, 0xfe, 0xfe, 0x1e, 0xfe, 0xfe, 0xfe, 0xfe},
		{0x7f, 0x7f, 0x7f, 0x7f, 0x78, 0x7f, 0x7f, 0x7f, 0x7f}

	}, {	// 1
		{0x1e, 0xfe, 0xfe, 0xfe, 0xfe},
		{0x00, 0x7f, 0x7f, 0x7f, 0x7f}
	}, { 	// 2
		{0xde, 0xde, 0xde, 0xde, 0xde, 0xfe, 0xfe, 0xfe, 0xfe},
		{0x7f, 0x7f, 0x7f, 0x7f, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b}
	}, {	// 3
		{0xde, 0xde, 0xde, 0xde, 0xde, 0xfe, 0xfe, 0xfe, 0xfe},
		{0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7f, 0x7f, 0x7f, 0x7f}
	}, {	// 4
		{0xfe, 0xfe, 0xfe, 0xfe, 0x00, 0xfe, 0xfe, 0xfe, 0xfe},
		{0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x7f, 0x7f, 0x7f, 0x7f}
	}, {	// 5
		{0xfe, 0xfe, 0xfe, 0xfe, 0xde, 0xde, 0xde, 0xde, 0xde},
		{0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7f, 0x7f, 0x7f, 0x7f}
	}, {	// 6
		{0xfe, 0xfe, 0xfe, 0xfe, 0xde, 0xde, 0xde, 0xde, 0xde},
		{0x7f, 0x7f, 0x7f, 0x7f, 0x7b, 0x7f, 0x7f, 0x7f, 0x7f}
	}, {	// 7
		{0xfe, 0xfe, 0xfe, 0xfe, 0x0e, 0xfe, 0xfe, 0xfe, 0xfe},
		{0x01, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x7f, 0x7f, 0x7f}
	}, {	// 8
		{0xfe, 0xfe, 0xfe, 0xfe, 0xde, 0xfe, 0xfe, 0xfe, 0xfe},
		{0x7f, 0x7f, 0x7f, 0x7f, 0x7b, 0x7f, 0x7f, 0x7f, 0x7f}
	}, {	// 9
		{0xfe, 0xfe, 0xfe, 0xfe, 0xde, 0xfe, 0xfe, 0xfe, 0xfe},
		{0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7f, 0x7f, 0x7f, 0x7f}
	}
};