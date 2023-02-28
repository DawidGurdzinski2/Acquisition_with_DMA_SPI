
#include <stdio.h>
#include "mcp3208.h"
#include "stm32f4xx_hal.h"

uint8_t databuf[BUFLEN];
uint8_t adcconv[3*CHAN];



/*  spi handle , gated timer to send byte by byte , channel of gated timer,
 * master timer which is gating previous timer, master channel ,PWM channel/Chip Select channel*/

void InitMCP3208(SPI_HandleTypeDef hspirx,TIM_HandleTypeDef htimgate, uint32_t gateChannel,
		TIM_HandleTypeDef htimmaster, uint32_t masterChannel, uint32_t ChipSelectChannel){
	for (int i = 0; i < CHAN; i++) {
		  	adcconv[3 * i] = 6 | ((i >> 2) & 1);
		  	adcconv[3 * i + 1] = (i & 3) << 6;
			adcconv[3 * i + 2] = 0;
		}
	__HAL_SPI_ENABLE(&hspirx);
	HAL_SPI_Receive_DMA(&hspirx, databuf, BUFLEN);
	HAL_TIM_OC_Start(&htimgate, gateChannel);
	uint16_t chanID=(uint16_t)(1+(gateChannel/4));
	HAL_DMA_Start_IT(htimgate.hdma[chanID],(uint32_t) &adcconv, (uint32_t)&hspirx.Instance->DR, 3*CHAN);

	switch(chanID){
		case 1:
			chanID=0x0200;
			break;
		case 2:
			chanID=0x0400;
			break;
		case 3:
			chanID=0x0800;
			break;
		case 4:
			chanID=0x1000;
			break;
		default:
			chanID=0x0200;
			break;
	}
	__HAL_TIM_ENABLE_DMA(&htimgate,chanID);
	HAL_TIM_OC_Start(&htimmaster, masterChannel);
	HAL_TIM_OC_Start(&htimmaster, ChipSelectChannel);

}



