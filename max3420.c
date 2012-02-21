#include "max3420.h"
#include "spi.h"

// MAX3420E_BF1.h
// Macros
// See the single bug fix below.
//
// For active-high lights attached to MAX3420E GP-Output pins.
#define L0_OFF wreg(rGPIO,(rreg(rGPIO) & 0xFE));
#define L0_ON wreg(rGPIO,(rreg(rGPIO) | 0x01));
#define L1_OFF wreg(rGPIO,(rreg(rGPIO) & 0xFD));
#define L1_ON wreg(rGPIO,(rreg(rGPIO) | 0x02));
#define L2_OFF wreg(rGPIO,(rreg(rGPIO) & 0xFB));
#define L2_ON wreg(rGPIO,(rreg(rGPIO) | 0x04));
#define L3_OFF wreg(rGPIO,(rreg(rGPIO) & 0xF7));
#define L3_ON wreg(rGPIO,(rreg(rGPIO) | 0x08));
#define L0_BLINK wreg(rGPIO,(rreg(rGPIO) ^ 0x01));
#define L1_BLINK wreg(rGPIO,(rreg(rGPIO) ^ 0x02));
#define L2_BLINK wreg(rGPIO,(rreg(rGPIO) ^ 0x04));
#define L3_BLINK wreg(rGPIO,(rreg(rGPIO) ^ 0x08));

#define SETBIT(reg,val) wreg(reg,(rreg(reg)|val));
#define CLRBIT(reg,val) wreg(reg,(rreg(reg)&~val));

// ************ BUG FIX ************
//#define STALL_EP0 wreg(9,0x23); // Set all three EP0 stall bits--data stage IN/OUT and status stage
// BUG FIX 2-9-06. The above statement hard-codes the register number to 9, ignoring the fact that
// the wreg function expects the register numbers to be pre-shifted 3 bits to put them into the 5 MSB's of
// the SPI command byte. Here is the correction:

#define STALL_EP0 wreg(rEPSTALLS,0x23);	// Set all three EP0 stall bits--data stage IN/OUT and status stage

// ******** END OF BUG FIX**********

#define MSB(word) (BYTE)(((WORD)(word) >> 8) & 0xff)
#define LSB(word) (BYTE)((WORD)(word) & 0xff)

// MAX3420E Registers
#define rEP0FIFO    0<<3
#define rEP1OUTFIFO 1<<3
#define rEP2INFIFO  2<<3
#define rEP3INFIFO  3<<3
#define rSUDFIFO    4<<3
#define rEP0BC      5<<3
#define rEP1OUTBC   6<<3
#define rEP2INBC    7<<3
#define rEP3INBC    8<<3
#define rEPSTALLS   9<<3
#define rCLRTOGS    10<<3
#define rEPIRQ      11<<3
#define rEPIEN      12<<3
#define rUSBIRQ     13<<3
#define rUSBIEN     14<<3
#define rUSBCTL     15<<3
#define rCPUCTL     16<<3
#define rPINCTL     17<<3
#define rRevision   18<<3
#define rFNADDR     19<<3
#define rGPIO       20<<3

// MAX3420E bit masks for register bits
// R9 EPSTALLS Register
#define bmACKSTAT   0x40
#define bmSTLSTAT   0x20
#define bmSTLEP3IN  0x10
#define bmSTLEP2IN  0x08
#define bmSTLEP1OUT 0x04
#define bmSTLEP0OUT 0x02
#define bmSTLEP0IN  0x01

// R10 CLRTOGS Register
#define bmEP3DISAB  0x80
#define bmEP2DISAB  0x40
#define bmEP1DISAB  0x20
#define bmCTGEP3IN  0x10
#define bmCTGEP2IN  0x08
#define bmCTGEP1OUT 0x04

// R11 EPIRQ register bits
#define bmSUDAVIRQ  0x20
#define bmIN3BAVIRQ 0x10
#define bmIN2BAVIRQ 0x08
#define bmOUT1DAVIRQ 0x04
#define bmOUT0DAVIRQ 0x02
#define bmIN0BAVIRQ 0x01

// R12 EPIEN register bits
#define bmSUDAVIE   0x20
#define bmIN3BAVIE  0x10
#define bmIN2BAVIE  0x08
#define bmOUT1DAVIE 0x04
#define bmOUT0DAVIE 0x02
#define bmIN0BAVIE  0x01

// R13 USBIRQ register bits
#define bmURESDNIRQ 0x80
#define bmVBUSIRQ   0x40
#define bmNOVBUSIRQ 0x20
#define bmSUSPIRQ   0x10
#define bmURESIRQ   0x08
#define bmBUSACTIRQ 0x04
#define bmRWUDNIRQ  0x02
#define bmOSCOKIRQ  0x01

