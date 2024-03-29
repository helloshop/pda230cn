/*
 * led_indic.h
 *
 * Created: 24.03.2013 18:23:27
 *  Author: Avega
 */ 


#ifndef LED_INDIC_H_
#define LED_INDIC_H_

/*
                         bufStartPos
led_data_buffer:             |
      0     1    2    3    4    5    6    7    8    9
    ===================================================
    |    |    |  e |  r |  r |    |  a |  c |    |    |    <- this is data
    ===================================================
              |                             |
			  |  0    1    2    3    4    5 |
              ===============================
Window:       |  e |  r |  r |    |  a |  c |  <- this is actualy displayed
              ===============================
Window start position (wStartPos) = 2
Window length = NUM_DIGITS (of LED display)
bufStartPos defines start position for writing strings
*/


//--------------------------------------------//
// Typedefs

typedef struct {
	uint8_t code;
	uint8_t segdata;
} encode_7seg_pair;


//--------------------------------------------//
// Settings					

#define NUM_DIGITS			6	// Number of digits of LED display
#define LED_BUFFER_LENGTH	(NUM_DIGITS * 2 + 2)	

#define SETUP_DELAY_US		5	// Serial data setup time, us
#define SHIFT_DELAY_US		5	// Serial clock hold time, us
#define PULLUP_DELAY_US		10	// Delay between disabling segment outputs and reading buttons

#define USE_EXTRA_LED_DIGIT		// Use this if you want to manage some separate LEDs dynamically
#define USE_SIMPLE_PRINT		// Simplified string print (no comma processing)
#define EXTRA_DIGIT_NUM		6	// Number of digit with separate LEDs, this value must be <= NUM_DIGITS

#define LED_DIGIT_ACT_LVL	0	// Active level for a digit (common pin for all segments)
#define LED_SEGMENT_ACT_LVL	1	// Active level for segments

#define CLEAN_OPERATION			// If defined, segments are enabled/disabled correctly while shifting
								// For some reason, may work bad in Proteus simulator - try disabling this option

#define LED_SHIFT_INTERVAL	10	// in units of processWindowShifter() call period, (Tsystimer * NUM_DIGITS)

// Encoding
#define SEGA			0x01
#define SEGB			0x02
#define SEGC			0x04
#define SEGD			0x08
#define SEGE			0x10
#define SEGF			0x20
#define SEGG			0x40
#define SEGH			0x80



// Increase total count of LED digits if separate LEDs are attached
#ifdef USE_EXTRA_LED_DIGIT
#define NUM_DIGITS_TOTAL (NUM_DIGITS + 1)
#else
#define NUM_DIGITS_TOTAL NUM_DIGITS
#endif

// Inverse mask
#if LED_SEGMENT_ACT_LVL == 0
#define LED_SEGMENT_MASK 0xFF
#else
#define LED_SEGMENT_MASK 0x00
#endif

// Button inverse mask
#define RAW_BUTTONS_INVERSE_MASK 0xFF

// Shifter states
#define LED_SHIFT_DONE 	0
#define LED_SHIFT_LEFT 	1
#define LED_SHIFT_RIGHT	2


//--------------------------------------------//
// Externs		

// Internal hardware-specific functions
void led_clock_pulse(uint8_t bit);
void set_led_segments(uint8_t seg_ch);
void enable_led_segments_pullups();
void enable_led_segments();
void disable_led_segments();
void capture_button_state();


// Globals
extern uint8_t wStartPos;		// Window position. Should be read-only. 
extern uint8_t bufStartPos;		// Buffer start position. Should be read-only. 
extern uint8_t shifterState;	// Should be read-only

#ifdef USE_EXTRA_LED_DIGIT
extern volatile uint8_t extra_led_state;
#endif
extern volatile uint8_t raw_button_state;


// User functions
void initLedIndicator();
void processLedIndicator();

void setWindowStartPos(int8_t pos);
void shiftWindowPosition(int8_t increment);
void startShiftingWindowRight(void);
void startShiftingWindowLeft(void);
void setBufferStartPos(int8_t pos);
void fillLedBuffer(int8_t offset, uint8_t length, char c);
void printLedBuffer(int8_t offset, char* str);
void setComma(int8_t offset);
void clearComma(int8_t offset);



#ifdef USE_EXTRA_LED_DIGIT
void setExtraLeds(uint8_t leds_set_value);
void clearExtraLeds(uint8_t leds_clear_value);
void toggleExtraLeds(uint8_t leds_toggle_value);
#endif

#endif /* LED_INDIC_H_ */

