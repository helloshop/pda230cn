/*
 * power_control.c
 *
 *	Module for heater and motor control
 *
 * Created: 27.03.2013 22:32:13
 *  Author: Avega
 */ 

#include "compilers.h"
#include "port_defs.h"
#include "power_control.h"
#include "control.h"
#include "adc.h"

// Heater controls
static uint16_t heaterPower = 0;		// Heater power control, [0 : HEATER_MAX_POWER]

// Motor controls
uint8_t rollState = 0;					// Roll controller state. Use as read-only
uint8_t activeRollCycle = 0;			// Indicates currently active roll cycle
static uint8_t newDirReq = 0;
static uint16_t rollPoint = 0;
static uint16_t topPoint = 0;
static uint16_t bottomPoint = 0;
static uint8_t dirChangedMask = 0xFF;

// p_state bits:
//	[7] <- half-period toggling flag
// [3:0] <- state
uint8_t p_state = 0x0F;			// default state - if AC line sync is present, 
								// it will be cleared at first comparator ISR call



// User function to control heater intensity
void setHeaterPower(uint16_t value)
{
	// Disable interrupts from analog comparator
	ACSR &= ~(1<<ACIE);
	// Update value
	heaterPower = (value > HEATER_MAX_POWER) ? HEATER_MAX_POWER : value;
	// Reenable interrupts
	ACSR |= (1<<ACIE);
}



// User function to control motor rotation
void setMotorDirection(uint8_t dir)
{
	// Disable interrupts from timer0 
	TIMSK &= ~(1<<TOIE0);
		
	newDirReq = dir;	// save new direction request
	dirChangedMask = ~ROLL_DIR_CHANGED;
	
	if (dir & ROLL_FWD)
		bottomPoint = rollPoint;
	else if (dir & ROLL_REV)
		topPoint = rollPoint;

	// Enable interrupts from timer 0
	TIMSK |= (1<<TOIE0);	
}	



uint8_t startCycleRolling(void)
{
	// Disable interrupts from timer0 
	TIMSK &= ~(1<<TOIE0);
	
	if ( isTopPointValid() && isBottomPointValid() )
	{
		rollState |= ROLL_CYCLE;
		activeRollCycle = 1;
	}
	
	// Enable interrupts from timer 0
	TIMSK |= (1<<TOIE0);
	
	return (rollState & ROLL_CYCLE);
}

void stopCycleRolling(uint8_t doResetPoints)
{
	// Disable interrupts from timer0 
	TIMSK &= ~(1<<TOIE0);
	
	rollState &= ~ROLL_CYCLE;
	if (doResetPoints)
	{
		topPoint = bottomPoint = rollPoint;
		activeRollCycle = 0;	
	}		
	
	// Enable interrupts from timer 0
	TIMSK |= (1<<TOIE0);
}



uint8_t isTopPointValid(void)
{
	return (	(int16_t)(topPoint - rollPoint) >= 0 );
}

uint8_t isBottomPointValid(void)
{
	return (	(int16_t)(rollPoint - bottomPoint) >= 0	);
}

//---------------------------------------------//
//---------------------------------------------//
//---------------------------------------------//

static inline uint8_t reachedTopPoint(void)
{
	return (	(int16_t)(topPoint - rollPoint) <= 0 );
}

static inline uint8_t reachedBottomPoint(void)
{
	return (	(int16_t)(rollPoint - bottomPoint) <= 0 );
}

static inline void updateRollPoint(void)
{	
	if (rollState & ROLL_FWD)
		rollPoint++;
	else if (rollState & ROLL_REV)
		rollPoint--;	
}
		


