#include <limits.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "spi.h"
#include "mcp23s17.h"
#include "mcp3204.h"
#include "lcd.h"
#include "Joystick.h"
#include "AudioInput.h"
#include "message.h"
#include "nlms.h"

volatile uint16_t data[32] = {0x0000};
volatile uint8_t updatePending = 1;
volatile uint8_t lcdtypes[7] = {TYPE_GEAR, TYPE_RPM, TYPE_SPEED, TYPE_POS, TYPE_LAP, TYPE_LAPTIME, TYPE_DELTA};
volatile uint16_t prevButtonMask = 0;
volatile uint16_t buttonChange = 0;
volatile int16_t audioSample;

extern volatile LCDtext labels[14];

//volatile uint8_t lcdupdate[32] = {1};

/** LUFA Audio Class driver interface configuration and state information. This structure is
 *  passed to all Audio Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_Audio_Device_t Microphone_Audio_Interface =
	{
		.Config =
			{
				.StreamingInterfaceNumber = 1,

				.DataINEndpointNumber     = AUDIO_STREAM_EPNUM,
				.DataINEndpointSize       = AUDIO_STREAM_EPSIZE,
			},
	};

/** Current audio sampling frequency of the streaming audio endpoint. */
static uint32_t CurrentAudioSampleFrequency = 8000;

/** Audio channel noise floor */
volatile int16_t noisefloor[2];


/** Buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevJoystickHIDReportBuffer[sizeof(USB_JoystickReport_Data_t)];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Joystick_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = 2,

				.ReportINEndpointNumber       = JOYSTICK_EPNUM,
				.ReportINEndpointSize         = JOYSTICK_EPSIZE,
				.ReportINEndpointDoubleBank   = false,

				.PrevReportINBuffer           = PrevJoystickHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevJoystickHIDReportBuffer),
			},
	};


/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Joystick_HID_Interface);
	ConfigSuccess &= Audio_Device_ConfigureEndpoints(&Microphone_Audio_Interface);	

	USB_Device_EnableSOFEvents();
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	if (USB_ControlRequest.bmRequestType == 64 && USB_ControlRequest.bRequest < sizeof(data)) {
		if(data[USB_ControlRequest.bRequest] != USB_ControlRequest.wValue) {
			data[USB_ControlRequest.bRequest] = USB_ControlRequest.wValue;
			updatePending = 1;
		}
		
		Endpoint_ClearStatusStage();
	}
	else {
	
		HID_Device_ProcessControlRequest(&Joystick_HID_Interface);
		Audio_Device_ProcessControlRequest(&Microphone_Audio_Interface);
	}
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Joystick_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean true to force the sending of the report, false to let the library determine if it needs to be sent
 */
volatile uint8_t btn = 0;

bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	USB_JoystickReport_Data_t* JoystickReport = (USB_JoystickReport_Data_t*)ReportData;

	uint8_t row;
	
	JoystickReport->Button = 0x0000;

	for(row = 0; row < 4; row++) {
		GPIOWrite(0, (1 << row) ^ 0x0f);
		JoystickReport->Button |= ((GPIORead(0) >> 4) << (row << 2));
	}

	// save rising edge
	buttonChange = (prevButtonMask ^ JoystickReport->Button) & JoystickReport->Button;
	prevButtonMask = JoystickReport->Button;

	*ReportSize = sizeof(USB_JoystickReport_Data_t);
	return false;
}

/** ISR to handle the reloading of the data endpoint with the next sample. */
ISR(TIMER0_COMPA_vect, ISR_BLOCK)
{
	uint8_t PrevEndpoint = Endpoint_GetCurrentEndpoint();

	/* Check that the USB bus is ready for the next sample to write */
	if (Audio_Device_IsReadyForNextSample(&Microphone_Audio_Interface))
	{
		Audio_Device_WriteSample16(&Microphone_Audio_Interface, audioSample);
		//audioSample = 0;
		//audioSample = nlms((ADCGetValue(0) - noisefloor[0]), (ADCGetValue(1) - noisefloor[1])) << 4;
		//audioSample = ((int16_t)ADCGetValue(0) - noisefloor[0]) << 4;
	}

	Endpoint_SelectEndpoint(PrevEndpoint);
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	/* Sample reload timer initialization */
	TIMSK0  = (1 << OCIE0A);
	OCR0A   = ((F_CPU / 8 / CurrentAudioSampleFrequency) - 1);
	TCCR0A  = (1 << WGM01);  // CTC mode
	TCCR0B  = (1 << CS01);   // Fcpu/8 speed
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	/* Stop the sample reload timer */
	TCCR0B = 0;
}

