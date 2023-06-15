/*
 * app_usart.c
 *
 *  Created on: Jun 15, 2023
 *      Author: mrdri
 */
#include <stdio.h>
#include <string.h>

#include "app_usart.h"

#define MAX_OUTPUT_LENGTH 100
#define MAX_INPUT_LENGTH 100
#define UART_115200_CHAR_TIME (1/10)

char input_buf[ MAX_INPUT_LENGTH ];
volatile uint32_t input_idx = 0;
static uint8_t dma_in[MAX_INPUT_LENGTH];
volatile bool uart_tx_cmplt = false;

extern UART_HandleTypeDef huart2;


/* Call this after the HAL USART init, but before receiving any data */
void app_uart_init(UART_HandleTypeDef *huart)
{
	__HAL_UART_CLEAR_IT(huart, UART_IT_IDLE);
	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
	if(HAL_OK != HAL_UART_Receive_DMA(huart, dma_in, MAX_INPUT_LENGTH))
	{
		while(1); //bad bad
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	uart_tx_cmplt = true;
}

size_t uart_write(const char * buf, size_t len)
{
	//verify last tx is done before starting new one
	if(uart_tx_cmplt != true)
	{
		HAL_Delay(1);
	}

	//transmit data
	uart_tx_cmplt = false;
	if (HAL_UART_Transmit_DMA(&huart2, (uint8_t *)buf, len) != HAL_OK) {
		return 0;
	}

	return len;
}
/**
 * @brief UART RX callback - loads data into application buffer.
 *
 * Expects DMA to be enabled.
 */
void uart_idle_callback(UART_HandleTypeDef* huart)
{
	HAL_StatusTypeDef status;

	__HAL_UART_CLEAR_IDLEFLAG(huart); //Clear idle interrupt flag
	// No good way to restart without totally stopping
	HAL_DMA_Abort(huart->hdmarx); //Pause DMA
	HAL_UART_AbortReceive(huart);

	//Calculate the length of the received data & copy it
	uint8_t num_bytes = huart->RxXferSize - __HAL_DMA_GET_COUNTER(huart->hdmarx);
	if(num_bytes < MAX_INPUT_LENGTH && num_bytes > 0)
	{
		memcpy(&input_buf[input_idx], dma_in, num_bytes);
		input_idx += num_bytes;
		memset(dma_in, 0, num_bytes);
	}

	//restart DMA for next rx
	status = HAL_UART_Receive_DMA(huart, dma_in, MAX_INPUT_LENGTH);
	if(status != HAL_OK)
	{
		//very bad
	}
}

/*
 * @brief Extract a message from the input buffer
 */
light_cmd_t get_cmd(void)
{
	light_cmd_t cmd = {0};
	uint8_t cmd_sz = sizeof(light_cmd_t);

	//Extract cmd
	memcpy(&cmd, input_buf, cmd_sz);

	//Shuffle buffer
	memcpy(input_buf, &input_buf[cmd_sz], cmd_sz);
	memset(&input_buf[MAX_INPUT_LENGTH - cmd_sz], 0, cmd_sz);

	int check_idx = input_idx - cmd_sz;
	if(check_idx >= 0)
	{
		input_idx -= cmd_sz;
	}
	else
	{
		while(1); //index problemo
	}

	return cmd;
}

/*
 * @brief Check if we have received an entire light_cmd_t message from a sender
 */
bool is_cmd_rdy(void)
{
	if(input_idx >= MAX_INPUT_LENGTH)
	{
		char* err_msg = "uC in buf overflow\r\n";
		uart_write(err_msg, strlen(err_msg));
		while(1);
	}

	if(input_idx >= sizeof(light_cmd_t))
		return true;
	else
		return false;
}