// R14 USBIEN register bits
#define bmURESDNIE  0x80
#define bmVBUSIE    0x40
#define bmNOVBUSIE  0x20
#define bmSUSPIE    0x10
#define bmURESIE    0x08
#define bmBUSACTIE  0x04
#define bmRWUDNIE   0x02
#define bmOSCOKIE   0x01

// R15 USBCTL Register
#define bmHOSCSTEN  0x80
#define bmVBGATE    0x40
#define bmCHIPRES   0x20
#define bmPWRDOWN   0x10
#define bmCONNECT   0x08
#define bmSIGRWU    0x04

// R16 CPUCTL Register
#define bmIE        0x01

// R17 PINCTL Register
#define bmFDUPSPI   0x10
#define bmINTLEVEL  0x08
#define bmPOSINT    0x04
#define bmGPOB      0x02
#define	bmGPOA      0x01

//
// GPX[B:A] settings (PINCTL register)
#define gpxOPERATE  0x00
#define gpxVBDETECT 0x01
#define gpxBUSACT   0x02
#define gpxSOF      0x03

// ************************
// Standard USB Requests
#define SR_GET_STATUS		0x00	// Get Status
#define SR_CLEAR_FEATURE	0x01	// Clear Feature
#define SR_RESERVED		0x02	// Reserved
#define SR_SET_FEATURE		0x03	// Set Feature
#define SR_SET_ADDRESS		0x05	// Set Address
#define SR_GET_DESCRIPTOR	0x06	// Get Descriptor
#define SR_SET_DESCRIPTOR	0x07	// Set Descriptor
#define SR_GET_CONFIGURATION	0x08	// Get Configuration
#define SR_SET_CONFIGURATION	0x09	// Set Configuration
#define SR_GET_INTERFACE	0x0a	// Get Interface
#define SR_SET_INTERFACE	0x0b	// Set Interface

// Get Descriptor codes	
#define GD_DEVICE		0x01	// Get device descriptor: Device
#define GD_CONFIGURATION	0x02	// Get device descriptor: Configuration
#define GD_STRING		0x03	// Get device descriptor: String
#define GD_HID	            	0x21	// Get descriptor: HID
#define GD_REPORT	        0x22	// Get descriptor: Report

// SETUP packet offsets
#define bmRequestType           0
#define	bRequest		1
#define wValueL			2
#define wValueH			3
#define wIndexL			4
#define wIndexH			5
#define wLengthL		6
#define wLengthH		7

// HID bRequest values
#define GET_REPORT		1
#define GET_IDLE		2
#define GET_PROTOCOL            3
#define SET_REPORT		9
#define SET_IDLE		0x0A
#define SET_PROTOCOL            0x0B
#define INPUT_REPORT            1

// EnumApp_enum_data.h 
// Enumeration tables & HID keyboard data
//
const unsigned char DD[]=	// DEVICE Descriptor
        {0x12,	       		// bLength = 18d
        0x01,			// bDescriptorType = Device (1)
        0x00,0x01,		// bcdUSB(L/H) USB spec rev (BCD)
	0x00,0x00,0x00, 	// bDeviceClass, bDeviceSubClass, bDeviceProtocol
	0x40,			// bMaxPacketSize0 EP0 is 64 bytes
	0x6A,0x0B,		// idVendor(L/H)--Maxim is 0B6A
	0x46,0x53,		// idProduct(L/H)--5346
	0x34,0x12,		// bcdDevice--1234
	1,2,3,			// iManufacturer, iProduct, iSerialNumber
	1};			// bNumConfigurations

const unsigned char CD[]=	// CONFIGURATION Descriptor
	{0x09,			// bLength
	0x02,			// bDescriptorType = Config
	0x22,0x00,		// wTotalLength(L/H) = 34 bytes
	0x01,			// bNumInterfaces
	0x01,			// bConfigValue
	0x00,			// iConfiguration
	0xE0,			// bmAttributes. b7=1 b6=self-powered b5=RWU supported
	0x01,			// MaxPower is 2 ma
// INTERFACE Descriptor
	0x09,			// length = 9
	0x04,			// type = IF
	0x00,			// IF #0
	0x00,			// bAlternate Setting
	0x01,			// bNum Endpoints
	0x03,			// bInterfaceClass = HID
	0x00,0x00,		// bInterfaceSubClass, bInterfaceProtocol
	0x00,			// iInterface
// HID Descriptor--It's at CD[18]
	0x09,			// bLength
	0x21,			// bDescriptorType = HID
	0x10,0x01,		// bcdHID(L/H) Rev 1.1
	0x00,			// bCountryCode (none)
	0x01,			// bNumDescriptors (one report descriptor)
	0x22,			// bDescriptorType	(report)
	43,0,                   // CD[25]: wDescriptorLength(L/H) (report descriptor size is 43 bytes)
// Endpoint Descriptor
	0x07,			// bLength
	0x05,			// bDescriptorType (Endpoint)
	0x83,			// bEndpointAddress (EP3-IN)		
	0x03,			// bmAttributes	(interrupt)
	64,0,                   // wMaxPacketSize (64)
	10};			// bInterval (poll every 10 msec)


