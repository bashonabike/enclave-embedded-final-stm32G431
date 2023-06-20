/*
 * floodlight.c
 *
 *  Created on: Jun. 19, 2023
 *      Author: mrdri
 */

#include "main.h"
#include "stdlib.h" //for abs
#include "floodlight.h"

#define MAX_LED_DIM 255
#define NUM_LED_LEVELS 256 //255+1

#define LED_ON_STATE(floodlightNum, LEDNum) if(!floodlights[floodlightNum][LEDNum].pinOn) {\
		HAL_GPIO_WritePin(floodlights[floodlightNum][LEDNum].GPIO, \
				floodlights[floodlightNum][LEDNum].LEDColourChannelPin, GPIO_PIN_SET);\
		floodlights[floodlightNum][LEDNum].pinOn = 1;}
#define LED_OFF_STATE(floodlightNum, LEDNum) if(floodlights[floodlightNum][LEDNum].pinOn) {\
		HAL_GPIO_WritePin(floodlights[floodlightNum][LEDNum].GPIO, \
				floodlights[floodlightNum][LEDNum].LEDColourChannelPin, GPIO_PIN_RESET);\
		floodlights[floodlightNum][LEDNum].pinOn = 0;}

//Set to (666+1)*(7+1) so same rate as timer2
extern volatile uint32_t *DWT_CYCCNT;
#define timer2cycle() (*DWT_CYCCNT/5336)
#define starttilnow() (timer2cycle() - start)

#define NUMLIGHTS 2

typedef enum {
	LED_R = 0,
	LED_G,
	LED_B,
	NUM_LEDS
}LEDIdx_t;

struct FloodlightLED {
	//0-255 for every light colour
	unsigned short brightness;

	GPIO_TypeDef * GPIO;
	uint32_t LEDColourChannelPin;
	_Bool pinOn;
	unsigned short rise1;
	unsigned short fall1;
	unsigned short rise2;
	unsigned short fall2;
};

struct FloodlightLED floodlights[NUMLIGHTS][NUM_LEDS];

_Bool fldInitialized = 0;

unsigned long width = 0;
unsigned long avgWidth = 0;
_Bool set = 0;
unsigned long start = 0;
unsigned short place = 0;

unsigned short curMains = 0;
uint8_t incrDuty = 0;
int curShift = 0;
uint32_t curStartTilNow = 0;

_Bool initializeFloodlightStructs(void)
{
	short floodlightNum, LEDNum;
	for (floodlightNum = 0; floodlightNum < NUMLIGHTS;
			floodlightNum++) {
		for (LEDNum = LED_R; LEDNum < NUM_LEDS; LEDNum++) {
			//NOTE: assuming no more than 10 LED per floodlight
			switch (10*floodlightNum + LEDNum) {
			case 0:
				floodlights[floodlightNum][LEDNum].GPIO = GPIOB;
				floodlights[floodlightNum][LEDNum].LEDColourChannelPin = Fld1R_Pin;
				break;
			case 1:
				floodlights[floodlightNum][LEDNum].GPIO = GPIOF;
				floodlights[floodlightNum][LEDNum].LEDColourChannelPin = Fld1G_Pin;
				break;
			case 2:
				floodlights[floodlightNum][LEDNum].GPIO = GPIOF;
				floodlights[floodlightNum][LEDNum].LEDColourChannelPin = Fld1B_Pin;
				break;
			case 10:
				floodlights[floodlightNum][LEDNum].GPIO = GPIOA;
				floodlights[floodlightNum][LEDNum].LEDColourChannelPin = Fld2R_Pin;
				break;
			case 11:
				floodlights[floodlightNum][LEDNum].GPIO = GPIOA;
				floodlights[floodlightNum][LEDNum].LEDColourChannelPin = Fld2G_Pin;
				break;
			case 12:
				floodlights[floodlightNum][LEDNum].GPIO = GPIOB;
				floodlights[floodlightNum][LEDNum].LEDColourChannelPin = Fld2B_Pin;
				break;
			default:
				//Configure shit!
				return 1;
			}
			floodlights[floodlightNum][LEDNum].pinOn = 0;
			floodlights[floodlightNum][LEDNum].brightness = 0;
			floodlights[floodlightNum][LEDNum].rise1 = 0;
			floodlights[floodlightNum][LEDNum].fall1 = 0;
			floodlights[floodlightNum][LEDNum].rise2 = 0;
			floodlights[floodlightNum][LEDNum].fall2 = 0;

		}
	}
	fldInitialized = 1;
	return 0;
}