/** Audio class driver callback for the setting and retrieval of streaming endpoint properties. This callback must be implemented
 *  in the user application to handle property manipulations on streaming audio endpoints.
 *
 *  When the DataLength parameter is NULL, this callback should only indicate whether the specified operation is valid for
 *  the given endpoint index, and should return as fast as possible. When non-NULL, this value may be altered for GET operations
 *  to indicate the size of the retreived data.
 *
 *  \note The length of the retrieved data stored into the Data buffer on GET operations should not exceed the initial value
 *        of the \c DataLength parameter.
 *
 *  \param[in,out] AudioInterfaceInfo  Pointer to a structure containing an Audio Class configuration and state.
 *  \param[in]     EndpointProperty    Property of the endpoint to get or set, a value from Audio_ClassRequests_t.
 *  \param[in]     EndpointAddress     Address of the streaming endpoint whose property is being referenced.
 *  \param[in]     EndpointControl     Parameter of the endpoint to get or set, a value from Audio_EndpointControls_t.
 *  \param[in,out] DataLength          For SET operations, the length of the parameter data to set. For GET operations, the maximum
 *                                     length of the retrieved data. When NULL, the function should return whether the given property
 *                                     and parameter is valid for the requested endpoint without reading or modifying the Data buffer.
 *  \param[in,out] Data                Pointer to a location where the parameter data is stored for SET operations, or where
 *                                     the retrieved data is to be stored for GET operations.
 *
 *  \return Boolean \c true if the property get/set was successful, \c false otherwise
 */
bool CALLBACK_Audio_Device_GetSetEndpointProperty(USB_ClassInfo_Audio_Device_t* const AudioInterfaceInfo,
                                                  const uint8_t EndpointProperty,
                                                  const uint8_t EndpointAddress,
                                                  const uint8_t EndpointControl,
                                                  uint16_t* const DataLength,
                                                  uint8_t* Data)
{
	/* Check the requested endpoint to see if a supported endpoint is being manipulated */
	if (EndpointAddress == (ENDPOINT_DIR_IN | Microphone_Audio_Interface.Config.DataINEndpointNumber))
	{
		/* Check the requested control to see if a supported control is being manipulated */
		if (EndpointControl == AUDIO_EPCONTROL_SamplingFreq)
		{
			switch (EndpointProperty)
			{
				case AUDIO_REQ_SetCurrent:
					/* Check if we are just testing for a valid property, or actually adjusting it */
					if (DataLength != NULL)
					{
						/* Set the new sampling frequency to the value given by the host */
						CurrentAudioSampleFrequency = (((uint32_t)Data[2] << 16) | ((uint32_t)Data[1] << 8) | (uint32_t)Data[0]);

						/* Adjust sample reload timer to the new frequency */
						OCR0A = ((F_CPU / 8 / CurrentAudioSampleFrequency) - 1);
					}

					return true;
				case AUDIO_REQ_GetCurrent:
					/* Check if we are just testing for a valid property, or actually reading it */
					if (DataLength != NULL)
					{
						*DataLength = 3;

						Data[2] = (CurrentAudioSampleFrequency >> 16);
						Data[1] = (CurrentAudioSampleFrequency >> 8);
						Data[0] = (CurrentAudioSampleFrequency &  0xFF);
					}

					return true;
			}
		}
	}

	return false;
}