// Function to process rolling - sets rotation direction for next period
// Call once per each AC line period
static inline void controlRolling()
{
	// Process cycle rolling
	switch(rollState & (ROLL_FWD | ROLL_REV | ROLL_CYCLE))
	{
		case (ROLL_FWD | ROLL_CYCLE):
			if (reachedTopPoint())
			{
				if (activeRollCycle >= p.rollCycleSet)	
				{
					// DONE!
					rollState &= ~ROLL_CYCLE;
					rollState |= CYCLE_ROLL_DONE;
				}
				else
				{
					activeRollCycle++;
					// Change dir	
					newDirReq = ROLL_REV;				
				}
			}
			break;
		
		case (ROLL_REV | ROLL_CYCLE):	
			if (reachedBottomPoint())
			{
				if (activeRollCycle >= p.rollCycleSet)	
				{
					// DONE!
					rollState &= ~ROLL_CYCLE;
					rollState |= CYCLE_ROLL_DONE;
				}
				else
				{
					activeRollCycle++;
					// Change dir	
					newDirReq = ROLL_FWD;
				}
			}
			break;
			
		default:
			break;
	}
	
	// Process direction change
	if ((rollState ^ newDirReq) & (ROLL_FWD | ROLL_REV))
	{
		// ROLL_DIR_CHANGED is used for sound beep
		rollState |= (SKIP_CURRENT_MOTOR_CTRL | ROLL_DIR_CHANGED);
	}
	
	rollState &= ~(ROLL_FWD | ROLL_REV);
	rollState |= newDirReq;
	rollState &= dirChangedMask;
	dirChangedMask = 0xFF;
	
	updateRollPoint();
}




ISR(ANA_COMP_vect)
{
	static uint16_t sigma = 0;
	uint16_t delta;
	
	// Once triggered, disable further comparator interrupt
	ACSR &= ~(1<<ACIE);
	
	// Process heater delta-sigma modulator
	if (sigma >= HEATER_MAX_POWER)
	{
		PORTD |= (1<<PD_HEATER | 1<<PD_HEAT_INDIC);
		delta = -HEATER_MAX_POWER;	
	}		
	else
	{
		PORTD &= ~(1<<PD_HEAT_INDIC);
		delta = 0;
	}
	sigma += delta + heaterPower;	
	
	// Reprogram timer0
	TCNT0 = 256 - TRIAC_IMPULSE_TIME;		// Triac gate impulse time
	TIFR |= (1<<TOV0);						// Clear interrupt flag
	// Modify state	
	p_state &= ~STATE_MASK;					// Start new state machine cycle
	p_state ^= HALF_PERIOD_FLAG;			// Toggle flag
	
}



ISR(TIMER0_OVF_vect)
{
	uint8_t temp;
	
	switch(p_state & STATE_MASK)
	{
		// TRIAC_IMPULSE_TIME finished
		case 0x00:
			// Turn off heater TRIAC
			PORTD &= ~(1<<PD_HEATER);
			TCNT0 = 256 - (QUATER_PERIOD_TIME - TRIAC_IMPULSE_TIME);
			break;
		// QUATER_PERIOD_TIME finished	
		case 0x01:	
			TCNT0 = 256 - (SYNC_IGNORE_TIME - QUATER_PERIOD_TIME);
			break;	
		// SYNC_IGNORE_TIME finished	
		case 0x02:
			TCNT0 = 256 - SYNC_LOST_TIMEOUT;
			// Clear flag and enable interrupt from analog comparator
			ACSR |= (1<<ACI);
			ACSR |= (1<<ACIE);
			break;
		// SYNC_LOST_TIMEOUT finished
		case 0x03:
			// Error - sync is lost. It means that device has been disconnected from AC line.
			// Disable all power-consuming devices like LEDs and save user settings to EEPROM.
			exitPowerOff();
			break;
		
		default:
			// Sync is not present - do nothing
			break;
	}	

	// Full period check
	if ((p_state & (HALF_PERIOD_FLAG | STATE_MASK)) == (HALF_PERIOD_FLAG | 0x01))
	{
		// Quater AC line period is done. Update motor controls.
		temp = PORTD;
		temp &= ~(1<<PD_M1 | 1<<PD_M2);
		if ( rollState & SKIP_CURRENT_MOTOR_CTRL )
		{
			// Direction control changed. Skip current period to allow TRIACs fully close
			rollState &= ~SKIP_CURRENT_MOTOR_CTRL;
			PORTD = temp; 	
		}
		else
		{
			// Apply direction control 
			if (rollState & ROLL_FWD)
				temp |= (1<<PD_M1);
			else if (rollState & ROLL_REV)
				temp |= (1<<PD_M2);
			PORTD = temp; 
			
			// Call main roll control function
			controlRolling();
		}
	}
	
	
	if ((p_state & STATE_MASK)  != 0x0F)
		p_state++;
}	









