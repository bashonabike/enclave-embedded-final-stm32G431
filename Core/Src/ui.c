/*
 * ui.c
 *
 *  Created on: Jun. 19, 2023
 *      Author: mrdri
 */

#include "main.h"
#include "ui.h"
#include "app_usart.h"
#include "stdlib.h"

typedef enum {
	//NOTE: repeat since latter half knobs are on adc2
	KNOB1Chn = ADC_CHANNEL_1, KNOB2Chn = ADC_CHANNEL_2, KNOB3Chn = ADC_CHANNEL_17,
	KNOB4Chn = ADC_CHANNEL_13,	KNOB5Chn = ADC_CHANNEL_3, KNOB6Chn = ADC_CHANNEL_4
} KnobChannel;

struct Knob {
	ADC_HandleTypeDef * adc;
	KnobChannel channel;
	uint16_t value;
	uint16_t oldValue;
};

struct Button {
	GPIO_TypeDef * GPIO;
	uint32_t pin;
	GPIO_PinState pinState;
	_Bool buttonState;
	uint8_t debounceCounter;
};

#define NUMKNOBS 6
//Div by 5 since 5ms polling
#define DEBOUNCECYCLES 20
struct Knob knobs[NUMKNOBS];

#define NUMBUTTONS 6
struct Button buttons[NUMBUTTONS];

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

_Bool knobsInitialized = 0;
_Bool butInitialized = 0;


uint16_t readADCChannel(KnobChannel channel, ADC_HandleTypeDef * adc) {
	ADC_ChannelConfTypeDef sConfig = { 0 };

	/** Configure Regular Channel
	 */
	sConfig.Channel = channel;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}

	HAL_ADC_Start(adc);
	HAL_ADC_PollForConversion(adc, 1);
	return HAL_ADC_GetValue(adc);
}

_Bool initializeKnobStructs(void) {
	uint8_t knob;
	for(knob = 0; knob < NUMKNOBS; knob++) {
		switch(knob) {
		case 0:
			knobs[knob].channel = KNOB1Chn;
			knobs[knob].adc =&hadc1;
			break;
		case 1:
			knobs[knob].channel = KNOB2Chn;
			knobs[knob].adc = &hadc1;
			break;
		case 2:
			knobs[knob].channel = KNOB3Chn;
			knobs[knob].adc = &hadc2;
			break;
		case 3:
			knobs[knob].channel = KNOB4Chn;
			knobs[knob].adc = &hadc2;
			break;
		case 4:
			knobs[knob].channel = KNOB5Chn;
			knobs[knob].adc = &hadc2;
			break;
		case 5:
			knobs[knob].channel = KNOB6Chn;
			knobs[knob].adc = &hadc2;
			break;
		default:
			//Configure shit!
			return 1;
		}

		knobs[knob].value = readADCChannel(knobs[knob].channel, knobs[knob].adc);
		knobs[knob].oldValue = knobs[knob].value;
	}
	knobsInitialized = 1;
	return 0;
}


_Bool initializeButtonStructs(void) {
	uint8_t button;
	for(button = 0; button < NUMBUTTONS; button++) {
		switch(button) {
		case 0:
			buttons[button].GPIO = GPIOA;
			buttons[button].pin = But1_Pin;
			break;
		case 1:
			buttons[button].GPIO = GPIOA;
			buttons[button].pin = But2_Pin;
			break;
		case 2:
			buttons[button].GPIO = GPIOA;
			buttons[button].pin = But3_Pin;
			break;
		case 3:
			buttons[button].GPIO = GPIOB;
			buttons[button].pin = But4_Pin;
			break;
		case 4:
			buttons[button].GPIO = GPIOB;
			buttons[button].pin = But5_Pin;
			break;
		case 5:
			buttons[button].GPIO = GPIOA;
			buttons[button].pin = But6_Pin;
			break;
		default:
			//Configure shit!
			return 1;
		}

		buttons[button].pinState = GPIO_PIN_RESET;
		buttons[button].buttonState = 0;
		buttons[button].debounceCounter = 0;
	}
	butInitialized = 1;
	return 0;
}

void pollKnobs(void) {
	static uint8_t knob;
	for(knob = 0; knob < NUMKNOBS; knob++) {
		uint16_t newVal = readADCChannel(knobs[knob].channel, knobs[knob].adc);

		//Filter out noise - only allow 1/100th of new value
		uint16_t newFiltVal = (9*(uint32_t)knobs[knob].value + (uint32_t)newVal) / 10;

		if(knobs[knob].value != newFiltVal)
		{
			knobs[knob].value = newFiltVal;
			if (abs(knobs[knob].value - knobs[knob].oldValue) > 10) {

			//Send ctrl cmd back to PC
			user_ctrl_t ctrl_input = {
				.type = INPUT_TYPE_POT,
				.idx = knob,
				.val = knobs[knob].value,
			};
			uart_write((char*)&ctrl_input, sizeof(user_ctrl_t));

			knobs[knob].oldValue = knobs[knob].value;
			}
		}
	}
}

void debounceButtons(void) {
	static uint8_t button;

	for (button = 0; button < NUMBUTTONS; button++) {
		if (HAL_GPIO_ReadPin(buttons[button].GPIO, buttons[button].pin)
				== GPIO_PIN_SET) {
			if (buttons[button].buttonState == 0) {
				buttons[button].debounceCounter++;
				if (buttons[button].debounceCounter >= DEBOUNCECYCLES) {
					//Legit button press!
					buttons[button].debounceCounter = 0;
					buttons[button].buttonState = 1;

					//Send ctrl cmd back to PC
					user_ctrl_t ctrl_input = {
						.type = INPUT_TYPE_BUTTON,
						.idx = button,
						.val = 1,
					};
					uart_write((char*)&ctrl_input, sizeof(user_ctrl_t));
				}
			}
		} else if (buttons[button].debounceCounter != 0) {
			buttons[button].debounceCounter = 0;
		} else if (buttons[button].buttonState != 0) {
			buttons[button].buttonState = 0;
		}
	}
}

_Bool isKnobsRdy(void)
{
	return knobsInitialized;
}

_Bool isButRdy(void)
{
	return butInitialized;
}