/** Audio class driver callback for the setting and retrieval of streaming interface properties. This callback must be implemented
 *  in the user application to handle property manipulations on streaming audio interfaces.
 *
 *  When the DataLength parameter is NULL, this callback should only indicate whether the specified operation is valid for
 *  the given entity and should return as fast as possible. When non-NULL, this value may be altered for GET operations
 *  to indicate the size of the retreived data.
 *
 *  \note The length of the retrieved data stored into the Data buffer on GET operations should not exceed the initial value
 *        of the \c DataLength parameter.
 *
 *  \param[in,out] AudioInterfaceInfo  Pointer to a structure containing an Audio Class configuration and state.
 *  \param[in]     Property            Property of the interface to get or set, a value from \ref Audio_ClassRequests_t.
 *  \param[in]     EntityAddress       Address of the audio entity whose property is being referenced.
 *  \param[in]     Parameter           Parameter of the entity to get or set, specific to each type of entity (see USB Audio specification).
 *  \param[in,out] DataLength          For SET operations, the length of the parameter data to set. For GET operations, the maximum
 *                                     length of the retrieved data. When NULL, the function should return whether the given property
 *                                     and parameter is valid for the requested endpoint without reading or modifying the Data buffer.
 *  \param[in,out] Data                Pointer to a location where the parameter data is stored for SET operations, or where
 *                                     the retrieved data is to be stored for GET operations.
 *
 *  \return Boolean \c true if the property GET/SET was successful, \c false otherwise
 */
bool CALLBACK_Audio_Device_GetSetInterfaceProperty(USB_ClassInfo_Audio_Device_t* const AudioInterfaceInfo,
                                                   const uint8_t Property,
                                                   const uint8_t EntityAddress,
                                                   const uint16_t Parameter,
                                                   uint16_t* const DataLength,
                                                   uint8_t* Data) 
{
	/* No audio interface entities in the device descriptor, thus no properties to get or set. */
	return false;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
}

volatile uint8_t d = 0;

void ShiftLedsUpdate(void) {
	GPIOWrite(1, data[TYPE_SHIFTLEDS] & 0x7f);
}

int main(void) {

	// USB
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	USB_Init();

	// SPI
	SPI_MasterInit();

	// ADC
	ADCinit();

	// GPIO
	GPIOInit();

	// LCD
	LCDinit();

	// Timer 1
	DDRB |= 1 << PB7;
	//PORTB |= 1 << PB7;
	
    // PWM for backlight
	TCCR1A |= (1 << WGM11)|(1 << WGM10)|(1 << COM1C1)|(1 << COM1C0);
    TCCR1B |= (1 << WGM12)|(0 << WGM13)|(0 << CS12)|(1 << CS11)|(0 << CS10); // prescaling 64
	data[TYPE_BACKLIGHT] = 1023;
    OCR1C = data[TYPE_BACKLIGHT];
	
	sei();
	for (;;)
	{
		Audio_Device_USBTask(&Microphone_Audio_Interface);
		HID_Device_USBTask(&Joystick_HID_Interface);
		USB_USBTask();

		if(buttonChange) {
			// catch button presses for internal functions

			// first button rotates top row
			if(buttonChange & (1 << 0)) {
				switch(lcdtypes[2]) {
					case TYPE_SPEED:
						lcdtypes[1] = TYPE_POS;
						lcdtypes[2] = TYPE_RPM;
						break;
					case TYPE_RPM:
						lcdtypes[1] = TYPE_SPEED;
						lcdtypes[2] = TYPE_POS;
						break;
					default:
						lcdtypes[1] = TYPE_RPM;
						lcdtypes[2] = TYPE_SPEED;
				}
			}
			// second button rotates second row


			// refresh everything
			uint8_t i;
			for(i = 0; i < sizeof(labels)/sizeof(LCDtext); i++)
				SET_REDRAW(labels[i]);

			updatePending = 1;
		}

		if(updatePending) {
			uint8_t i;
			for(i=0; i < sizeof(lcdtypes); i++)
				LCDProcessMessage(i << 1, lcdtypes[i], data[lcdtypes[i]]);

			LCDupdate();
			ShiftLedsUpdate();
			OCR1C = data[TYPE_BACKLIGHT];
			updatePending = 0;
		}

		audioSample = nlms((ADCGetValue(0) - noisefloor[0]), (ADCGetValue(1) - noisefloor[1]));
	}
}
