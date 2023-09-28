#include "Utilities/my_basic.h"

void printOut(const char *text)
{
	while (__HAL_UART_GET_FLAG(&huart3,UART_FLAG_TC) == RESET)
	{
	}
	HAL_UART_Transmit(&huart3, (uint8_t*) text, strlen(text), 100);
}

void printOutLine(const char *text)
{
	printOut(text);
	printOut("\r\n");
}

void receiveUserInput(char *rxData)
{
	while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE) == RESET)
	{
	}
	HAL_UART_Receive(&huart3, (uint8_t*) rxData, 1, 100);

	printOutLine(rxData);
}

void delay(uint32_t delay)
{
	HAL_Delay(delay);
}

// For AM2320
void AM2320_setSensorValue(uint8_t *cmdBuffer)
{
	cmdBuffer[0] = 0x03;
	cmdBuffer[1] = 0x00;
	cmdBuffer[2] = 0x04;
}

void AM2320_startSensor(I2C_HandleTypeDef *hi2c, uint8_t *cmdBuffer, uint8_t *dataBuffer)
{
	// Setting hi2c1: PB8, PB9
	HAL_I2C_Master_Transmit(hi2c, 0x5c<<1, cmdBuffer, 3, 200);
	HAL_I2C_Master_Transmit(hi2c, 0x5c<<1, cmdBuffer, 3, 200);
	delay(1);

	HAL_I2C_Master_Receive(hi2c, 0x5c<<1, dataBuffer, 8, 200);
}

void AM2320_calculateValue(float *temperature, float *humidity, uint8_t dataBuffer[])
{
	uint16_t Rcrc = dataBuffer[7] << 8;
	Rcrc += dataBuffer[6];
	if (Rcrc == AM2320_CRC16_2(dataBuffer, 6))
	{
		uint16_t temperatureRawValue = ((dataBuffer[4] & 0x7F) << 8) + dataBuffer[5];
		*temperature = temperatureRawValue / 10.0;
		*temperature = (((dataBuffer[4] & 0x80) >> 7) == 1) ? (*temperature * (-1)) : *temperature;

		uint16_t himdityRawValue = (dataBuffer[2] << 8) + dataBuffer[3];
		*humidity = himdityRawValue / 10.0;
	}
}

uint16_t AM2320_CRC16_2(uint8_t *ptr, uint8_t length)
{
	uint16_t crc = 0xFFFF;
	uint8_t s = 0x00;

	while (length--)
	{
		crc ^= *ptr++;
		for (s = 0; s < 8; s++)
		{
			if ((crc & 0x01) != 0)
			{
				crc >>= 1;
				crc ^= 0xA001;
			}
			else
			{
				crc >>= 1;
			}
		}
	}
	return crc;
}
