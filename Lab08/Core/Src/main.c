/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rng.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "ILI9341_Touchscreen.h"
#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"

#include "snow_tiger.h"

#include "Utilities/my_lcd.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
typedef struct _ColorIntensity
{
	Circle circle;
	float intensity;
} ColorIntensity ;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
const uint8_t SCREEN_ROTATION = SCREEN_HORIZONTAL_1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void testTouch() {
	while (1) {
		HAL_Delay(20);

		if (TP_Touchpad_Pressed()) {

			uint16_t x_pos = 0;
			uint16_t y_pos = 0;

			HAL_GPIO_WritePin(GPIOB, LD3_Pin | LD2_Pin, GPIO_PIN_SET);

			uint16_t position_array[2];

			if (TP_Read_Coordinates(position_array) == TOUCHPAD_DATA_OK) {
				x_pos = position_array[0];
				y_pos = position_array[1];
				ILI9341_Draw_Filled_Circle(x_pos, y_pos, 2, BLACK);

				ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
				char counter_buff[30];
				sprintf(counter_buff, "POS X: %.3d", x_pos);
				ILI9341_Draw_Text(counter_buff, 10, 80, BLACK, 2, WHITE);
				sprintf(counter_buff, "POS Y: %.3d", y_pos);
				ILI9341_Draw_Text(counter_buff, 10, 120, BLACK, 2, WHITE);
				ILI9341_Set_Rotation(SCREEN_VERTICAL_1);
			}
			ILI9341_Draw_Pixel(x_pos, y_pos, BLACK);
		} else {
			HAL_GPIO_WritePin(GPIOB, LD3_Pin | LD2_Pin, GPIO_PIN_RESET);
		}

	}
}

void testPerformance() {
	uint32_t Timer_Counter = 0;
	for (uint32_t j = 0; j < 2; j++) {
		HAL_TIM_Base_Start(&htim1);
		for (uint16_t i = 0; i < 10; i++) {
			fillScreenColor(WHITE);
			fillScreenColor(BLACK);
		}

		// 20.000 per second!
		HAL_TIM_Base_Stop(&htim1);
		Timer_Counter += __HAL_TIM_GET_COUNTER(&htim1);
		__HAL_TIM_SET_COUNTER(&htim1, 0);
	}

	Timer_Counter /= 2;

	char counter_buff[30];
	fillScreenColor(GREENYELLOW);
	setRotation(SCREEN_HORIZONTAL_1);
	sprintf(counter_buff, "Timer counter value: %ld", Timer_Counter * 2);
	drawText(counter_buff, 10, 10, 2);

	double seconds_passed = 2 * ((float) Timer_Counter / 20000);
	sprintf(counter_buff, "Time: %.3f Sec", seconds_passed);
	ILI9341_Draw_Text(counter_buff, 10, 30, BLACK, 2, WHITE);

	double timer_float = 20 / (((float) Timer_Counter) / 20000); // Frames per sec

	sprintf(counter_buff, "FPS:  %.2f", timer_float);
	ILI9341_Draw_Text(counter_buff, 10, 50, BLACK, 2, WHITE);
	double MB_PS = timer_float * 240 * 320 * 2 / 1000000;
	sprintf(counter_buff, "MB/S: %.2f", MB_PS);
	ILI9341_Draw_Text(counter_buff, 10, 70, BLACK, 2, WHITE);
	double SPI_utilized_percentage = (MB_PS / (6.25)) * 100; // 50mbits / 8 bits
	sprintf(counter_buff, "SPI Utilized: %.2f", SPI_utilized_percentage);
	ILI9341_Draw_Text(counter_buff, 10, 90, BLACK, 2, WHITE);
	HAL_Delay(10000);
}

void getTemperature(double *temperature) {
	// In celcius
}

void getTemperatureString(double temperature, char *temperatureString) {
	sprintf(temperatureString, "%.1f C", temperature);
}

void getHumidity(double *humidity) {
	// In Relative Humidity, %RH
}

void getHumidityString(double humidity, char *humidityString) {
	sprintf(humidityString, "%.1f %%RH", humidity);
}

void drawHueCircle(uint16_t color) {
	Circle circle = { SCREEN_WIDTH / 2 - 25, 45, 25 };
	drawFilledCircleByCoord(circle, color);
}

uint16_t convertColorToRgb565(float redIntensity, float greenIntensity, float blueIntensity) {
  uint16_t red = (uint16_t)(redIntensity * 31.0 + 0.5);
  uint16_t green = (uint16_t)(greenIntensity * 63.0 + 0.5);
  uint16_t blue = (uint16_t)(blueIntensity * 31.0 + 0.5);

  uint16_t color = (red << 11) | (green << 5) | blue;

  return color;
}

