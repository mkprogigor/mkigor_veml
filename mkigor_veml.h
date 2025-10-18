/**
*	@brief		Arduino Library for sensor light VEML7700.
*	@author		Igor Mkprog, mkprogigor@gmail.com
*	@version	V1.0	@date	16.10.2025
*	@details	of using. 1st - wake up sendor from shut down, metod cl_VEML7700.wakeUp(),
*			wait for > 800 ms and receive light ALS and WHITE by metod readAW().
*			Method do not has fix time executing, because it find and select proper coefficient
*			for gain & time counting. It can takes time form 500 to 3000ms.
*			I correct a little bit std algoritm from datasheet of Vishay company.
*			From my expirience, should take one or couple times more measuremant. Results will be more stable.
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

#include <Arduino.h>
#include <Wire.h>

#ifndef mkigor_veml_h
#define mkigor_veml_h

/// Command code of registers
#define cd_ALS_CONF 0
#define cd_PSM		3
#define cd_ALS		4
#define cd_WHITE	5
#define cd_ID		7

// #define DEBUG_EN		/// Uncomment to print add info

struct AW_stru_t	{
	uint32_t als1;
	uint32_t whi1;
};

//============================================================================================

class cl_VEML7700 {
private:
	uint8_t clv_i2cAddr;
public:
	cl_VEML7700() {					/// default class constructor
		clv_i2cAddr = 0x10;			/// default VEML7700 i2c address
	};

/**
 * @brief Read 16 bit register of command code = command.
 * @param command - command code of 16 bit register
 * @return data 16 bit register = command.
 */
	uint16_t readReg(uint8_t command);

/**
 * @brief Write 16 bit data to command code register.
 * @param command code where to be write,
 * @param data to be write (uint16_t).
 */
void writeReg(uint8_t command, uint16_t data);

/**
 * @brief Check the present VEML7700 on i2c bus and init it by default value.
 * @param lp_addr - i2c address of VEML7700 (default is 0x10).
 * @return 0 -  if is error (maybe no connection) or (uint16_t) number = code chip (command 7 reg).
 */
uint16_t check(uint8_t lp_addr = 0x10);

/**
 * @brief sent sensor to shut down, min power.
 * @details	all config settings are save,
 * after wakeUp sensor will start count with the same parameters
 */
void sleep();

/**
 * @brief Wake Up the sensor from shut down.
 * @details All config settings are the same before shut down.
 */
void wakeUp();

/**
 * @brief read raw data from sensor and calc it to LUX value
 * @details	ALS and WHITE raw data need to be actual, after call fn wakeUp(),
 * 			should to do delay > 800 ms,
 * @return structure AW_stru_t { (uint32_t)Lux ALS, (uint32_t)Lux WHITE } = ALS & WHATI values in lux 
 */
AW_stru_t readAW();

};

#endif
//============================================================================================