const unsigned char RepD[]=   // Report descriptor 
	{
	0x05,0x01,		// Usage Page (generic desktop)
	0x09,0x06,		// Usage (keyboard)
	0xA1,0x01,		// Collection
	0x05,0x07,		//   Usage Page 7 (keyboard/keypad)
	0x19,0xE0,		//   Usage Minimum = 224
	0x29,0xE7,		//   Usage Maximum = 231
	0x15,0x00,		//   Logical Minimum = 0
	0x25,0x01,		//   Logical Maximum = 1
	0x75,0x01,		//   Report Size = 1
	0x95,0x08,		//   Report Count = 8
	0x81,0x02,		//  Input(Data,Variable,Absolute)
	0x95,0x01,		//   Report Count = 1
	0x75,0x08,		//   Report Size = 8
	0x81,0x01,		//  Input(Constant)
	0x19,0x00,		//   Usage Minimum = 0
	0x29,0x65,		//   Usage Maximum = 101
	0x15,0x00,		//   Logical Minimum = 0,
	0x25,0x65,		//   Logical Maximum = 101
	0x75,0x08,		//   Report Size = 8
	0x95,0x01,		//   Report Count = 1
	0x81,0x00,		//  Input(Data,Variable,Array)
	0xC0};			// End Collection 

// STRING descriptors. An array of string arrays

const unsigned char strDesc[][64]= {
// STRING descriptor 0--Language string
{
        0x04,			// bLength
	0x03,			// bDescriptorType = string
	0x09,0x04		// wLANGID(L/H) = English-United Sates
},
// STRING descriptor 1--Manufacturer ID
{
        12,			// bLength
	0x03,			// bDescriptorType = string
	'M',0,'a',0,'x',0,'i',0,'m',0 // text in Unicode
}, 
// STRING descriptor 2 - Product ID
{	24,			// bLength
	0x03,			// bDescriptorType = string
	'M',0,'A',0,'X',0,'3',0,'4',0,'2',0,'0',0,'E',0,' ',0,
        'E',0,'n',0,'u',0,'m',0,' ',0,'C',0,'o',0,'d',0,'e',0
},

// STRING descriptor 3 - Serial Number ID
{       20,			// bLength
	0x03,			// bDescriptorType = string
	'S',0,				
	'/',0,
	'N',0,
	' ',0,
	'3',0,
	'4',0,
	'2',0,
	'0',0,
        'E',0,
}};

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



//---------------------------------------------------------------------------
// Copyright (C) 2005 Maxim Integrated Products, Inc. All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY,  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL MAXIM INTEGRATED PRODUCTS INC. BE LIABLE FOR ANY CLAIM, DAMAGES
// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//---------------------------------------------------------------------------
// EnumAppNote_BF1.C	2-9-06. Changes the include file "MAX3420E.h" to "MAX3420#-BF1.h".
//			To incorporate a bug fix that did not correctly issue a USB stall handshake
//
// This code demonstrates enumeration using the MAXIM MAX3420E USB Peripheral Controller.
// The code handles all host enumeration requests, as well as suspend/resume. The USB device
// implemented with this code reports itself as capable of signaling remote wakeup (RWU), and
// the code supports this operation by checking a RWU pushbutton while the bus is suspended.
//
//  The code requires pushbuttons on GPIN0 and GPIN2. Connect the other side of the pushbutton to GND
//    (No pullups necessary--they're inside the MAX3420E.)
//  The code supports 4 LEDS connected to the GPOUT pins. These are programmed as
//    active high, so the LED anodes go to the GPOUT pin and the cathodes go to GND.
//    Use series resistors of about 150 Ohms for VL=3.3V.
//            Pushbuttons                 LED's
//    GPIN0   Send Keystrokes   GPOUT0    Sending Keystrokes
//    GPIN1   ---               GPOUT1    Bus Reset
//    GPIN2   Remote Wakeup     GPOUT2    SUSPEND
//    GPIN3   ---               GPOUT3    Program Loop Active
//
//  The code runs fine without the LEDS or pushbutton (it enumerates)
//  but it requires the pushbuttons to function as a USB device and to signal remote wakeup.
//
//  After enumerating as a USB HID class device, the code implements a one-button keyboard
//  using the button attached to GPIN0. When this button is pressed the program types the
//  character string defined in "EnumApp_enum_data.h".
//
//  Because the I/O is implemented using MAX3420E IO pins, I/O code is portable across processors.
//
//  The code enables certain interrupts and the MAX3420E INT pin, but it does not assume any particular
//  uC or compiler for implementing vectored interrupts. It accomplishes this by polling the INT pin for
//  an active low level instead of having the INT pin trigger a particular uC hardware interrupt. This
//  makes the code as generic as possible since it does not assume any particular uC interrupt system
//  or compiler interrupt handler syntax.
//
//#include "MAX3420E_BF1.h"       // MAX3420E registers (rREGNAME), bits (bmBITNAME), and some handy macros
//#include "EnumApp_enum_data.h"  // HID keyboard enumeration data

