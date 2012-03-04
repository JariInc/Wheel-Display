#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "max3420.h"
#include "spi.h"
#include "usbdescriptors.h"

#define SETBIT(reg,val) USBWrite(reg,(USBRead(reg) | val));
#define CLRBIT(reg,val) USBWrite(reg,(USBRead(reg) &~ val));
#define ACK ackstat=1
#define STALL_EP0 USBWrite(EPSTALLS, 0x23)

//#define SERIALDEBUG 1

// registers
#define EP0FIFO		0x00
#define EP1OUTFIFO	0x01
#define EP3INFIFO	0x03
#define SUDFIFO		0x04
#define EP0BC		0x05
#define EP3INBC		0x08
#define EPSTALLS	0x09
#define CLRTOGS		0x0a
#define EPIRQ		0x0b
#define EPIEN		0x0c
#define USBIRQ		0x0d
#define USBIEN		0x0e
#define USBCTL		0x0f
#define CPUCTL		0x10
#define PINCTL		0x11
#define FNADDR		0x13
#define IOPINS		0x14

// SETUP packet offsets
#define bmRequestType	0
#define	bRequest		1
#define wValueL			2
#define wValueH			3
#define wIndexL			4
#define wIndexH			5
#define wLengthL		6
#define wLengthH		7

// Standard USB Requests
#define SR_GET_STATUS			0x00	// Get Status
#define SR_CLEAR_FEATURE		0x01	// Clear Feature
#define SR_RESERVED				0x02	// Reserved
#define SR_SET_FEATURE			0x03	// Set Feature
#define SR_SET_ADDRESS			0x05	// Set Address
#define SR_GET_DESCRIPTOR		0x06	// Get Descriptor
#define SR_SET_DESCRIPTOR		0x07	// Set Descriptor
#define SR_GET_CONFIGURATION	0x08	// Get Configuration
#define SR_SET_CONFIGURATION	0x09	// Set Configuration
#define SR_GET_INTERFACE		0x0a	// Get Interface
#define SR_SET_INTERFACE		0x0b	// Set Interface

// Get Descriptor codes	
#define GD_DEVICE			0x01	// Get device descriptor: Device
#define GD_CONFIGURATION	0x02	// Get device descriptor: Configuration
#define GD_STRING			0x03	// Get device descriptor: String
#define GD_HID	            0x21	// Get descriptor: HID
#define GD_REPORT	        0x22	// Get descriptor: Report

extern volatile uint8_t usbready;

volatile char sudfifo[8];		// Local copy of the 8 setup data read from the MAX3420E SUDFIFO
volatile char ackstat;			// ACKSTAT global
volatile char configval;		// Set/Get_Configuration value
volatile char RWU_enabled;		// Set by Set/Clear_Feature RWU request, sent back for Get_Status-RWU
volatile char ep3stall;			// Flag for EP3 Stall, set by Set_Feature, reported back in Get_Status
volatile char Suspended;		// Tells the main loop to look for host resume and RWU pushbutton
volatile char inhibit_send;		// Flag for the keyboard character send routine
volatile char send3zeros;		// EP3-IN function uses this to send HID (key up) codes between keystrokes
volatile char msgidx, msglen;	// Text string in EnumApp_enum_data.h--index and length

const unsigned char Message[]={ // each letter is 3 bytes--shiftcode, 00, HID keycode
	0x00,0x00,0x28,         // (cr)
        0x02,0x00,0x17,		// T (02 is shift)
	0x00,0x00,0x0B,		// h
	0x00,0x00,0x08,		// e
	0x00,0x00,0x2C,		// (sp)
	0x02,0x00,0x10,		// M
	0x02,0x00,0x04,		// A
	0x02,0x00,0x1B,		// X
	0x00,0x00,0x20,		// 3
	0x00,0x00,0x21,		// 4
	0x00,0x00,0x1F,		// 2
	0x00,0x00,0x27,		// 0
	0x02,0x00,0x08,		// E
	0x00,0x00,0x2C,		// (sp)
	0x00,0x00,0x07,		// d
	0x00,0x00,0x12,		// o
	0x00,0x00,0x08,		// e
	0x00,0x00,0x16,		// s
	0x00,0x00,0x2C,		// (sp)
	0x02,0x00,0x18,		// U
	0x02,0x00,0x16,		// S
	0x02,0x00,0x05,		// B
	0x02,0x00,0x1E,		// !
	0x00,0x00,0x28};	// (cr)