_Bool isFloodlightRdy(void)
{
	return fldInitialized;
}

void resetRiseFall(uint8_t floodlightNum, uint8_t LEDNum) {
	//Divide by 2 since we cycle duty per half-wave, not full wave
		static unsigned short effDuty;
	effDuty = (short) ((((unsigned long) width
			* (unsigned long) floodlights[floodlightNum][LEDNum].brightness)) / (2 * NUM_LED_LEVELS));
	//If dim is odd, this granularity will be lost in compression since only 128 pulse width levels available
	//i.e. (512*255)/(2*256) == 255 == (512*254)/(2*256)
	//Circumvent this by offsetting all odd values by 1 on fall to increase pulse width by 1/2 a level
	//NOTE passing in as pointer since resetRiseFall can be triggered asynchronously via seperate interrupts
	_Bool dimCorrect = floodlights[floodlightNum][LEDNum].brightness & 1;
	//TODO: calculate & set timer freq so exactly 512*mains freq (change if using ext osc)
	floodlights[floodlightNum][LEDNum].rise1 = ((width / 2) - effDuty) / 2;
	floodlights[floodlightNum][LEDNum].fall1 = floodlights[floodlightNum][LEDNum].rise1 + effDuty + dimCorrect;
	floodlights[floodlightNum][LEDNum].rise2 = (width / 2) + floodlights[floodlightNum][LEDNum].rise1;
	floodlights[floodlightNum][LEDNum].fall2 = (width / 2) + floodlights[floodlightNum][LEDNum].fall1;
}

void resetAllRiseFall(void) {
	static short floodlightNum, LEDNum;
		for (floodlightNum = 0; floodlightNum < NUMLIGHTS;
				floodlightNum++) {
			for (LEDNum = LED_R; LEDNum < NUM_LEDS; LEDNum++) {
				resetRiseFall(floodlightNum, LEDNum);
			}
		}
}

void updateFloodlight(uint8_t lightIdx, uint8_t R, uint8_t G, uint8_t B)
{
	if(floodlights[lightIdx][LED_R].brightness != R)
	{
		floodlights[lightIdx][LED_R].brightness = R;
		resetRiseFall(lightIdx, LED_R);
	}
	if(floodlights[lightIdx][LED_G].brightness != G)
	{
		floodlights[lightIdx][LED_G].brightness = G;
		resetRiseFall(lightIdx, LED_G);
	}
	if(floodlights[lightIdx][LED_B].brightness != B)
	{
		floodlights[lightIdx][LED_B].brightness = B;
		resetRiseFall(lightIdx, LED_B);
	}
}