typedef unsigned char BYTE;     // these save typing
typedef unsigned short WORD;

// function prototypes
void SPI_Init(void);            // Configure MAXQ2000 and MAX3420E IO pins for SPI
void Reset_MAX(void);           // Reset the MAX3420E
void wreg(BYTE r,BYTE v);       // Write a MAX3420E register byte
void wregAS(BYTE r,BYTE v);     // Same as 'wreg' but also set the ACKSTAT bit in the SPI command byte
BYTE rreg(BYTE r);              // Read a MAX3420E register byte
BYTE rregAS(BYTE r);            // Same as 'rreg' but also set the ACKSTAT bit
void readbytes(BYTE reg, BYTE N, BYTE *p);  // Read N MAX3420E FIFO bytes into the array p
void writebytes(BYTE reg, BYTE N, BYTE *p); // Write N MAX3420E FIFO bytes into the array p
BYTE MAX_Int_Pending(void);     // Poll the MAX3420E INT pin (set for active low level)

// USB functions
void std_request(void);
void class_request(void);
void vendor_request(void);
void send_descriptor(void);
void send_keystroke(BYTE);
void feature(BYTE);
void get_status(void);
void set_interface(void);
void get_interface(void);
void set_configuration(void);
void get_configuration(void);

// Application code
void do_SETUP(void);      // Handle a USB SETUP transfer
void do_IN3(void);        // Send keyboard characters over Endpoint 3-IN
void check_for_resume(void);
void service_irqs(void);
void initialize_MAX(void);

//Global variables
BYTE SUD[8];		// Local copy of the 8 setup data read from the MAX3420E SUDFIFO
BYTE msgidx,msglen;	// Text string in EnumApp_enum_data.h--index and length
BYTE configval;		// Set/Get_Configuration value
BYTE ep3stall;		// Flag for EP3 Stall, set by Set_Feature, reported back in Get_Status
BYTE interfacenum;      // Set/Get interface value
BYTE inhibit_send;	// Flag for the keyboard character send routine
BYTE RWU_enabled;       // Set by Set/Clear_Feature RWU request, sent back for Get_Status-RWU
BYTE Suspended;         // Tells the main loop to look for host resume and RWU pushbutton
WORD msec_timer;        // Count off time in the main loop
WORD blinktimer;        // Count milliseconds to blink the "loop active" light
BYTE send3zeros;        // EP3-IN function uses this to send HID (key up) codes between keystrokes
//
#define ENABLE_IRQS wreg(rEPIEN,(bmSUDAVIE+bmIN3BAVIE)); wreg(rUSBIEN,(bmURESIE+bmURESDNIE));
// Note: the SUSPEND IRQ will be enabled later, when the device is configured.
// This prevents repeated SUSPEND IRQ's

void initialize_MAX(void)
{
    ep3stall=0;			// EP3 inintially un-halted (no stall) (CH9 testing)
    msgidx = 0;			// start of KB Message[]
    msglen = sizeof(Message);     // so we can check for the end of the message
    inhibit_send = 0x01;		// 0 means send, 1 means inhibit sending
    send3zeros=1;
    msec_timer=0;
    blinktimer=0;
// software flags
    configval=0;                    // at pwr on OR bus reset we're unconfigured
    Suspended=0;
    RWU_enabled=0;                  // Set by host Set_Feature(enable RWU) request
//
    //SPI_Init();                     // set up MAXQ2000 to use its SPI port as a master
//
// Always set the FDUPSPI bit in the PINCTL register FIRST if you are using the SPI port in
// full duplex mode. This configures the port properly for subsequent SPI accesses.
//
    wreg(rPINCTL,(bmFDUPSPI+bmINTLEVEL+gpxSOF)); // MAX3420: SPI=full-duplex, INT=neg level, GPX=SOF
    Reset_MAX();
    wreg(rGPIO,0x00);                   // lites off (Active HIGH)
// This is a self-powered design, so the host could turn off Vbus while we are powered.
// Therefore set the VBGATE bit to have the MAX3420E automatically disconnect the D+
// pullup resistor in the absense of Vbus. Note: the VBCOMP pin must be connected to Vbus
// or pulled high for this code to work--a low on VBCOMP will prevent USB connection.
    wreg(rUSBCTL,(bmCONNECT+bmVBGATE)); // VBGATE=1 disconnects D+ pullup if host turns off VBUS
    ENABLE_IRQS
	//SPIUSB_INT_DDR &= ~(1 << SPIUSB_INT_PIN); // input
    wreg(rCPUCTL,bmIE);                 // Enable the INT pin
}