void USBEnableIRQ() {
	// MAX3420E IRQ
	USBWrite(EPIEN, (1 << 5) | (1 << 4)); // SUDAVIE IN3BAVIE
    USBWrite(USBIEN, (1 << 7) | (1 << 3)); // URESIE URESDNIE

	// AVR IRQ
	DDRD &= ~(1 << 2);
	PORTD |= (1 << 2);
	MCUCR |= (1 << ISC01) | (0 << ISC00);
	GICR |= (1 << INT0);
}

void USBInit() {
	ackstat = 0;
	usbready = 0;

	msgidx = 0;			// start of KB Message[]
	msglen = sizeof(Message);
	inhibit_send = 0x01;		// 0 means send, 1 means inhibit sending
	send3zeros=1;

	configval=0;                    // at pwr on OR bus reset we're unconfigured
	Suspended=0;
	RWU_enabled=0;                  // Set by host Set_Feature(enable RWU) request

	USBWrite(PINCTL, (1 << 4) | /*(1 << 3) |*/ 0x02); // FDUPSPI INTLEVEL GPXB GPXA as SOF
	USBReset();

	USBEnableIRQ();

    USBWrite(EPIRQ, 0xff); // clear interrupt flags
    USBWrite(USBIRQ, 0xff); // clear interrupt flags

	USBWrite(CPUCTL, 0x01); // interrupt enable
	USBWrite(USBCTL, /*(1 << 6) | */(1 << 3)); // vbgate & connect

	USBWrite(IOPINS, 0x00);

	SETBIT(IOPINS, (1 << 3));
	usbready = 1;
}

void USBReset() {
	char dum;

	SPI_SelectSlave(SPIUSB);
	USBWrite(USBCTL, 0x20);	// chip reset
	USBWrite(USBCTL, 0x00);	// remove the reset
    do {                 // Chip reset stops the oscillator. Wait for it to stabilize.
	    dum = USBRead(USBIRQ);
	    dum &= 0x01; // OSCOKIRQ
    } while (dum == 0x00);
	SPI_UnselectAllSlaves();
}

void USBSendDescriptor() {
	uint16_t reqlen,sendlen,desclen;
	char *pDdata; // pointer to ROM Descriptor data to send
	//
	// NOTE This function assumes all descriptors are 64 or fewer bytes and can be sent in a single packet
	//
	desclen = 0;					// check for zero as error condition (no case statements satisfied)
	reqlen = (sudfifo[wLengthH] << 8) | sudfifo[wLengthL];
	switch (sudfifo[wValueH]) {			// wValueH is descriptor type
		case GD_DEVICE:
			USART_Transmit_Str("GD_DEVICE");
			desclen = DD[0];	// descriptor length
			pDdata = DD;
			break;	
		case GD_CONFIGURATION:
			USART_Transmit_Str("GD_CONFIGURATION");
			desclen = CD[2];	// Config descriptor includes interface, HID, report and ep descriptors
			pDdata = CD;
			break;
		case GD_STRING:
			USART_Transmit_Str("GD_STRING");
			desclen = strDesc[sudfifo[wValueL]][0];   // wValueL=string index, array[0] is the length
			pDdata = strDesc[sudfifo[wValueL]];       // point to first array element
			break;
		case GD_HID:
			USART_Transmit_Str("GD_HID");
			desclen = CD[18];
			pDdata = &CD[18];
			break;
		case GD_REPORT:
			USART_Transmit_Str("GD_REPORT");
			desclen = CD[25];
			pDdata = RepD;
			break;
	}	// end switch on descriptor type
	//
	if (desclen != 0) {                  // one of the case statements above filled in a value
		sendlen = (reqlen <= desclen) ? reqlen : desclen; // send the smaller of requested and avaiable
		USBWriteBuffer(EP0FIFO, pDdata, sendlen);
		ACK;
		USBWrite(EP0BC, sendlen);   // load EP0BC to arm the EP0-IN transfer & ACKSTAT
		USART_Transmit_Int(sendlen);
	}
	else 
		STALL_EP0;  // none of the descriptor types match
}

