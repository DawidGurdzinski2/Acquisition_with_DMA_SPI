#ifndef __MCP3208_H__
#define __MCP3208_H__

#include "stm32f4xx_hal.h"

#define CHAN 8
#define SAMPLES 680//
#define BUFLEN 3*CHAN*SAMPLES//3 bajty na 8 kanałOW po 680 probek na jedej tablicy 16320


void InitMCP3208(SPI_HandleTypeDef hspirx,TIM_HandleTypeDef htimgate, uint32_t gateChannel,
		TIM_HandleTypeDef htimmaster, uint32_t masterChannel, uint32_t ChipSelectChannel);


#endif /* __MCP3208_H__ */
