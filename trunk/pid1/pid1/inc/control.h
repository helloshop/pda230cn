/*
 * control.h
 *
 * Created: 15.04.2013 21:26:56
 *  Author: Avega
 */ 


#ifndef CONTROL_H_
#define CONTROL_H_


typedef struct  
{
	uint8_t setup_temp_value;
	uint8_t rollCycleSet;
	uint8_t sound_enable;
	uint8_t power_off_timeout;
	uint8_t cpoint1;
	uint8_t cpoint2;
} gParams_t;


#define MAX_SET_TEMP	210
#define MIN_SET_TEMP	30

#define MAX_ROLL_CYCLES	99
#define MIN_ROLL_CYCLES 1

#define MAX_POWEROFF_TIMEOUT	95
#define MIN_POWEROFF_TIMEOUT	5

#define MAX_CALIB_TEMP		250	
#define MIN_CALIB_TEMP		10


// Global variables - main system control
extern uint16_t setup_temp_value;			// reference temperature
extern uint8_t rollCycleSet;				// number of rolling cycles
extern uint8_t sound_enable;				// Global sound enable
extern uint8_t power_off_timeout;			// Auto power OFF timeout
extern uint8_t cpoint1;						// Calibration point 1
extern uint8_t cpoint2;						// Calibration point 2

void processRollControl(void);
void processHeaterControl(void);

void restoreGlobalParams(void);
void exitPowerOff(void);
void processAutoPowerOff(void);


#endif /* CONTROL_H_ */