void USBSetup() {
	USBReadBuffer(SUDFIFO, &sudfifo, 8);  // got a SETUP packet. Read 8 SETUP bytes
	switch(sudfifo[bmRequestType] & 0x60) {    // Parse the SETUP packet. For request type, look only at b6&b5
		case 0x00:
			switch(sudfifo[bRequest]) {
				case SR_GET_DESCRIPTOR:
					USART_Transmit_Str("SR_GET_DESCRIPTOR");
					USBSendDescriptor();
					break;
				case SR_SET_FEATURE:
					USART_Transmit_Str("SR_SET_FEATURE");
					USBFeature(0);
					break;
				case SR_CLEAR_FEATURE:
					USART_Transmit_Str("SR_CLEAR_FEATURE");
					USBFeature(1);
					break;
				case SR_GET_STATUS:
					USART_Transmit_Str("SR_GET_STATUS");
					USBGetStatus();
					break;
				case SR_SET_INTERFACE:
					USART_Transmit_Str("SR_SET_INTERFACE");
					USBSetIf();
					break;
				case SR_GET_INTERFACE:
					USART_Transmit_Str("SR_GET_INTERFACE");
					USBGetIf();
					break;
				case SR_GET_CONFIGURATION:
					USART_Transmit_Str("SR_GET_CONFIGURATION");
					USBGetConfig();
					break;
				case SR_SET_CONFIGURATION:
					USART_Transmit_Str("SR_SET_CONFIGURATION");
					USBSetConfig();
					break;
				case SR_SET_ADDRESS:
					USART_Transmit_Str("SR_SET_ADDRESS");
					ACK;
					USBRead(FNADDR);
					break;
				default:
					USART_Transmit_Str("STALL_EP0");
					STALL_EP0;
			}
			break;
		case 0x20:
			USART_Transmit_Str("STALL_EP0");
			STALL_EP0;
			break;  // just a stub in this program
		case 0x40:
			USART_Transmit_Str("STALL_EP0");
			STALL_EP0;
			break;  // just a stub in this program
		default:
			USART_Transmit_Str("STALL_EP0");
			STALL_EP0; // unrecognized request type
	}
}

void USBWrite(char addr, char data) {
	SPI_SelectSlave(SPIUSB);
	SPI_MasterTransmit((addr << 3)|0x02|(ackstat & 1));
	SPI_MasterTransmit(data);
	SPI_UnselectAllSlaves();
	#ifdef SERIALDEBUG
	USART_Transmit(data);
	#endif
	if(ackstat)
		USART_Transmit_Str("ACK");
	ackstat = 0;
}

char USBRead(char addr) {
	char input;
	SPI_SelectSlave(SPIUSB);
	SPI_MasterTransmit((addr << 3)|(ackstat & 1));
	input = SPI_MasterTransmit(0x00);
	SPI_UnselectAllSlaves();
	#ifdef SERIALDEBUG
	USART_Transmit(input);
	#endif
	if(ackstat)
		USART_Transmit_Str("ACK");
	ackstat = 0;
	return input;
}

void USBWriteBuffer(char addr, char buffer[], uint8_t length) {
	uint8_t i = 0;
	SPI_SelectSlave(SPIUSB);
	SPI_MasterTransmit((addr << 3)|0x02|(ackstat & 1));
	for(i=0; i < length; i++) {
		SPI_MasterTransmit(buffer[i]);
		#ifdef SERIALDEBUG
		USART_Transmit(buffer[i]);
		#endif
	}
	SPI_UnselectAllSlaves();
	if(ackstat)
		USART_Transmit_Str("ACK");
	ackstat = 0;
}

void USBReadBuffer(char addr, char buffer[], uint8_t length) {
	char input;
	uint8_t i = 0;

	SPI_SelectSlave(SPIUSB);
	SPI_MasterTransmit((addr << 3)|(ackstat & 1));
	for(i=0; i < length; i++) {
		buffer[i] = SPI_MasterTransmit(0x00);
		#ifdef SERIALDEBUG
		USART_Transmit(buffer[i]);
		#endif
	}
	SPI_UnselectAllSlaves();

	if(ackstat)
		USART_Transmit_Str("ACK");

	buffer[length] = 0x00;
	ackstat = 0;
}

void USBSetConfig() {
	configval = sudfifo[wValueL];           // Store the config value
	if(configval != 0)                // If we are configured, 
		SETBIT(USBIEN, 1 << 4);       // start looking for SUSPEND interrupts
	ACK;
	USBRead(FNADDR);                  // dummy read to set the ACKSTAT bit
}

void USBGetConfig() {
	USBWrite(EP0FIFO, configval);         // Send the config value
	ACK;
	USBWrite(EP0BC, 1);   
}

