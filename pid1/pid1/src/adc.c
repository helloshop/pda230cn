/*
 * adc.c
 *
 * Created: 31.03.2013 21:15:32
 *  Author: Avega
 */ 


#include "compilers.h"
#include "port_defs.h"
#include "adc.h"


uint16_t adc_filtered_value;

static uint16_t adc_sample_buffer[ADC_BUFFER_LENGTH];
static int8_t adc_buffer_pos = -1;


uint16_t conv_ADC_to_Celsius(uint16_t adc_value)
{
	float k_norm = -0.4454;
	float offset_norm = 408.2037;
	
	
	return (uint16_t)((float)adc_value * k_norm + offset_norm);
}


void update_filtered_adc()
{
	uint8_t i;
	uint16_t filtered_value = 0;
	// Disable interrupts from ADC - to save data integrity
	ACSR &= ~(1<<ACIE);	
	// Count up
	for(i=0;i<ADC_BUFFER_LENGTH;i++)
	{
		filtered_value += adc_sample_buffer[i];
	}	
	// Enable interrupts from ADC
	ACSR |= (1<<ACIE);
	filtered_value /= ADC_BUFFER_LENGTH;
	adc_filtered_value = filtered_value;
}





ISR(ADC_vect)
{
	uint16_t new_sample = ADC;
	
	if (adc_buffer_pos < 0)
	{
		// First call to the function, fill whole buffer with current sample
		for (adc_buffer_pos = ADC_BUFFER_LENGTH-1; adc_buffer_pos > 0; adc_buffer_pos--)
		adc_sample_buffer[adc_buffer_pos] = new_sample;
	}
	else
	{
		// Normal call
		adc_sample_buffer[adc_buffer_pos] = new_sample;
		adc_buffer_pos = (adc_buffer_pos == ADC_BUFFER_LENGTH - 1) ? 0 : adc_buffer_pos + 1;
	}		
}	





