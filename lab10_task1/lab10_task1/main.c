//***************************************************************************
//
// File Name : "BME680 Sensor API dt2"
// Title : BME680 Sensor API dt2
// Date : 5/8/20
// Version : 1.0
// Target MCU : SAML21J18B
// Target Hardware ; DOG LCD, BME680
// Author : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Utilizes the BME680's API to convert it's ADC's readings into actual
// values to be displayed on the DOG LCD and TeraTerm.
//
// Warnings :
// Restrictions : none
// Algorithms : none
// References :
//
// Revision History : Initial version
//
//
//**************************************************************************
#include "saml21j18b.h"
#include "bme680.h"
#include "bme680_defs.h"
#include "lcd.h"
#include "rs232.h"
#include "sys_support.h"

uint8_t status, id; //declare chip id and page number global variables

//function prototypes
void init_spi_MCU (void);
static void init_spi_BME680 (void);
static void init_BME680(void);
void user_delay_ms (uint32_t period);
static uint8_t spi_transfer (uint8_t data);
int8_t user_spi_read (uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
int8_t user_spi_write (uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
int main(void) {
	int KeyPress=0x04, Last_KeyPress, count=0;
	uint16_t set_required_settings, meas_period;
	//initialize SERCOM4 for RS232 communication and SERCOM1 for the LCD and BME680
	UART4_init();
	init_lcd_dog();
	init_spi_BME680();
	init_BME680();
	struct bme680_dev gas_sensor; //create instance of bme680_dev named gas_sensor
	gas_sensor.dev_id = 0; //fill in various parameters for gas_sensor
	gas_sensor.intf = BME680_SPI_INTF;
	gas_sensor.read = user_spi_read;
	gas_sensor.write = user_spi_write;
	gas_sensor.delay_ms = user_delay_ms;
	gas_sensor.amb_temp = 25;
	int8_t rslt = BME680_OK;
	rslt = bme680_init(&gas_sensor);
	//set the temperature, pressure and humidity oversampling. set IIR filter size
	gas_sensor.tph_sett.os_hum = BME680_OS_2X;
	gas_sensor.tph_sett.os_pres = BME680_OS_4X;
	gas_sensor.tph_sett.os_temp = BME680_OS_8X;
	gas_sensor.tph_sett.filter = BME680_FILTER_SIZE_3;
	//enable gas measurements and configure gas heat plate temperature and heating duration
	gas_sensor.gas_sett.run_gas = BME680_ENABLE_GAS_MEAS;
	gas_sensor.gas_sett.heatr_temp = 320;
	gas_sensor.gas_sett.heatr_dur = 150;
	//put BME680 into forced mode
	gas_sensor.power_mode = BME680_FORCED_MODE;
	//configure all the temperature, pressure, humidity and gas settings
	set_required_settings = BME680_OST_SEL | BME680_OSP_SEL | BME680_OSH_SEL | BME680_FILTER_SEL |
	BME680_GAS_SENSOR_SEL;
	rslt = bme680_set_sensor_settings(set_required_settings, &gas_sensor);
	rslt = bme680_set_sensor_mode(&gas_sensor);
	bme680_get_profile_dur(&meas_period, &gas_sensor);
	struct bme680_field_data data; //create instance of bme680_field_data name 'data'
	while (1) {
		user_delay_ms(meas_period); //delay for meas_period ms
		rslt = bme680_get_sensor_data(&data, &gas_sensor); //read sensor measurements and store in data
		init_spi_lcd(); //initialize spi for lcd before transactions
		Last_KeyPress = KeyPress; //set last key press equal to current key press
		KeyPress = REG_PORT_IN0 & 0x04; //mask pushbutton value onto current key press
		if (Last_KeyPress != KeyPress) { //if last and current key value no equal
			if (KeyPress == 0x04) { //and if key is not held down, increment counter
				count++;
			}
		}
		if (count % 2 == 0) { //if count is even display temperature and pressure
			sprintf(dsp_buff_1, "T: %.2f degC", data.temperature / 100.0f);
			sprintf(dsp_buff_2, "P: %.2f hPa ", data.pressure / 100.0f);
			update_lcd_dog();
		}
		else { //if count is odd display humidity and gas resistance
			sprintf(dsp_buff_1, "H: %.2f %%rH", data.humidity / 1000.0f);
			if (data.status & BME680_GASM_VALID_MSK) {
				sprintf(dsp_buff_2, "G: %ld ohms ", data.gas_resistance);
			}
			update_lcd_dog();
		}
		//print values to TeraTerm
		printf("T: %.2f degC, P: %.2f hPa, H: %.2f %%rH", data.temperature / 100.0f, data.pressure / 100.0f,
		data.humidity / 1000.0f);
		if (data.status & BME680_GASM_VALID_MSK) {
			printf(", G: %ld ohms", data.gas_resistance);
		}
		printf("\r\n");
		init_spi_BME680(); //initialize spi for BME680 before transactions
		if (gas_sensor.power_mode == BME680_FORCED_MODE) {
			rslt = bme680_set_sensor_mode(&gas_sensor);
		}
	}
}
//***************************************************************************
//
// Function Name : "init_spi_BME680"
// Date : 5/9/20
// Version : 1.0
// Target MCU : SAML21J18B
// Target Hardware ; BME680
// Author : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Configures the SAML21J18B's SERCOM1 for SPI communication with the
// BME680. PA16 = MOSI, PA17 = SCK, PA19 = MISO, PB07 = CS.
//
// Warnings : none
// Restrictions : none
// Algorithms : none
// References : none
//
// Revision History : Initial version
//
//**************************************************************************
static void init_spi_BME680 (void) {
	REG_GCLK_PCHCTRL19 = 0x00000040; /* SERCOM1 core clock not enabled by default */
	ARRAY_PORT_PINCFG0[16] |= 1; /* allow pmux to set PA16 pin configuration */
	ARRAY_PORT_PINCFG0[17] |= 1; /* allow pmux to set PA17 pin configuration */
	ARRAY_PORT_PINCFG0[18] |= 1; /* allow pmux to set PA18 pin configuration */
	ARRAY_PORT_PINCFG0[19] |= 1; /* allow pmux to set PA19 pin configuration */
	ARRAY_PORT_PMUX0[8] = 0x22; /* PA16 = MOSI, PA17 = SCK */
	ARRAY_PORT_PMUX0[9] = 0x22; /* PA18 = SS, PA19 = MISO */
	REG_PORT_DIRSET1 = 0x80; /* PB07 = CS for BME680 */
	REG_PORT_DIRCLR0 = 0x04; //PA02 input for SW0
	ARRAY_PORT_PINCFG0[2] |= 6; //enable PA02 with pull
	REG_PORT_OUTSET0 = 0x04; //make PA02 pull-up
	REG_SERCOM1_SPI_CTRLA = 1; /* reset SERCOM1 */
	while (REG_SERCOM1_SPI_CTRLA & 1) {} /* wait for reset to complete */
	REG_SERCOM1_SPI_CTRLA = 0x3030000C; /* MISO-3, MOSI-0, SCK-1, SS-2, CPOL=1, CPHA=1 */
	REG_SERCOM1_SPI_CTRLB = 0x00020000; /* Master SS, 8-bit, receiver enabled */
	REG_SERCOM1_SPI_BAUD = 0; /* SPI clock is 4MHz/2 = 2MzHz */
	REG_SERCOM1_SPI_CTRLA |= 2; /* enable SERCOM1 */
}
//***************************************************************************
//
// Function Name : "init_BME680"
// Date : 5/9/20
// Version : 1.0
// Target MCU : SAML21J18B
// Target Hardware ; BME680
// Author : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Software resets the BME680 and reads its memory map page and status
// register. Sets the memory map to page 1 so the BME680 is ready for
// configuration after this function.
//
// Warnings : none
// Restrictions : none
// Algorithms : none
// References : none
//
// Revision History : Initial version
//
//**************************************************************************
static void init_BME680
(void) {
	uint8_t write_data = 0xB6;
	REG_PORT_DIRSET1 |= 0x80;
	user_spi_write (0, 0x60, &write_data, 1); //software reset BME680
	write_data = 0x00;
	user_spi_write(0, 0x73, &write_data, 1); //switch to page 0 of memory map
	user_spi_read(0, 0x73, &status, 1); //read status register
	status >>= 4;
	user_spi_read(0, 0x50, &id, 1); //read id register
	write_data = 0x10;
	user_spi_write(0, 0x73, &write_data, 1); //switch to page 1 of memory map
	user_spi_read(0, 0x73, &status, 1); //read status register
	status >>= 4;
}
//***************************************************************************
//
// Function Name : "user_delay_ms"
// Date : 5/9/20
// Version : 1.0
// Target MCU : SAML21J18B
// Target Hardware ; N/A
// Author : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Delays the system by a 'period' number of ms.
//
// Warnings : none
// Restrictions : none
// Algorithms : none
// References : none
//
// Revision History : Initial version
//
//**************************************************************************
void user_delay_ms (uint32_t period) {
	for (int i = 0; i < 170*period; i++) { //based off of 30us delay in DOGM163W_A_SERCOM1.c
		__asm("nop"); //delay by period ms
	}
}
//***************************************************************************
//
// Function Name : "spi_transfer"
// Date : 5/9/20
// Version : 1.0
// Target MCU : SAML21J18B
// Target Hardware ; BME680
// Author : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// Transmits an unsigned integer byte 'data' through SERCOM1's SPI data
// register, then receives a transmission through SERCOM1's SPI data register
// and returns the received unsigned integer byte.
//
// Warnings : none
// Restrictions : none
// Algorithms : none
// References : none
//
// Revision History : Initial version
//
//**************************************************************************
static uint8_t spi_transfer (uint8_t data) {
	uint8_t Rx_data;
	while(!(REG_SERCOM1_SPI_INTFLAG & 1)) {} //wait until Tx ready
	REG_SERCOM1_SPI_DATA = data; //send data byte
	while(!(REG_SERCOM1_SPI_INTFLAG & 2)) {} //wait until transmit is complete
	while(!(REG_SERCOM1_SPI_INTFLAG & 4)) {} //wait until receive is complete
	Rx_data = REG_SERCOM1_SPI_DATA; //read data register
	return Rx_data;
}
//***************************************************************************
//
// Function Name : "user_spi_read"
// Date : 5/9/20
// Version : 1.0
// Target MCU : SAML21J18B
// Target Hardware ; BME680
// Author : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// This function is passed 4 parameters, only reg_addr, *reg_data and len are
// used in this definition. The MS bit of reg_addr is masked to a '1' in order
// to indicate a read transaction to the BME680. The spi_transfer function
// is first called to pass the register address to be read. It is then called
// a second time to receive the register's data and update *reg_data. If
// len > 1, the register address to be read is auto-incremented and the address
// of reg_data is incremented as well to allow for multiple read transactions.
//
// Warnings : none
// Restrictions : none
// Algorithms : none
// References : none
//
// Revision History : Initial version
//
//**************************************************************************
int8_t user_spi_read (uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
	int8_t rslt = 0; //return 0 for success, non-zero for failure
	reg_addr |= 0x80; //mask bit 7 to a '1' for a read transaction
	REG_PORT_OUTCLR1 &= 0x80; //CS = 0 -> BME680 selected
	spi_transfer(reg_addr); //send control byte with register address
	while (len--) {
		*reg_data = spi_transfer(0);
		reg_data++;
	}
	REG_PORT_OUTSET1 |= 0x80; //CS = 1 -> BME680 unselected
	return rslt;
}
//***************************************************************************
//
// Function Name : "user_spi_write"
// Date : 5/9/20
// Version : 1.0
// Target MCU : SAML21J18B
// Target Hardware ; BME680
// Author : Brandon Cheung, Ishabul Haque
// DESCRIPTION
// This function is passed 4 parameters, only reg_addr, *reg_data and len are
// used in this definition. The spi_transfer function is first called to pass
// the register address to be written to. spi_transfer is then called a second
// time to write *reg_data to the register. If len > 1, the register address
// to be written to is auto-incremented and reg_data is incremented as well
// to allow for multiple write transactions.
//
// Warnings : none
// Restrictions : none
// Algorithms : none
// References : none
//re
// Revision History : Initial version
//
//**************************************************************************
int8_t user_spi_write (uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
	int8_t rslt = 0;
	REG_PORT_OUTCLR1 &= 0x80; //CS = 0 -> BME680 selected
	spi_transfer(reg_addr); //send control byte with register address
	while (len--) {
		spi_transfer(*reg_data);
		reg_data++;
	}
	REG_PORT_OUTSET1 |= 0x80; //CS = 1 -> BME680 unselected
	return rslt;
}