#define TWENTY_MSEC 14200           // adjust this constant for 20 msec button checks
#define BLINKTIME 25                // blink every 500 msec

// ************************************************************************************
// This endless loop checks for two high priority events (every time through the loop):
// 1. USB suspend ("Suspended" flag = 1). If suspended, checks for resume signaling.
// 2. A MAX3420E pending interrupt.
//
// Every 20 msec, it reads the "SEND" pushbutton. Every half second, it blinks
// the "Loop Active" light.
//
// *********************************** MAIN *******************************************

void runmax(void)
{
    initialize_MAX();
    while(1) {	// endless loop
        if(Suspended)
            check_for_resume();
        if (MAX_Int_Pending())
            service_irqs();
        msec_timer++;
        if(msec_timer==TWENTY_MSEC) {
            msec_timer=0;
            if((rreg(rGPIO) & 0x10) == 0) { // Check the pushbutton on GPI-0
                inhibit_send = 0x00;      // Tell the "do_IN3" function to send the text string
                L0_ON                     // Turn on the SEND light
            }
            blinktimer++;                 // blink the loop active light every half second
            if(blinktimer==BLINKTIME) {
                blinktimer=0;
                L3_BLINK
            }
        }// msec_timer==ONE_MSEC
    } // while(1)
}// main

void check_for_resume(void)
{
    if(rreg(rUSBIRQ) & bmBUSACTIRQ) {   // THE HOST RESUMED BUS TRAFFIC
        L2_OFF
        Suspended=0;                    // no longer suspended
    } else if(RWU_enabled) {            // Only if the host enabled RWU
        if((rreg(rGPIO)&0x40)==0) {     // See if the Remote Wakeup button was pressed
            L2_OFF                        // turn off suspend light
            Suspended=0;                  // no longer suspended
            SETBIT(rUSBCTL,bmSIGRWU)      // signal RWU
            while ((rreg(rUSBIRQ)&bmRWUDNIRQ)==0) ;	// spin until RWU signaling done
            CLRBIT(rUSBCTL,bmSIGRWU)      // remove the RESUME signal
            wreg(rUSBIRQ,bmRWUDNIRQ);     // clear the IRQ
            while((rreg(rGPIO)&0x40)==0) ;  // hang until RWU button released
            wreg(rUSBIRQ,bmBUSACTIRQ);    // wait for bus traffic -- clear the BUS Active IRQ
            while((rreg(rUSBIRQ) & bmBUSACTIRQ)==0) ; // & hang here until it's set again...
        }
    }
}
//

void service_irqs(void)
{
    BYTE itest1,itest2;
    itest1 = rreg(rEPIRQ);            // Check the EPIRQ bits
    itest2 = rreg(rUSBIRQ);           // Check the USBIRQ bits
    if(itest1 & bmSUDAVIRQ) {
        wreg(rEPIRQ,bmSUDAVIRQ);     // clear the SUDAV IRQ
        do_SETUP();
    }
    if(itest1 & bmIN3BAVIRQ) {        // Was an EP3-IN packet just dispatched to the host?
        do_IN3();                     // Yes--load another keystroke and arm the endpoint
    }                             // NOTE: don't clear the IN3BAVIRQ bit here--loading the EP3-IN byte
    // count register in the do_IN3() function does it.
    if((configval != 0) && (itest2&bmSUSPIRQ)) { // HOST suspended bus for 3 msec
        wreg(rUSBIRQ,(bmSUSPIRQ+bmBUSACTIRQ));  // clear the IRQ and bus activity IRQ
        L2_ON                         // turn on the SUSPEND light
        L3_OFF                        // turn off blinking light (in case it's on)
        Suspended=1;                  // signal the main loop
    }
    if(rreg(rUSBIRQ)& bmURESIRQ) {
        L1_ON                         // turn the BUS RESET light on
        L2_OFF                        // Suspend light off (if on)
        wreg(rUSBIRQ,bmURESIRQ);      // clear the IRQ
    }
    if(rreg(rUSBIRQ) & bmURESDNIRQ) {
        L1_OFF                        // turn the BUS RESET light off
        wreg(rUSBIRQ,bmURESDNIRQ);    // clear the IRQ bit
        Suspended=0;                  // in case we were suspended
        ENABLE_IRQS                   // ...because a bus reset clears the IE bits
    }
}

