void USBEnableIRQ();
void USBInit();
void USBReset();
void USBSetup();
void USBWrite(char addr, char data);
char USBRead(char addr);
void USBWriteBuffer(char addr, char buffer[], uint8_t length);
void USBReadBuffer(char addr, char buffer[], uint8_t length);
void USBServeIRQ();
void USBSetConfig();
void USBGetConfig();
void USBSetIf();
void USBGetIf();
void USBGetStatus();
void USBFeature(uint8_t sc);
void do_IN3();
void USBChkResume();