void mainsDetect(void) {
	//Why does width occasionally jump to ~20k micros for a bit??  Then returns back to ~61Hz
	//Filtered out by blip discard method.  May be artifact of low frequency operation
	if (set) {
		curStartTilNow = starttilnow();
		//Slowly track to actual width if permanent shift
		avgWidth = ((19 * (unsigned long) width + (curStartTilNow)) / 20);
		//Try to filter out blips, is this an actual frequency shift on mains?
		//If average closer to calculated than historical, this is the new frequency
		//Explicit cast to (long) since stdlib has no overload of abs for unsigned long
      if (width == 0 || abs((unsigned long)((curStartTilNow) - width)) < width/16
       || abs((unsigned long)((curStartTilNow) - avgWidth)) < abs((unsigned long)(width - avgWidth))) {
        width = curStartTilNow;
		//Reset avg
		//avgWidth = width;
		//reset applicable variables
		resetAllRiseFall();
      }

	} else {
		set = 1;
	}
	if (*DWT_CYCCNT > DWT_CYCLE_RESET)  *DWT_CYCCNT = 0;                  // clear DWT cycle counter
	start = timer2cycle();

#define xTESTING_FLOODLIGHT
#if TESTING_FLOODLIGHT
	static _Bool flip1, flip2;
	static uint16_t reset = 0;
		reset++;
		if (reset >= 1) {
			if (floodlights[0][0].brightness > MAX_LED_DIM) floodlights[0][0].brightness=0;
			if (floodlights[0][1].brightness > MAX_LED_DIM) floodlights[0][1].brightness=0;
			if(!flip1) {
			floodlights[0][0].brightness = (floodlights[0][0].brightness + 1);
			if (floodlights[0][0].brightness >= MAX_LED_DIM-1) {
				flip1 = 1;
				floodlights[0][0].brightness = MAX_LED_DIM-1;
			}
			} else {
				floodlights[0][0].brightness = (floodlights[0][0].brightness - 1);
				if (floodlights[0][0].brightness <= 0 || floodlights[0][0].brightness > MAX_LED_DIM) {
					flip1 = 0;
					floodlights[0][0].brightness = 0;
				}
			}
			if(!flip2) {
			floodlights[0][1].brightness = (floodlights[0][1].brightness - 1);
			if (floodlights[0][1].brightness == MAX_LED_DIM-1) flip2 = 1;
			if (floodlights[0][1].brightness <= 0 || floodlights[0][1].brightness > MAX_LED_DIM ) {
				flip2 = 1;
				floodlights[0][1].brightness = 0;
			}
			} else {
				floodlights[0][1].brightness = (floodlights[0][1].brightness + 1);
				if (floodlights[0][0].brightness >= MAX_LED_DIM-1) {
					flip2 = 1;
					floodlights[0][1].brightness = MAX_LED_DIM-1;
				}
			}
					floodlights[0][2].brightness = abs(floodlights[0][0].brightness - floodlights[0][1].brightness) % MAX_LED_DIM;
			        static _Bool dimCorrect;
					resetRiseFall(0, 0, &dimCorrect);
					resetRiseFall(0, 1, &dimCorrect);
					resetRiseFall(0, 2, &dimCorrect);
					reset = 0;
					floodlights[1][0].brightness = 255 - floodlights[0][0].brightness;
					floodlights[1][1].brightness = 255 - floodlights[0][1].brightness;
					floodlights[1][2].brightness = 255 - floodlights[0][2].brightness;
		}
		//--------------------------------------------------------------------------------------------------------
#endif
}

#pragma GCC push_options
#pragma GCC optimize ("-O3")
void pulseFloodlight() {
	//Triggers every 32 us
	place = starttilnow();
	static short floodlightNum, LEDNum;
	for (floodlightNum = 0; floodlightNum < NUMLIGHTS;
			floodlightNum++) {
		for (LEDNum = LED_R; LEDNum < NUM_LEDS; LEDNum++) {
			//NOTE: we know as each success LT is evaluated, don't need to re-eval subsequent GTE
			if (place < floodlights[floodlightNum][LEDNum].rise1) {
				LED_OFF_STATE(floodlightNum, LEDNum)
			} else if (place < floodlights[floodlightNum][LEDNum].fall1) {
				LED_ON_STATE(floodlightNum, LEDNum)
			} else if (place < floodlights[floodlightNum][LEDNum].rise2) {
				LED_OFF_STATE(floodlightNum, LEDNum)
			} else if (place < floodlights[floodlightNum][LEDNum].fall2) {
				LED_ON_STATE(floodlightNum, LEDNum)
			} else {
				LED_OFF_STATE(floodlightNum, LEDNum)
			}
		}
	}
}
#pragma GCC pop_options

// EXTI Mains External Interrupt ISR Handler
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if(fldInitialized) {
		if (GPIO_Pin == Mains_Pin) {
			mainsDetect();
		}
	}
}