void do_SETUP(void)
{
    readbytes(rSUDFIFO,8,SUD);          // got a SETUP packet. Read 8 SETUP bytes
    switch(SUD[bmRequestType]&0x60) {   // Parse the SETUP packet. For request type, look only at b6&b5
    case 0x00:
        std_request();
        break;
    case 0x20:
        class_request();
        break;  // just a stub in this program
    case 0x40:
        vendor_request();
        break;  // just a stub in this program
    default:
        STALL_EP0                       // unrecognized request type
    }
}

void do_IN3(void)
{
    if (inhibit_send==0x01) {
        wreg(rEP3INFIFO,0);			// send the "keys up" code
        wreg(rEP3INFIFO,0);
        wreg(rEP3INFIFO,0);
    } else if (send3zeros==0x01) {                    // precede every keycode with the "no keys" code
        wreg(rEP3INFIFO,0);			// send the "keys up" code
        wreg(rEP3INFIFO,0);
        wreg(rEP3INFIFO,0);
        send3zeros=0;                           // next time through this function send the keycode
    } else {
        send3zeros=1;
        wreg(rEP3INFIFO,Message[msgidx++]);	// load the next keystroke (3 bytes)
        wreg(rEP3INFIFO,Message[msgidx++]);
        wreg(rEP3INFIFO,Message[msgidx++]);
        if(msgidx >= msglen) {                  // check for message wrap
            msgidx=0;
            L0_OFF
            inhibit_send=1;                     // send the string once per pushbutton press
        }
    }
    wreg(rEP3INBC,3);				// arm it
}

//*******************
void std_request(void)
{
    switch(SUD[bRequest]) {
    case	SR_GET_DESCRIPTOR:
        send_descriptor();
        break;
    case	SR_SET_FEATURE:
        feature(1);
        break;
    case	SR_CLEAR_FEATURE:
        feature(0);
        break;
    case	SR_GET_STATUS:
        get_status();
        break;
    case	SR_SET_INTERFACE:
        set_interface();
        break;
    case	SR_GET_INTERFACE:
        get_interface();
        break;
    case	SR_GET_CONFIGURATION:
        get_configuration();
        break;
    case	SR_SET_CONFIGURATION:
        set_configuration();
        break;
    case	SR_SET_ADDRESS:
        rregAS(rFNADDR);
        break;  // discard return value
    default:
        STALL_EP0
    }
}

//**************************
void set_configuration(void)
{
    configval=SUD[wValueL];           // Store the config value
    if(configval != 0)                // If we are configured,
        SETBIT(rUSBIEN,bmSUSPIE);       // start looking for SUSPEND interrupts
    rregAS(rFNADDR);                  // dummy read to set the ACKSTAT bit
}

void get_configuration(void)
{
    wreg(rEP0FIFO,configval);         // Send the config value
    wregAS(rEP0BC,1);
}

//**********************
void set_interface(void)	// All we accept are Interface=0 and AlternateSetting=0, otherwise send STALL
{
    BYTE dumval;
    if((SUD[wValueL]==0)		// wValueL=Alternate Setting index
       &&(SUD[wIndexL]==0))		// wIndexL=Interface index
        dumval=rregAS(rFNADDR);	// dummy read to set the ACKSTAT bit
    else STALL_EP0
    }

//**********************
void get_interface(void)	// Check for Interface=0, always report AlternateSetting=0
{
    if(SUD[wIndexL]==0) {	// wIndexL=Interface index
        wreg(rEP0FIFO,0);		// AS=0
        wregAS(rEP0BC,1);		// send one byte, ACKSTAT
    } else STALL_EP0
    }

//*******************
void get_status(void)
{
    BYTE testbyte;
    testbyte=SUD[bmRequestType];
    switch(testbyte) {
    case 0x80: 			// directed to DEVICE
        wreg(rEP0FIFO,(RWU_enabled+1));	// first byte is 000000rs where r=enabled for RWU and s=self-powered.
        wreg(rEP0FIFO,0x00);		// second byte is always 0
        wregAS(rEP0BC,2); 		// load byte count, arm the IN transfer, ACK the status stage of the CTL transfer
        break;
    case 0x81: 			// directed to INTERFACE
        wreg(rEP0FIFO,0x00);		// this one is easy--two zero bytes
        wreg(rEP0FIFO,0x00);
        wregAS(rEP0BC,2); 		// load byte count, arm the IN transfer, ACK the status stage of the CTL transfer
        break;
    case 0x82: 			// directed to ENDPOINT
        if(SUD[wIndexL]==0x83) {	// We only reported ep3, so it's the only one the host can stall IN3=83
            wreg(rEP0FIFO,ep3stall);	// first byte is 0000000h where h is the halt (stall) bit
            wreg(rEP0FIFO,0x00);		// second byte is always 0
            wregAS(rEP0BC,2); 		// load byte count, arm the IN transfer, ACK the status stage of the CTL transfer
            break;
        } else  STALL_EP0		// Host tried to stall an invalid endpoint (not 3)
        default:
        STALL_EP0		// don't recognize the request
    }
}

