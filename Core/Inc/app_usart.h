/*
 * app_usart.h
 *
 *  Created on: Jun 15, 2023
 *      Author: mrdri
 *
 *  EXAMPLE:
 *
 *  ##########################
 *  In main application loop (or uart thread in RTOS scenario)
 *  ##########################
 *
 *	  app_uart_init(&huart2);
 *
 *	  while(1)
 *	  {
	 *    // Read all messages in buffer
	 * 	  while(is_cmd_rdy())
		  {
			  light_cmd_t cmd = get_cmd(input_buf);

			  //Send a spoof ctrl cmd back to PC
			  ctrl_input_t ctrl_input = {
				  .type = INPUT_TYPE_BUTTON,
				  .idx = 1,
				  .val = 1,
			  };
			  uart_write(&ctrl_input, sizeof(ctrl_input_t));
		  }
	  }

	##########################
	In st32f4xx_it.c, add to USARTx_IRQHandler()
	##########################
*/
//		void USART2_IRQHandler(void)
//		{
//		  /* USER CODE BEGIN USART2_IRQn 0 */
//
//		  /* USER CODE END USART2_IRQn 0 */
//		  HAL_UART_IRQHandler(&huart2);
//		  /* USER CODE BEGIN USART2_IRQn 1 */
//		  if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE) == 1)
//		  {
//			uart_idle_callback(&huart2);
//		  }
//		  /* USER CODE END USART2_IRQn 1 */
//		}
/* */

#ifndef INC_APP_USART_H_
#define INC_APP_USART_H_

#include <stdbool.h>
#include "main.h"


typedef enum {
	INPUT_TYPE_INVALID = 0,
	INPUT_TYPE_BUTTON = 'b',
	INPUT_TYPE_POT = 'p',
}user_ctrl_type_t;

/* This struct is information we receive from PC */
typedef struct {
	// MUST MATCH PYTHON: class RGB(ctypes.Structure)
	uint8_t lightIdx; //which floodlight to talk to
	uint8_t R;
	uint8_t G;
	uint8_t B;
}light_cmd_t;

/* This struct is information we send to PC */
typedef struct {
	// MUST MATCH PYTHON: class ctrl_input(ctypes.Structure)
	user_ctrl_type_t type;
	uint8_t idx; //which pot or button (0 - n)
	uint16_t val; //0-1 for button, 0-uint16 for pot
}user_ctrl_t;

/* Call this after the HAL USART init, but before receiving any data */
void app_uart_init(UART_HandleTypeDef *huart);

/*
 * @brief Send an array of bytes
 *
 * @note snprintf() can be handy if sending strings
 */
size_t uart_write(const char * buf, size_t len);

/*
 * @brief Check if we have received an entire light_cmd_t message from a sender
 */
bool is_cmd_rdy(void);

/*
 * @brief Extract a message from the input buffer
 */
light_cmd_t get_cmd(void);

/**
 * @brief UART RX callback - loads data into application buffer.
 *
 * Expects DMA to be enabled.
 */
void uart_idle_callback(UART_HandleTypeDef* huart);

#endif /* INC_APP_USART_H_ */
