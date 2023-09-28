/*
 * my_basic.h
 *
 *  Created on: Sep 28, 2023
 *      Author: sunsu
 */

#ifndef INC_UTILITIES_MY_BASIC_H_
#define INC_UTILITIES_MY_BASIC_H_

#include "usart.h"
#include "i2c.h"
#include "string.h"
#include "stdint.h"

void printOut(const char *text);

void printOutLine(const char *text);

void receiveUserInput(char *rxData);

void delay(uint32_t delay);


void AM2320_setSensorValue(uint8_t *cmdBuffer);
void AM2320_startSensor(I2C_HandleTypeDef *hi2c, uint8_t *cmdBuffer, uint8_t *dataBuffer);
void AM2320_calculateValue(float *temperature, float *humidity, uint8_t dataBuffer[]);
uint16_t AM2320_CRC16_2(uint8_t *ptr, uint8_t length);


#endif /* INC_UTILITIES_MY_BASIC_H_ */