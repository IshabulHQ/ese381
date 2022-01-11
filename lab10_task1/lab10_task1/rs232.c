//***************************************************************************
//
// File Name            : "rs232.c "
// Title                : rs232.c
// Date                 : 4/8/2020
// Version              : 1.0
// Target MCU           : SAML21J18B
// Target Hardware      ; DOG LCD
// Author               : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Contains functioon and variable definitions to setup RS232 communication 
// between the MCU and a terminal
//
// Warnings             :
// Restrictions         : none
// Algorithms           : none
// References           :
//
// Revision History     : Initial version
//
//
//**************************************************************************
#include "saml21j18b.h"




extern unsigned char* ARRAY_PORT_PINCFG1 = (unsigned char*)&REG_PORT_PINCFG1;
extern unsigned char* ARRAY_PORT_PMUX1 = (unsigned char*)&REG_PORT_PMUX1;


//***************************************************************************
//
// Function Name        : "UART4_init"
// Date                 : 4/8/2020
// Version              : 1.0
// Target MCU           : SAML21J18B
// Target Hardware      ; DOG LCD
// Author               : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Initializes the MCU's RS232 communication at SERCOM4 
// 9600 baud, LSB first, 8 bits, no parity bit, 1 stop bit 
// PB09 = Rx, PB08 = Tx
//
// Warnings             :
// Restrictions         : none
// Algorithms           : none
// References           :
//
// Revision History     : Initial version
//
//
//**************************************************************************

/* initialize UART4 to transmit at 9600 Baud */
extern void UART4_init(void){
	REG_GCLK_PCHCTRL22 = 0x00000040;				// SERCOM4 Core Clock enabled
	
	REG_SERCOM4_USART_CTRLA |= 1;					// Reset SERCOM4
	while(REG_SERCOM4_USART_SYNCBUSY & 1){}			// Wait for reset to finish
	
	REG_SERCOM4_USART_CTRLA = 0x40106004;			/* LSB first, async, no parity, 
	PAD[1] - Rx, PAD[0[] - Tx, BAURD uses fraction, 8x oversampling, internal clock */ 
	REG_SERCOM4_USART_CTRLB = 0x00030000;			// Enable Tx, one stop bit, 8 bit
	REG_SERCOM4_USART_BAUD = 52;					// 100000 / 8 / 9600 = 13.02
	REG_SERCOM4_USART_CTRLA |= 2;					// Enable SERCOM4
	while(REG_SERCOM4_USART_SYNCBUSY & 2){}			// Wait for enable to finish
		
	ARRAY_PORT_PINCFG1[8] |= 1;			// Allow pmux to set PB08 pin configuration 
	ARRAY_PORT_PINCFG1[9] |= 1;			// Allow pmux to set PB09 pin configuration 
	ARRAY_PORT_PMUX1[4] = 0x33;			// PB08 = TxD, PB09 = RxD 
}	

//***************************************************************************
//
// Function Name        : "UART4_write"
// Date                 : 4/8/2020
// Version              : 1.0
// Target MCU           : SAML21J18B
// Target Hardware      ; DOG LCD
// Author               : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Writes a sinngle 8-bit value to data register oof SERCOM4 
//
// Warnings             :
// Restrictions         : none
// Algorithms           : none
// References           :
//
// Revision History     : Initial version
//
//
//**************************************************************************
	
extern void UART4_write(char data){
		while(!(REG_SERCOM4_USART_INTFLAG & 1)){}	// Wait for data register to empty
		REG_SERCOM4_USART_DATA = data;				// Send a char 
		
}
	
//***************************************************************************
//
// Function Name        : "UART4_read"
// Date                 : 4/8/2020
// Version              : 1.0
// Target MCU           : SAML21J18B
// Target Hardware      ; DOG LCD
// Author               : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Returns a single 8-bit value read from SERCOM4's data register 
//
// Warnings             :
// Restrictions         : none
// Algorithms           : none
// References           :
//
// Revision History     : Initial version
//
//
//**************************************************************************
extern char UART4_read(void){
		while(!(REG_SERCOM4_USART_INTFLAG & 4)){}	// Wait until data is received
		return REG_SERCOM4_USART_DATA;				//  Read the data and return it
		
	
}