void drawRgbInfo(ColorIntensity redColor, ColorIntensity greenColor, ColorIntensity blueColor) {
	const uint16_t BRIGHT_RED = 0xff3c;
	const uint16_t BRIGHT_GREEN = 0xcff9;
	const uint16_t BRIGHT_BLUE = 0xe73f;
	const uint16_t BOX_WIDTH = 150;
	const uint16_t BOX_HEIGHT = 20;
	const uint16_t GAP_X_CIRCLE_AND_BOX = 15;

	Rectangle redBox =
	{
			getCircleEdgeX(redColor.circle) + GAP_X_CIRCLE_AND_BOX,
			redColor.circle.y - (redColor.circle.radius / 2),
			getCircleEdgeX(redColor.circle) + GAP_X_CIRCLE_AND_BOX + BOX_WIDTH,
			redColor.circle.y + BOX_HEIGHT
	};

	Rectangle greenBox =
	{
			getCircleEdgeX(greenColor.circle) + GAP_X_CIRCLE_AND_BOX,
			greenColor.circle.y - (greenColor.circle.radius / 2),
			getCircleEdgeX(greenColor.circle) + GAP_X_CIRCLE_AND_BOX + BOX_WIDTH,
			greenColor.circle.y + BOX_HEIGHT
	};

	Rectangle blueBox =
	{
			getCircleEdgeX(blueColor.circle) + GAP_X_CIRCLE_AND_BOX,
			blueColor.circle.y - (blueColor.circle.radius / 2),
			getCircleEdgeX(blueColor.circle) + GAP_X_CIRCLE_AND_BOX + BOX_WIDTH,
			blueColor.circle.y + BOX_HEIGHT
	};

	Rectangle redIntensityBar =
	{
			redBox.x0, redBox.y0,
			redBox.x0 + (int) (redColor.intensity*BOX_WIDTH) , redBox.y1
	};
	Rectangle greenIntensityBar =
	{
			greenBox.x0, greenBox.y0,
			greenBox.x0 + (int) (greenColor.intensity*BOX_WIDTH) , greenBox.y1
	};
	Rectangle blueIntensityBar =
	{
			blueBox.x0, blueBox.y0,
			blueBox.x0 + (int) (blueColor.intensity*BOX_WIDTH) , blueBox.y1
	};


	const uint8_t COLOR_PERCENTAGE_FONT_SIZE = 2;
	const uint8_t GAP_X_BOX_AND_PERCENTAGE = 10;

	Point redPercentagePos = {redBox.x1 + GAP_X_BOX_AND_PERCENTAGE, redBox.y0};
	Point greenPercentagePos = {greenBox.x1 + GAP_X_BOX_AND_PERCENTAGE, greenBox.y0};
	Point bluePercentagePos = {blueBox.x1 + GAP_X_BOX_AND_PERCENTAGE, blueBox.y0};

//	fillScreenColor(WHITE);

	drawHueCircle(convertColorToRgb565(redColor.intensity, greenColor.intensity, blueColor.intensity));

	drawFilledCircleByCoord(redColor.circle, RED);
	drawFilledCircleByCoord(greenColor.circle, GREEN);
	drawFilledCircleByCoord(blueColor.circle, BLUE);

	drawFilledRectangleByCoord(redBox, BRIGHT_RED);
	drawFilledRectangleByCoord(greenBox, BRIGHT_GREEN);
	drawFilledRectangleByCoord(blueBox, BRIGHT_BLUE);

	drawFilledRectangleByCoord(redIntensityBar, RED);
	drawFilledRectangleByCoord(greenIntensityBar, GREEN);
	drawFilledRectangleByCoord(blueIntensityBar, BLUE);

	char text[10];
	Rectangle area = {redPercentagePos.x, redPercentagePos.y, bluePercentagePos.x + 50, bluePercentagePos.y + 50};
	clearScreenArea(area, WHITE);

	sprintf(text, "%d", (int) (redColor.intensity * 100));
	drawTextByPoint(text, redPercentagePos, COLOR_PERCENTAGE_FONT_SIZE);

	sprintf(text, "%d", (int) (greenColor.intensity * 100));
	drawTextByPoint(text, greenPercentagePos, COLOR_PERCENTAGE_FONT_SIZE);

	sprintf(text, "%d", (int) (blueColor.intensity * 100));
	drawTextByPoint(text, bluePercentagePos, COLOR_PERCENTAGE_FONT_SIZE);
}

void drawColorIntensityPercentage()
{

}

