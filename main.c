#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/pgmspace.h>   /* required by usbdrv.h */

#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */
#include "requests.h"       /* The custom request numbers we use */
#include "lcd.h"
#include "spi.h"
#include "mcp23s17.h"
#include "mcp3204.h"
#include "timer.h"

#define BAUD 57600
#include <util/setbaud.h>

void USART_Transmit( unsigned char data ) {
	while (!( UCSRA & (1<<UDRE)));
	UDR = data;
}

volatile LCDtext speed;
volatile LCDtext rpm;
volatile uint8_t rpmleds;
volatile LCDtext gear;
volatile buttons btns;

/*
PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = { /* USB report descriptor, size must match usbconfig.h 
	0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,        // USAGE (Game Pad)
	0xa1, 0x01,        // COLLECTION (Application)
	0x09, 0x01,        //   USAGE (Pointer)
	0xa1, 0x00,        //   COLLECTION (Physical)
	0x09, 0x30,        //     USAGE (X)
	0x09, 0x31,        //     USAGE (Y)
	0x15, 0x00,        //   LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,  //     LOGICAL_MAXIMUM (255)
	0x75, 0x08,        //   REPORT_SIZE (8)
	0x95, 0x02,        //   REPORT_COUNT (2)
	0x81, 0x02,        //   INPUT (Data,Var,Abs)
	0xc0,              // END_COLLECTION
	0x05, 0x09,        // USAGE_PAGE (Button)
	0x19, 0x01,        //   USAGE_MINIMUM (Button 1)
	0x29, 0x08,        //   USAGE_MAXIMUM (Button 8)
	0x15, 0x00,        //   LOGICAL_MINIMUM (0)
	0x25, 0x01,        //   LOGICAL_MAXIMUM (1)
	0x75, 0x01,        // REPORT_SIZE (1)
	0x95, 0x08,        // REPORT_COUNT (8)
	0x81, 0x02,        // INPUT (Data,Var,Abs)
	0xc0               // END_COLLECTION
};

static unsigned char last_read_bytes[2];
static unsigned char last_reported_bytes[2];
static uchar reportBuffer[3];    /* buffer for HID reports 
static uchar idleRate;           /* in 4 ms units 

static void buildReport(void) {
	int x, y;
	unsigned char lrcb1, lrcb2;

	lrcb1 = last_read_bytes[0];
	lrcb2 = last_read_bytes[1];

	last_reported_bytes[0] = lrcb1;
	last_reported_bytes[1] = lrcb2;

	y = x = 128;
	if(lrcb1 & 0x01) x = 255;
	if(lrcb1 & 0x02) x = 0;
	
	if(lrcb1 & 0x04) y = 255;
	if(lrcb1 & 0x08) y = 0;

	reportBuffer[0] = x;
	reportBuffer[1] = y;
	reportBuffer[2] = lrcb1 & 0xF0;
	reportBuffer[2] |= lrcb2 >> 4;
}

uchar usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (void *)data;

    usbMsgPtr = reportBuffer;
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type 
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) 
            /* we only have one report type, so don't look at wValue 
            buildReport();
            return sizeof(reportBuffer);
        }else if(rq->bRequest == USBRQ_HID_GET_IDLE){
            usbMsgPtr = &idleRate;
            return 1;
        }else if(rq->bRequest == USBRQ_HID_SET_IDLE){
            idleRate = rq->wValue.bytes[1];
        }
    }else{
        /* no vendor specific requests implemented 
    }
	return 0;
}
*/
int main() {

	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
	#if USE_2X
	UCSRA |= (1 << U2X);
	#else
	UCSRA &= ~(1 << U2X);
	#endif
	UCSRB = (1<<RXEN)|(1<<TXEN);
	USART_Transmit(0x01);

	// SPI
	SPI_MasterInit();
	
	// LCD
	LCDinit();

	// GPIO
	GPIOInit();

	// ADC
	ADCinit();

	// Timer
	TimerInit();
	TimerSet(TIMER0, 0xff);
	TimerSetFreq(TIMER1, 8000);

	sei();

	while(1);
/*
	// USB
	uchar   i;

    wdt_enable(WDTO_1S);
    /* Even if you don't use the watchdog, turn it off here. On newer devices,
     * the status of the watchdog (on/off, period) is PRESERVED OVER RESET!
     
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     
    odDebugInit();
    DBG1(0x00, 0, 0);       /* debug output: main starts 
    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! 
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms 
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();
    DBG1(0x01, 0, 0);       /* debug output: main loop starts 
	for(;;) { //main event loop
		wdt_reset();
		usbPoll();

		if(usbInterruptIsReady() && btns.interrupt){
			buildReport();
			usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
			btns.interrupt = 0x00;
			btns.prevstate = btns.state;
		}		
	}
*/
	return 0;
}
