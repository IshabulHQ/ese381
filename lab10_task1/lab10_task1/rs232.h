//***************************************************************************
//
// File Name            : "rs232.h "
// Title                : rs232.h
// Date                 : 4/8/2020
// Version              : 1.0
// Target MCU           : SAML21J18B
// Target Hardware      ; DOG LCD
// Author               : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Include file containing function proototypes and variable declarations
// used to initialize the MCU's RS232 communication 
// Functions and variables defined in rs232.c
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


#ifndef RS232_H_
#define RS232_H_
  
  // Functionn and variable declarations
void UART4_init(void);
void UART4_write(char data);
char UART4_read(void);
unsigned char* ARRAY_PORT_PINCFG1; 
unsigned char* ARRAY_PORT_PMUX1; 



#endif /* RS232_H_ */