void checkTouchHueCircle(ColorIntensity *redColor, ColorIntensity *greenColor, ColorIntensity *blueColor)
{
	uint16_t xPos = 0;
	uint16_t yPos = 0;

	uint16_t redXPositive = getCircleEdgeX(redColor->circle);
	uint16_t redXNegative = getCircleEdgeXNegative(redColor->circle);
	uint16_t redYPositive = getCircleEdgeY(redColor->circle);
	uint16_t redYNegative = getCircleEdgeYNegative(redColor->circle);

	uint16_t greenXPositive = getCircleEdgeX(greenColor->circle);
	uint16_t greenXNegative = getCircleEdgeXNegative(greenColor->circle);
	uint16_t greenYPositive = getCircleEdgeY(greenColor->circle);
	uint16_t greenYNegative = getCircleEdgeYNegative(greenColor->circle);

	uint16_t blueXPositive = getCircleEdgeX(blueColor->circle);
	uint16_t blueXNegative = getCircleEdgeXNegative(blueColor->circle);
	uint16_t blueYPositive = getCircleEdgeY(blueColor->circle);
	uint16_t blueYNegative = getCircleEdgeYNegative(blueColor->circle);

	while (1) {
		HAL_Delay(20);

		if (TP_Touchpad_Pressed()) {

			uint16_t position_array[2];

			if (TP_Read_Coordinates(position_array) == TOUCHPAD_DATA_OK)
			{
				if (SCREEN_ROTATION == SCREEN_HORIZONTAL_1)
				{
					xPos = position_array[1];
					yPos = SCREEN_HEIGHT - position_array[0];
				} else if (SCREEN_ROTATION == SCREEN_HORIZONTAL_2)
				{
					xPos = SCREEN_WIDTH - position_array[1];
					yPos = position_array[0];
				}
			}


			if (xPos > redXNegative && xPos < redXPositive && yPos > redYNegative && yPos < redYPositive)
			{
				if (redColor->intensity >= 1.0) {
					redColor->intensity = 0.0;
				} else {
					redColor->intensity += 0.2;
				}
			} else if (xPos > greenXNegative && xPos < greenXPositive && yPos > greenYNegative && yPos < greenYPositive)
			{
				if (greenColor->intensity >= 1.0) {
					greenColor->intensity = 0.0;
				} else {
					greenColor->intensity += 0.2;
				}
			} else if (xPos > blueXNegative && xPos < blueXPositive && yPos > blueYNegative && yPos < blueYPositive)
			{
				if (blueColor->intensity >= 1.0) {
					blueColor->intensity = 0.0;
				} else {
					blueColor->intensity += 0.2;
				}
			}
			drawRgbInfo(*redColor, *greenColor, *blueColor);
//			sprintf(text, "Touch: (%d, %d)", xPos, yPos);
//			drawText(text, 10, 10, 2);
		}
	}
}

void drawColorInfoPage()
{

}

void drawStudentInfoPage()
{
//	char *picArray;
	fillScreenColor(WHITE);
//	drawImage(picArray, SCREEN_HORIZONTAL_1);

	char *group = "G03";
	char *firstName = "Alice";
	char *lastName = "Brabra";

	int fontSize = 2;
	Point infoPos = {SCREEN_WIDTH/2 + 10, 20};
	const int LINE_SPACEING_SIZE = 20;

	drawText(group, infoPos.x, infoPos.y , fontSize);
	drawText(lastName, infoPos.x, infoPos.y + LINE_SPACEING_SIZE, fontSize);
	drawText(lastName, infoPos.x, infoPos.y + LINE_SPACEING_SIZE, fontSize);
	drawText(lastName, infoPos.x, infoPos.y + LINE_SPACEING_SIZE, fontSize);
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* Enable I-Cache---------------------------------------------------------*/
	SCB_EnableICache();

	/* Enable D-Cache---------------------------------------------------------*/
	SCB_EnableDCache();

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART3_UART_Init();
	MX_SPI5_Init();
	MX_TIM1_Init();
	MX_RNG_Init();
	/* USER CODE BEGIN 2 */
	ILI9341_Init(); // initial driver setup to drive ili9341

	double temperature = 25.8;
	double humidity = 55.6;

	char temperatureString[10];
	char humidityString[10];

	uint16_t radius = 20;

	ColorIntensity redColor = {{ 30, SCREEN_HEIGHT / 4 + 35, radius }, 0};
	ColorIntensity greenColor = {{ 30, getCircleEdgeY(redColor.circle) + radius + 15, radius }, 0};
	ColorIntensity blueColor = {{ 30, getCircleEdgeY(greenColor.circle) + radius + 15, radius }, 0};

	getTemperature(&temperature);
	getTemperatureString(temperature, temperatureString);

	getHumidity(&humidity);
	getHumidityString(humidity, humidityString);

	Point temperaturePosition = { 25, 30 };
	Point humidityPosition = { SCREEN_WIDTH / 2 + 10, 30 };

	int fontSize = 2;

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		fillScreenColor(WHITE);
		setRotation(SCREEN_ROTATION);
		drawTextByPoint(temperatureString, temperaturePosition, fontSize);
		drawHueCircle(convertColorToRgb565(redColor.intensity, greenColor.intensity, blueColor.intensity));
		drawTextByPoint(humidityString, humidityPosition, fontSize);
		drawRgbInfo(redColor, greenColor, blueColor);

		checkTouchHueCircle(&redColor, &greenColor, &blueColor);
		HAL_Delay(5000);

//		drawStudentInfoPage();
//		HAL_Delay(5000);

	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 200;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Activate the Over-Drive mode
	 */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
