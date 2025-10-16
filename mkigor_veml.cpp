/**
*	@brief		Arduino Library for sensor light VEML7700.
*	@author		Igor Mkprog, mkprogigor@gmail.com
*	@version	V1.0	@date	16.10.2025
*
*	@remarks	Glossary, abbreviations used in the module. Name has small or capital letters ("camelCase"),
*	and consist only 2 or 1 symbol '_', that divede it in => prefix + name + suffix.
*	prefix: 
*		gv_*	- Global Variable;
*		lv_*	- Local Variable (live inside statement);
*		cl_*	- CLass;
*		cd_*	- Class Definition;
*		cgv_*	- Class public (Global) member (Variable);
*		clv_*	- Class private (Local) member (Variable);
*		cgf_*	- Class public (Global) metod (Function), not need, no usefull, becouse we see parenthesis => ();
*		clf_*	- Class private (Local) metod (Function);
*		lp_*	- in function, local parameter.
*	suffix:
*		*_stru	- [like *_t] as usual, point to the type.
	example:	- prefix_nameOfFuncOrVar_suffix, gv_tphg_stru => global var (tphg) structure.
*/

#include <mkigor_veml.h>

/*
Short graph structure of registers VEML7700
#COM	NAME|	15	14	13	12	11	10	9	8	7	6	5	4	3	2	1	0	|
=================================================================================
			|	0	0	0	-	-	0	-	-	-	-	-	-	0	0	-	-	|
0	ALS_CONF|		ALS_GAIN <12:11>=Gain selection:
			|				00 = ALS gain x 1
			|				01 = ALS gain x 2
			|				10 = ALS gain x (1/8)
			|				11 = ALS gain x (1/4)
			|						ALS_IT <9:6>=ALS integration time setting
			|							1100 = 25 ms
			|							1000 = 50 ms
			|							0000 = 100 ms
			|							0001 = 200 ms
			|							0010 = 400 ms
			|							0011 = 800 ms
			|									ALS_PERS <5:4>=ALS persistence protect number setting
			|											00 = 1
			|											01 = 2
			|											10 = 4
			|											11 = 8
			|												ALS_INT_EN <1>=ALS interrupt enable setting
			|													ALS_SD <0>=ALS shut down enable setting 
---------------------------------------------------------------------------------
1	ALS_WH	|						ALS high threshold							|
2	ALS_WL	|						ALS low threshold							|
3	PSM		| bit <15:3> reserved, <2:1>=PSM, <0> PSM_EN. 						|
4	ALS		|						whole ALS 16 bits raw data count			|
5	WHITE	|						whole WHITE 16 bits raw data count			|
6	ALS_INT	|bit <15>=int_th_low, <14>=int_th_high, bits <13:0> reserved		|
7	ID code	|0xC4->adr 0x20 or 0xD4->adr 0x90|		Fixed device ID = 0x81		|
=================================================================================
*/

//============================================================================================

/**
 * @brief Read 16 bit register of command code = command.
 * @param command - command code of 16 bit register
 * @return data 16 bit register = command.
 */
uint16_t cl_VEML7700::readReg(uint8_t command) {
	uint8_t lv_lsb, lv_msb;
	uint16_t lv_data = 0;

	Wire.beginTransmission(clv_i2cAddr);
	Wire.write(command);
	Wire.endTransmission(false);		///	don't send stop bit here
	Wire.requestFrom(clv_i2cAddr, 2ul, true);	/// stop bit after request
	lv_lsb = Wire.read();
	lv_msb = Wire.read();
	lv_data = lv_msb << 8 | lv_lsb;
	return lv_data;
}

/**
 * @brief Write 16 bit data to command code register.
 * @param command code where to be write,
 * @param data to be write (uint16_t).
 */
void cl_VEML7700::writeReg(uint8_t command, uint16_t data) {
	uint8_t lv_lsb = uint8_t(data & 0x00FF);
	uint8_t lv_msb = uint8_t(data >> 8);
	Wire.beginTransmission(clv_i2cAddr);
	Wire.write(command);
	Wire.write(lv_lsb);
	Wire.write(lv_msb);
	Wire.endTransmission();
}

/**
 * @brief Check the present VEML7700 on i2c bus and init it by default value.
 * @param lp_addr - i2c address of VEML7700 (default is 0x10).
 * @return 0 -  if is error (maybe no connection) or (uint16_t) number = code chip (command 7 reg).
 */
uint16_t cl_VEML7700::check(uint8_t lp_addr) {
	clv_i2cAddr = lp_addr;
	uint8_t lv_lsb, lv_msb;
	uint16_t lv_chipCode = 0;

	Wire.beginTransmission(clv_i2cAddr);
	if (Wire.write(cd_ID) != 1)			return 0;
	if (Wire.endTransmission(false))	return 0;	///	don't send stop bit here
	if (Wire.requestFrom(clv_i2cAddr, 2ul, true) != 2) return 0;
	lv_lsb = Wire.read();
	lv_msb = Wire.read();
	lv_chipCode = lv_msb << 8 | lv_lsb;

	///	write 0 PSM regs -> swich off PSM
	writeReg(cd_PSM, 0);
	///	init command code ALS_CONF:
	///	ALS gain x (1/8), ALS integration time 100 ms, ALS INT disable, ALS power on (wake up)
	writeReg(cd_ALS_CONF, 0x1000);

	return lv_chipCode;
}

/**
 * @brief sent sensor to shut down, min power.
 * @details	all config settings are save,
 * after wakeUp sensor will start count with the same parameters
 */
void cl_VEML7700::sleep() {
	uint16_t lv_reg16 = readReg(cd_ALS_CONF);
	writeReg(cd_ALS_CONF, lv_reg16 | 0x0001);
}

