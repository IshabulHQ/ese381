//***************************************************************************
//
// File Name            : "lcd.c "
// Title                : "lcd.c
// Date                 : 4/8/2020
// Version              : 1.0
// Target MCU           : SAML21J18B
// Target Hardware      ; DOG LCD
// Author               : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Contains function and variable definitions used to intialize and perform 
// SPI communicatioonn between the MCU and DOG LCD
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


#include <stdio.h>
#include "saml21j18b.h"




#define freq 2000000

extern unsigned char* ARRAY_PORT_PINCFG0 = (unsigned char*)&REG_PORT_PINCFG0;
extern unsigned char* ARRAY_PORT_PMUX0 = (unsigned char*)&REG_PORT_PMUX0;

extern char dsp_buff_1[17], dsp_buff_2[17], dsp_buff_3[17];





extern void delay_30us (void) {
	int clock_factor = freq/1000000;				// delay loop from i=0 to i<6 is 30us long at 1 MHz.
	for (int i = 0; i < 6*clock_factor; i++) {		// loop end value is adjusted based on frequency macro constant.
		__asm("nop");
	}
}

extern void v_delay (signed char inner, signed char outer) {
	delay_30us();
	inner--;
	if (inner != 0) {
		v_delay(inner, outer);
	}
	outer--;
	if (outer != 0 ) {
		v_delay(0,outer);
	}
}

extern void delay_40ms (void) {
	/* 
	v_delay(0,4);		
	//call v_delay function with inner loop variable=0 and outer loop variable=4

	*/ 
	for(int j = 0; j <= 1240; j++){
		delay_30us(); 
	}
	
}

//***************************************************************************
//
// Function Name        : "init_spi_lcd"
// Date                 : 4/8/2020
// Version              : 1.0
// Target MCU           : SAML21J18B
// Target Hardware      ; DOG LCD
// Author               : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Initializzes the MCU's SPI communication to the DOGG LCD at SERCOM1. 
// PA16 = MOSI, PA17 = SCK, PA18 = /SS, PA19 = MISO (noot used), PB06 = RS
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

extern void init_spi_lcd (void) {
	REG_GCLK_PCHCTRL19 = 0x00000040;	/* SERCOM1 core clock not enabled by default */

	ARRAY_PORT_PINCFG0[16] |= 1;    /* allow pmux to set PA16 pin configuration */
	ARRAY_PORT_PINCFG0[17] |= 1;    /* allow pmux to set PA17 pin configuration */
	ARRAY_PORT_PINCFG0[18] |= 1;    /* allow pmux to set PA18 pin configuration */
	ARRAY_PORT_PINCFG0[19] |= 1;    /* allow pmux to set PB06 pin configuration */
	ARRAY_PORT_PMUX0[8] = 0x22;     /* PA16 = MOSI, PA17 = SCK */
	ARRAY_PORT_PMUX0[9] = 0x22;     /* PA18 = SS,   PA19 = MISO */

	REG_PORT_DIRSET1 = 0x40;		/* RS output */ 
	
	REG_SERCOM1_SPI_CTRLA = 1;              /* reset SERCOM1 */
	while (REG_SERCOM1_SPI_CTRLA & 1) {}    /* wait for reset to complete */
	
	REG_SERCOM1_SPI_CTRLA = 0x3030000C;     /* MISO-3, MOSI-0, SCK-1, SS-2, CPOL=1, CPHA=1 */
	REG_SERCOM1_SPI_CTRLB = 0x00002000;     /* Master SS, 8-bit */
	REG_SERCOM1_SPI_BAUD = 0;               /* SPI clock is 4MHz/2 = 2MzHz */
	REG_SERCOM1_SPI_CTRLA |= 2;             /* enable SERCOM1 */
}

//***************************************************************************
//
// Function Name        : "lcd_spi_transmit_CMD"
// Date                 : 4/8/2020
// Version              : 1.0
// Target MCU           : SAML21J18B
// Target Hardware      ; DOG LCD
// Author               : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Sends a single 8 - bit value to the DOG LCD. Value seen as a command
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

extern void lcd_spi_transmit_CMD (char CMD) {
	REG_PORT_OUTCLR1 = 0x00000040;					// RS = 0 -> command, /SS = 0 -> selected
	while(!(REG_SERCOM1_SPI_INTFLAG & 1)) {}		/* wait until Tx ready */
	REG_SERCOM1_SPI_DATA = CMD;						/* send data byte */
	REG_PORT_OUTSET1 = 0x00040000;					// /SS = 1 -> deselected
}