void USBSetIf() { // All we accept are Interface=0 and AlternateSetting=0, otherwise send STALL
	if((sudfifo[wValueL] == 0 && (sudfifo[wIndexL] == 0))) {		// wValueL=Alternate Setting index wIndexL=Interface index
	  	ACK;
	  	USBRead(FNADDR);	// dummy read to set the ACKSTAT bit
	}
	else STALL_EP0;
}

void USBGetIf() {	// Check for Interface=0, always report AlternateSetting=0
	if(sudfifo[wIndexL] == 0) {		// wIndexL=Interface index
		USBWrite(EP0FIFO, 0);		// AS=0
		ACK;
		USBWrite(EP0BC, 1);		// send one byte, ACKSTAT
	}
	else STALL_EP0;
}

void USBGetStatus() {
	switch(sudfifo[bmRequestType]) {
		case 0x80: 			// directed to DEVICE
			USBWrite(EP0FIFO, (RWU_enabled+1));	// first byte is 000000rs where r=enabled for RWU and s=self-powered.
			USBWrite(EP0FIFO, 0x00);		// second byte is always 0
			ACK;
			USBWrite(EP0BC, 2); 		// load byte count, arm the IN transfer, ACK the status stage of the CTL transfer
			break; 				
		case 0x81: 			// directed to INTERFACE
			USBWrite(EP0FIFO, 0x00);		// this one is easy--two zero bytes
			USBWrite(EP0FIFO, 0x00);		
			ACK;
			USBWrite(EP0BC, 2); 		// load byte count, arm the IN transfer, ACK the status stage of the CTL transfer
			break; 				
		case 0x82: 			// directed to ENDPOINT
			if(sudfifo[wIndexL] == 0x83) {		// We only reported ep3, so it's the only one the host can stall IN3=83
	        	USBWrite(EP0FIFO, ep3stall);	// first byte is 0000000h where h is the halt (stall) bit
				USBWrite(EP0FIFO, 0x00);		// second byte is always 0
				ACK;
				USBWrite(EP0BC, 2); 		// load byte count, arm the IN transfer, ACK the status stage of the CTL transfer
				break;
			}
			else  STALL_EP0;		// Host tried to stall an invalid endpoint (not 3)				
		default:
			STALL_EP0;		// don't recognize the request
	}
}

// **********************************************************************************************
// FUNCTION: Set/Get_Feature. Call as feature(1) for Set_Feature or feature(0) for Clear_Feature.
// There are two set/clear feature requests:
//	To a DEVICE: 	Remote Wakeup (RWU). 
//  	To an ENDPOINT:	Stall (EP3 only for this app)
//
void USBFeature(uint8_t sc)
{
	char mask;
	if((sudfifo[bmRequestType]==0x02)	// dir=h->p, recipient = ENDPOINT
	  &&  (sudfifo[wValueL]==0x00)		// wValueL is feature selector, 00 is EP Halt
	  &&  (sudfifo[wIndexL]==0x83)) {	// wIndexL is endpoint number IN3=83
		mask = USBRead(EPSTALLS);   // read existing bits
		if(sc == 1) {              // set_feature
			mask += (1 << 4);       // Halt EP3IN (STLEP3IN)
			ep3stall=1;
		}
		else {                       // clear_feature
			mask &= ~(1 << 4);      // UnHalt EP3IN (STLEP3IN)
			ep3stall=0;
			USBWrite(CLRTOGS, 1 << 4);  // clear the EP3 data toggle (CTGEP3IN)
		}
		USBWrite(EPSTALLS, (mask | (1 << 6))); // Don't use wregAS for this--directly writing the ACKSTAT bit
	}
	else if ((sudfifo[bmRequestType]==0x00)	// dir=h->p, recipient = DEVICE
	  && (sudfifo[wValueL] == 0x01)) {	// wValueL is feature selector, 01 is Device_Remote_Wakeup
		RWU_enabled = sc<<1;	// =2 for set, =0 for clear feature. The shift puts it in the get_status bit position.			
		ACK;
		USBRead(FNADDR);		// dummy read to set ACKSTAT
	}
	else STALL_EP0;
}

void USBServeIRQ() {

}