/**
 * @brief Wake Up the sensor from shut down.
 * @details All config settings are the same before shut down.
 */
void cl_VEML7700::wakeUp() {
	uint16_t lv_reg16 = readReg(cd_ALS_CONF);
	writeReg(cd_ALS_CONF, lv_reg16 & 0xFFFE);
}

/**
 * @brief read raw data from sensor and calc it to LUX value
 * @details	ALS and WHITE raw data need to be actual, after call fn wakeUp(),
 * 			should to do delay > 800 ms,
 * @return structure LW_stru_t { (uint32_t)Lux ALS, (uint32_t)Lux WHITE } = ALS & WHATI values in lux 
 */
AW_stru_t cl_VEML7700::readAW() {
	/// Constant vars
	const uint8_t	lvc_nGain = 4;
	const uint8_t	lvc_nTime = 6;
	const uint8_t	lvc_ALSgain[lvc_nGain] = { 2, 3, 0, 1};	///	1/8, 1/4, 1. 2
	const uint8_t	lvc_ALStime[lvc_nTime] = { 0x0C, 0x08, 0, 0x01, 0x02, 0x03};
	const uint16_t	lvc_ALSdelay[lvc_nTime] = { 25, 50, 100,  200,  400,  800};
	const float 	lvc_tableResol[lvc_nGain] [lvc_nTime] = {
			{2.1504, 1.0752, 0.5376, 0.2688, 0.1344, 0.0672},
			{1.0752, 0.5376, 0.2688, 0.1344, 0.0672, 0.0336},
			{0.2688, 0.1344, 0.0672, 0.0336, 0.0168, 0.0084},
			{0.1344, 0.0672, 0.0336, 0.0168, 0.0084, 0.0042} };
	/// Calculate vars
	uint8_t lv_gainIndex = 0;
	uint8_t lv_timeIndex = 0;
	AW_stru_t lv_AW =  { 0, 0 };
	uint16_t lv_ALSdata = readReg(cd_ALS);
	uint16_t lv_WHITEda = readReg(cd_WHITE);
/// It is possible 24 times, find gain & time value
	for (uint8_t k = 0; k < 24; k++) {
		lv_ALSdata = readReg(cd_ALS);
		lv_WHITEda = readReg(cd_WHITE);
		uint16_t lv_ALSconf = readReg(cd_ALS_CONF);
		uint8_t lv_gain = (lv_ALSconf >> 11) & 0x03;
		uint8_t lv_time = (lv_ALSconf >> 6) & 0x0F;
		///	find index
		for (uint8_t i = 0; i < lvc_nGain; i++) if (lv_gain == lvc_ALSgain[i]) lv_gainIndex = i;
		for (uint8_t i = 0; i < lvc_nTime; i++) if (lv_time == lvc_ALStime[i]) lv_timeIndex = i;
#ifdef DEBUG_EN
		printf("Attempt to measure #%d -> ALS=%d, WHITE=%d, gainIdx=%d, timeIdx=%d\n",
			k, lv_ALSdata, lv_WHITEda, lv_gainIndex, lv_timeIndex);
#endif
///		increase or decrease gain or time index to keep raw data ALS in boindes 1000 .. 10000
		if ((lv_ALSdata >= 1000) && (lv_ALSdata <= 10000)) break;	///	raw ALS data is OK, go out of loop for
		if (lv_ALSdata < 1000) {
			if (lv_timeIndex < 2) lv_timeIndex = 2;
			else if (lv_gainIndex < (lvc_nGain - 1)) lv_gainIndex++;
			else if (lv_timeIndex < (lvc_nTime - 1)) lv_timeIndex++;
		}
		else if (lv_ALSdata > 10000) {
			if (lv_timeIndex > 2)	lv_timeIndex--;
			else if (lv_gainIndex != 0)	lv_gainIndex--;
			else if (lv_timeIndex != 0) lv_timeIndex--;
		}

		sleep();		/// Shut down to change config Gain & Time
		lv_ALSconf = lv_ALSconf & 0xE43F;	///	0b 1110 0100 0011 1111 - zero mask for gain & time
		lv_ALSconf = lv_ALSconf | ((uint16_t)lvc_ALSgain[lv_gainIndex])<<11 | ((uint16_t)lvc_ALStime[lv_timeIndex])<<6;
		writeReg(cd_ALS_CONF, lv_ALSconf);
		wakeUp();
		delay(lvc_ALSdelay[lv_timeIndex] + 100);	///	Delay for sensor can update count with new Gain & Time
		// delay(850);	// if something not good work

		///	if reach max or min sensivity of sensor => go out of loop for, no more :-)
		if ( ( lv_gainIndex == (lvc_nGain-1) ) && ( lv_timeIndex == (lvc_nTime-1) ) )	break;
		if ( (lv_gainIndex == 0) && (lv_timeIndex == 0) )	break;
	}

	lv_ALSdata = readReg(cd_ALS);
	lv_WHITEda = readReg(cd_WHITE);
	float lv_coef = lvc_tableResol[lv_gainIndex][lv_timeIndex];
	lv_AW.lux1 = (uint32_t)( round(lv_coef * (float)lv_ALSdata) );
	lv_AW.whi1 = (uint32_t)( round(lv_coef * (float)lv_WHITEda) );
#ifdef DEBUG_EN
	printf("vars -> ALS=%d, WHITE=%d, gainIdx=%d, timeIdx=%d, coef=%f, LUX=%d, WHITE=%d\n\n",
		lv_ALSdata, lv_WHITEda, lv_gainIndex, lv_timeIndex, lv_coef, lv_AW.lux1, lv_AW.whi1);
#endif
	return lv_AW;
}

//============================================================================================