//***************************************************************************
//
// Function Name        : "lcd_spi_transmit Data"
// Date                 : 4/8/2020
// Version              : 1.0
// Target MCU           : SAML21J18B
// Target Hardware      ; DOG LCD
// Author               : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Sends a single 8-bit value to the DOG LCD> Value seen as data to be 
// displayed by the LCD
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
extern void lcd_spi_transmit_DATA (char data) {
	REG_PORT_OUTSET1 = 0x00000040;					// RS = 1 -> data
	REG_PORT_OUTCLR1 = 0x00040000;					// /SS = 0 -> selected
	while(!(REG_SERCOM1_SPI_INTFLAG & 1)) {}		/* wait until Tx ready */
	REG_SERCOM1_SPI_DATA = data;					/* send data byte */
	REG_PORT_OUTSET1 = 0x00040000;					// /SS = 1 -> deselected
}

//***************************************************************************
//
// Function Name        : "init_lcd_dog"
// Date                 : 4/8/2020
// Version              : 1.0
// Target MCU           : SAML21J18B
// Target Hardware      ; DOG LCD
// Author               : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Configures and initializes DOG LCD and MCU for SPI coommunication 
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
extern void init_lcd_dog (void) {
	init_spi_lcd();			// initialize SPI ports for LCD DOG
	delay_40ms();
	
	//function set 1
	lcd_spi_transmit_CMD(0x39);		//send function set 1
	delay_30us();
	
	//function set 2
	lcd_spi_transmit_CMD(0x39);		//send function set 2
	delay_30us();
	
	//set bias value
	lcd_spi_transmit_CMD(0x1E);		//set bias value
	delay_30us();
	
	//power control
	lcd_spi_transmit_CMD(0x55);		//configure for 3.3 V
	delay_30us();
	
	//follower control
	lcd_spi_transmit_CMD(0x6C);		//follower mode on
	delay_40ms();
	
	//contrast set
	lcd_spi_transmit_CMD(0x7F);		//configure for 3.3 V
	delay_30us();
	
	//display on
	lcd_spi_transmit_CMD(0x0C);		//display on, cursor off, blink off
	delay_30us();
	
	//clear display
	lcd_spi_transmit_CMD(0x01);		//clear display, cursor home
	delay_30us();
	
	//entry mode
	lcd_spi_transmit_CMD(0x06);		//clear display, cursor home
	delay_30us();
}

//***************************************************************************
//
// Function Name        : "update_lcd_dog"
// Date                 : 4/8/2020
// Version              : 1.0
// Target MCU           : SAML21J18B
// Target Hardware      ; DOG LCD
// Author               : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Updates the LCD display with the current value in dsp_buff_1, dsp_buff_2
// and dsp_buff_3
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

extern void update_lcd_dog (void) {
	char i;							//initialize loop variable
	init_spi_lcd();					//initialize SPI ports
	
	//send line 1 to LCD
	lcd_spi_transmit_CMD(0x80);		//initialize DDRAM addr-ctr
	delay_30us();
	
	for (i=0; i<=15; i++) {
		lcd_spi_transmit_DATA(dsp_buff_1[i]);		//send buff 1
		delay_30us();
	}
	
	//send line 2 to LCD
	lcd_spi_transmit_CMD(0x90);		//initialize DDRAM addr-ctr
	delay_30us();
	
	for (i=0; i<=15; i++) {
		lcd_spi_transmit_DATA(dsp_buff_2[i]);		//send buff 2
		delay_30us();
	}
	
	//send line 3 to LCD
	lcd_spi_transmit_CMD(0xA0);		//initialize DDRAM addr-ctr
	delay_30us();
	
	for (i=0; i<=15; i++) {
		lcd_spi_transmit_DATA(dsp_buff_3[i]);		//send buff 3
		delay_30us();
	}
}

//***************************************************************************
//
// Function Name        : "clr_dsp_buff"
// Date                 : 4/8/2020
// Version              : 1.0
// Target MCU           : SAML21J18B
// Target Hardware      ; DOG LCD
// Author               : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Clears LCD by filling display buffers with all spaces and calling the 
// update_lcd_dog function 
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
extern void clr_dsp_buff(void) {
	sprintf(dsp_buff_1, "                ");	//loads display buffers with all spaces
	sprintf(dsp_buff_2, "                ");
	sprintf(dsp_buff_3, "                ");
	update_lcd_dog();
}