// **********************************************************************************************
// FUNCTION: Set/Get_Feature. Call as feature(1) for Set_Feature or feature(0) for Clear_Feature.
// There are two set/clear feature requests:
//	To a DEVICE: 	Remote Wakeup (RWU).
//  	To an ENDPOINT:	Stall (EP3 only for this app)
//
void feature(BYTE sc)
{
    BYTE mask;
    if((SUD[bmRequestType]==0x02)	// dir=h->p, recipient = ENDPOINT
       &&  (SUD[wValueL]==0x00)	// wValueL is feature selector, 00 is EP Halt
       &&  (SUD[wIndexL]==0x83)) {	// wIndexL is endpoint number IN3=83
        mask=rreg(rEPSTALLS);   // read existing bits
        if(sc==1) {             // set_feature
            mask += bmSTLEP3IN;       // Halt EP3IN
            ep3stall=1;
        } else {                    // clear_feature
            mask &= ~bmSTLEP3IN;      // UnHalt EP3IN
            ep3stall=0;
            wreg(rCLRTOGS,bmCTGEP3IN);  // clear the EP3 data toggle
        }
        wreg(rEPSTALLS,(mask|bmACKSTAT)); // Don't use wregAS for this--directly writing the ACKSTAT bit
    } else if ((SUD[bmRequestType]==0x00)	// dir=h->p, recipient = DEVICE
               &&  (SUD[wValueL]==0x01)) {	// wValueL is feature selector, 01 is Device_Remote_Wakeup
        RWU_enabled = sc<<1;	// =2 for set, =0 for clear feature. The shift puts it in the get_status bit position.
        rregAS(rFNADDR);		// dummy read to set ACKSTAT
    } else STALL_EP0
    }

//************************
void send_descriptor(void)
{
    WORD reqlen,sendlen,desclen;
    BYTE *pDdata;					// pointer to ROM Descriptor data to send
//
// NOTE This function assumes all descriptors are 64 or fewer bytes and can be sent in a single packet
//
    desclen = 0;					// check for zero as error condition (no case statements satisfied)
    reqlen = SUD[wLengthL] + 256*SUD[wLengthH];	// 16-bit
    switch (SUD[wValueH]) {		// wValueH is descriptor type
    case  GD_DEVICE:
        desclen = DD[0];	// descriptor length
        pDdata = DD;
        break;
    case  GD_CONFIGURATION:
        desclen = CD[2];	// Config descriptor includes interface, HID, report and ep descriptors
        pDdata = CD;
        break;
    case  GD_STRING:
        desclen = strDesc[SUD[wValueL]][0];   // wValueL=string index, array[0] is the length
        pDdata = strDesc[SUD[wValueL]];       // point to first array element
        break;
    case  GD_HID:
        desclen = CD[18];
        pDdata = &CD[18];
        break;
    case  GD_REPORT:
        desclen = CD[25];
        pDdata = RepD;
        break;
    }	// end switch on descriptor type
//
    if (desclen!=0) {                 // one of the case statements above filled in a value
        sendlen = (reqlen <= desclen) ? reqlen : desclen; // send the smaller of requested and avaiable
        writebytes(rEP0FIFO,sendlen,pDdata);
        wregAS(rEP0BC,sendlen);   // load EP0BC to arm the EP0-IN transfer & ACKSTAT
    } else STALL_EP0 // none of the descriptor types match
    }

void class_request(void)
{
    STALL_EP0
}

void vendor_request(void)
{
    STALL_EP0
}

// ******************** END of ENUMERATION CODE ********************
//
void Reset_MAX(void)
{
    BYTE dum;
    wreg(rUSBCTL,0x20);	// chip reset
    wreg(rUSBCTL,0x00);	// remove the reset
    do {                // Chip reset stops the oscillator. Wait for it to stabilize.
        dum=rreg(rUSBIRQ);
        dum &= bmOSCOKIRQ;
    } while (dum==0);
}
//
// ------------------------------------------------------------
// The code below customizes this app for the MAXQ2000
// microprocessor and the Rowley compiler. Only this
// section changes if you use a different uP and/or compiler.
// ------------------------------------------------------------
//
// The MAX3420E is wired to the MAXQ2000 as follows:
//
// MISO P57 (in)
// SCLK P56 (out)
// MOSI P55 (out)
// SS#  P54 (out)
// GPX  P53 (in)
// INT  P60 (in)