ISR(INT0_vect) {
	GICR &= ~(1 << INT0);
	
	uint8_t irqserved = 0;
	char epint, usbint;

	do {
		irqserved = 0;
		
		epint = USBRead(EPIRQ);							// Check the EPIRQ bits
		usbint = USBRead(USBIRQ);						// Check the USBIRQ bits

		if(epint & (1 << 5)) {
			USART_Transmit_Str("setup");
			USBWrite(EPIRQ, (1 << 5));					// clear the SUDAV IRQ
			USBSetup();
			irqserved++;
		}
/*
		if(epint & (1 << 4)) {	// Was an EP3-IN packet just dispatched to the host?
		    do_IN3();			// Yes--load another keystroke and arm the endpoint
			USART_Transmit_Str("in3");	// NOTE: don't clear the IN3BAVIRQ bit here--loading the EP3-IN byte
			irqserved++;
		}						// count register in the do_IN3() function does it.
*/
		if((configval != 0) && (usbint & (1 << 4))) {	// HOST suspended bus for 3 msec
			USART_Transmit_Str("suspend");
			USBWrite(USBIRQ,((1 << 4) | (1 << 2)));		// clear the IRQ and bus activity IRQ
			SETBIT(IOPINS, (1 << 2));					// turn on the SUSPEND light
			CLRBIT(IOPINS, (1 << 3));					// turn off blinking light (in case it's on)
			Suspended = 1;								// signal the main loop
			irqserved++;
		}

		if(usbint & (1 << 3)) {
			USART_Transmit_Str("reset");
			SETBIT(IOPINS, (1 << 1));					// turn the BUS RESET light on 
			CLRBIT(IOPINS, (1 << 2));					// Suspend light off (if on)
			USBWrite(USBIRQ, (1 << 3));					// clear the IRQ
			irqserved++;
		}

		if(usbint & (1 << 7)) {
			USART_Transmit_Str("resume");
			CLRBIT(IOPINS, (1 << 1));					// turn the BUS RESET light off
			USBWrite(USBIRQ, (1 << 7));				    // clear the IRQ bit
			Suspended = 0;								// in case we were suspended
			USBEnableIRQ();								// ...because a bus reset clears the IE bits
			irqserved++;
		}
	} while(irqserved);

	USBWrite(EPIRQ, 0xff); // clear interrupt flags
    USBWrite(USBIRQ, 0xff); // clear interrupt flags

	GIFR = 0x00;
	GICR |= (1 << INT0);
}

void do_IN3() {
	if (inhibit_send==0x01) {
		USBWrite(EP3INFIFO,0);			// send the "keys up" code
		USBWrite(EP3INFIFO,0);
		USBWrite(EP3INFIFO,0);
		}
	else if (send3zeros==0x01) {                        // precede every keycode with the "no keys" code
		USBWrite(EP3INFIFO,0);			// send the "keys up" code
		USBWrite(EP3INFIFO,0);
		USBWrite(EP3INFIFO,0);
		send3zeros=0;                           // next time through this function send the keycode
	}
	else {
		send3zeros=1;
		USBWrite(EP3INFIFO,Message[msgidx++]);	// load the next keystroke (3 bytes)
		USBWrite(EP3INFIFO,Message[msgidx++]);
		USBWrite(EP3INFIFO,Message[msgidx++]);
		if(msgidx >= msglen) {                   // check for message wrap
			msgidx=0;
			CLRBIT(IOPINS, 1);
			inhibit_send=1;                     // send the string once per pushbutton press
		}
	}
	USBWrite(EP3INBC,3);				// arm it
}

void USBChkResume() {
	if(USBRead(USBIRQ) & (1 << 2)) {    // THE HOST RESUMED BUS TRAFFIC
		CLRBIT(IOPINS, (1 << 1));
		Suspended=0;                    // no longer suspended
	}
	else if(RWU_enabled) {               // Only if the host enabled RWU
		if((USBRead(IOPINS) & 0x40) == 0) {       // See if the Remote Wakeup button was pressed
			CLRBIT(IOPINS, (1 << 1));		// turn off suspend light
			Suspended=0;                  // no longer suspended
			SETBIT(USBCTL, (1 << 2));      // signal RWU
			while ((USBRead(USBIRQ) & (1 << 1)) == 0) ;	// spin until RWU signaling done
			CLRBIT(USBCTL, (1 << 2));      // remove the RESUME signal 
			USBWrite(USBIRQ, (1 << 1));     // clear the IRQ
			while((USBRead(IOPINS) & 0x40)==0);  // hang until RWU button released
			USBWrite(USBIRQ, (1 << 2));    // wait for bus traffic -- clear the BUS Active IRQ
			while((USBRead(USBIRQ) & (1 << 2)) == 0); // & hang here until it's set again...
		}
	}
} 
