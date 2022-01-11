//***************************************************************************
//
// File Name            : "lcd.h "
// Title                : lcd.h
// Date                 : 4/8/2020
// Version              : 1.0
// Target MCU           : SAML21J18B
// Target Hardware      ; DOG LCD
// Author               : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Include file containing function prototypes and variable declarations
// which are defined in lcd.c
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
 



#ifndef LCD_H_
#define LCD_H_


#include <stdio.h>
#include "saml21j18b.h"

//variable declarations and function prototypes for C file
unsigned char* ARRAY_PORT_PINCFG0;
unsigned char* ARRAY_PORT_PMUX0;

char dsp_buff_1[17], dsp_buff_2[17], dsp_buff_3[17];



void delay_30us (void);
void v_delay (signed char inner, signed char outer);
void delay_40us (void);
void init_spi_lcd (void);
void lcd_spi_transmit_CMD (char CMD);
void lcd_spi_transmit_DATA (char data);
void init_lcd_dog (void);
void update_lcd_dog (void);
void clr_dsp_buff (void);





#endif /* LCD_H_ */