/*
sfrw SPIB =   0x53;      // This is how the Rowley compiler declares MAXQ SFR registers
sfrb SPICF =  0x163;
sfrb SPICK =  0x173;
sfrb CKCN =   0xe8;
sfrb PO5 =    0x11;
sfrb PD5 =    0x111;
sfrb PD6 =    0x121;
sfrb PI6 =    0xA1;
sfrb SPICN =  0x153;
*/
// Register SPICN bit masks
#define bmSPIEN 0x01
#define bmMSTM  0x02
#define bmSTBY  0x80

BYTE MAX_Int_Pending(void)
{
    return (BYTE)((SPIUSB_INT_PORT & (1 << SPIUSB_INT_PIN))==0);
}

/*
void SPI_Init(void)
{
// Set up the MAXQ2000 SPI port
  CKCN = 0x00;              // system clock divisor is 1
  SPI_UnselectAllSlaves();                     // SS# high
  PD5 |= 0x070;             // Set SPI pins (SCLK, MOSI, and SS#) as outputs
  PD5 &= ~0x088;            // Set SPI pins (MISO,GPX) as inputs
  PD6 &= ~0x01;             // Set P60 (INT) as input
  SPICK = 0x00;             // fastest SPI clock--div by 2
  SPICF = 0x00;             // mode(0,0), 8 bit data
  SPICN |= bmMSTM;          // Set SPI controller as master
  SPICN |= bmSPIEN;         // Enable the SPI controller
}
*/

void wreg(BYTE reg, BYTE dat)
{
    SPI_SelectSlave(SPIUSB);                     // Set SS# low
    SPI_MasterTransmit(reg+2);             // send the register number with the DIR bit (b1) set to WRITE
    SPI_MasterTransmit(dat);               // send the data
    SPI_UnselectAllSlaves();                     // set SS# high
}


// Write a MAX3410E register with the "ACK STATUS" bit set in the command byte
void wregAS(BYTE reg, BYTE dat)
{
    SPI_SelectSlave(SPIUSB);              // Set SS# low
    SPI_MasterTransmit(reg+3);            // reg number with DIR=1 (write) and ACKSTAT=1
    SPI_MasterTransmit(dat);               // send the data
    SPI_UnselectAllSlaves();              // set SS# high
}

// Read a register, return its value.
BYTE rreg(BYTE reg)
{
    BYTE dum;
    SPI_SelectSlave(SPIUSB);
    dum = SPI_MasterTransmit(reg);
    dum = SPI_MasterTransmit(0x00);
    SPI_UnselectAllSlaves();
    return(dum);
}

// Read a byte (as rreg), but also set the AckStat bit in the command byte.
BYTE rregAS(BYTE reg)
{
    BYTE dum;
    SPI_SelectSlave(SPIUSB);
    dum = SPI_MasterTransmit(reg+1);             // reg number w. dir=0 (IN) and ACKSTAT=1
    dum = SPI_MasterTransmit(0xFF);
    SPI_UnselectAllSlaves();
    return(dum);
}

void readbytes(BYTE reg, BYTE N, BYTE *p)
{
    BYTE j;
    SPI_SelectSlave(SPIUSB);
    j = SPI_MasterTransmit(reg);               // write bit b1=0 to command a read operation
    for(j=0; j<N; j++) {
        *p = SPI_MasterTransmit(0x00);              // store it in the data array
        p++;                    // bump the pointer
    }
    SPI_UnselectAllSlaves();
}
void writebytes(BYTE reg, BYTE N, BYTE *p)
{
    BYTE j,wd;
    SPI_SelectSlave(SPIUSB);
    SPI_MasterTransmit(reg+2);             // write bit b1=1 to command a write operation
    for(j=0; j<N; j++) {
        wd = *p;                // write the array value
        SPI_MasterTransmit(wd);
        p++;                    // bump the pointer
    }
    SPI_UnselectAllSlaves();
}
//
// Diagnostic Aid:
// Call this function from main() to verify operation of your SPI port.
//
void test_SPI(void)         // Use this to check your versions of the rreg and wreg functions
{
    BYTE j,wr,rd;
    //SPI_Init();                 // Configure and initialize the uP's SPI port
    wreg(rPINCTL,bmFDUPSPI);    // MAX3420: SPI=full-duplex
    wreg(rUSBCTL,bmCHIPRES);    // reset the MAX3420E
    wreg(rUSBCTL,0);            // remove the reset
    wr=0x01;                    // initial register write value
    for(j=0; j<8; j++) {
        wreg(rUSBIEN,wr);
        rd = rreg(rUSBIEN);
        wr <<= 1;       // Put a breakpoint here. Values of 'rd' should be 01,02,04,08,10,20,40,80
    }
}


