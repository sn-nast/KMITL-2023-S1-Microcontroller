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
#include "adc.h"
#include "i2c.h"
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

#include "string.h"
#include "Utilities/my_lcd.h"
#include "Utilities/my_basic.h"
#include "Utilities/my_picture.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
typedef struct _ColorInfo
{
	Circle circle;
	uint16_t color;
	float intensity;
} ColorInfo ;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
const uint8_t SCREEN_ROTATION = SCREEN_HORIZONTAL_1;
uint32_t count = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void getTemperature(float *temperature) {
	// In celcius
}

void getTemperatureString(float temperature, char *temperatureString) {
	sprintf(temperatureString, "%.1f C", temperature);
}

void getHumidity(float *humidity) {
	// In Relative Humidity, %RH
}

void getHumidityString(float humidity, char *humidityString) {
	sprintf(humidityString, "%.1f %%RH", humidity);
}

uint16_t convertColorToRgb565(float redIntensity, float greenIntensity, float blueIntensity) {
  uint16_t red = (uint16_t)(redIntensity * 31.0 + 0.5);
  uint16_t green = (uint16_t)(greenIntensity * 63.0 + 0.5);
  uint16_t blue = (uint16_t)(blueIntensity * 31.0 + 0.5);

  uint16_t color = (red << 11) | (green << 5) | blue;

  return color;
}

Rectangle createColorBox(Circle circle)
{
	const uint16_t BOX_WIDTH = 150;
	const uint16_t BOX_HEIGHT = 20;
	const uint16_t GAP_X_CIRCLE_AND_BOX = 15;

	Rectangle rectangle = {
			getCircleEdgeX(circle) + GAP_X_CIRCLE_AND_BOX,
			circle.y - (circle.radius / 2),
			getCircleEdgeX(circle) + GAP_X_CIRCLE_AND_BOX + BOX_WIDTH,
			circle.y + BOX_HEIGHT
	};
	return rectangle;
}

Rectangle createColorIntensityBar(Rectangle refRectangle, float colorIntensity)
{
	const uint16_t BOX_WIDTH = 150;

	Rectangle intensityBar = {
			refRectangle.x0,
			refRectangle.y0,
			refRectangle.x0 + (int) (colorIntensity * BOX_WIDTH),
			refRectangle.y1
	};
	return intensityBar;
}

void drawRgbInfo(ColorInfo redColor, ColorInfo greenColor, ColorInfo blueColor, ColorInfo mixedColor) {
	const uint16_t BRIGHT_RED = 0xff3c;
	const uint16_t BRIGHT_GREEN = 0xcff9;
	const uint16_t BRIGHT_BLUE = 0xe73f;

	Rectangle redBox = createColorBox(redColor.circle);
	Rectangle greenBox = createColorBox(greenColor.circle);
	Rectangle blueBox = createColorBox(blueColor.circle);

	Rectangle redIntensityBar = createColorIntensityBar(redBox, redColor.intensity);
	Rectangle greenIntensityBar = createColorIntensityBar(greenBox, greenColor.intensity);
	Rectangle blueIntensityBar = createColorIntensityBar(blueBox, blueColor.intensity);

	const uint8_t GAP_X_BOX_AND_PERCENTAGE = 10;

	Point redPercentagePos = {redBox.x1 + GAP_X_BOX_AND_PERCENTAGE, redBox.y0};
	Point greenPercentagePos = {greenBox.x1 + GAP_X_BOX_AND_PERCENTAGE, greenBox.y0};
	Point bluePercentagePos = {blueBox.x1 + GAP_X_BOX_AND_PERCENTAGE, blueBox.y0};

	drawFilledCircleAtCoord(redColor.circle, redColor.color);
	drawFilledCircleAtCoord(greenColor.circle, greenColor.color);
	drawFilledCircleAtCoord(blueColor.circle, blueColor.color);

	drawFilledCircleAtCoord(mixedColor.circle, mixedColor.color);

	drawFilledRectangleAtCoord(redBox, BRIGHT_RED);
	drawFilledRectangleAtCoord(greenBox, BRIGHT_GREEN);
	drawFilledRectangleAtCoord(blueBox, BRIGHT_BLUE);

	drawFilledRectangleAtCoord(redIntensityBar, RED);
	drawFilledRectangleAtCoord(greenIntensityBar, GREEN);
	drawFilledRectangleAtCoord(blueIntensityBar, BLUE);

	char text[10];
	const uint8_t COLOR_PERCENTAGE_FONT_SIZE = 2;

	Rectangle clearArea = {redPercentagePos.x, redPercentagePos.y, bluePercentagePos.x + 50, bluePercentagePos.y + 50};
	clearScreenArea(clearArea, WHITE);

	sprintf(text, "%d", (int) (redColor.intensity * 100));
	drawTextAtPoint(text, redPercentagePos, COLOR_PERCENTAGE_FONT_SIZE);

	sprintf(text, "%d", (int) (greenColor.intensity * 100));
	drawTextAtPoint(text, greenPercentagePos, COLOR_PERCENTAGE_FONT_SIZE);

	sprintf(text, "%d", (int) (blueColor.intensity * 100));
	drawTextAtPoint(text, bluePercentagePos, COLOR_PERCENTAGE_FONT_SIZE);
}

uint8_t isTouchWithinCircle(Circle circle, uint16_t xPos, uint16_t yPos)
{
	uint16_t xPositive = getCircleEdgeX(circle);
	uint16_t xNegative = getCircleEdgeXNegative(circle);
	uint16_t yPositive = getCircleEdgeY(circle);
	uint16_t yNegative = getCircleEdgeYNegative(circle);

	if (xPos > xNegative && xPos < xPositive && yPos > yNegative && yPos < yPositive)
	{
		return 1;
	} else
		return 0;
}

void addColorIntensity(ColorInfo *colorInfo)
{
	const float MAX_INTENSITY = 1.0;
	const float STEP_INTENSITY = 0.1;

	if (colorInfo->intensity >= MAX_INTENSITY) {
		colorInfo->intensity = 0.0;
	} else {
		colorInfo->intensity += STEP_INTENSITY;
	}
}

void getTouchPosition(uint16_t *xPos, uint16_t *yPos)
{
	uint16_t position_array[2];

	if (TP_Read_Coordinates(position_array) == TOUCHPAD_DATA_OK)
	{
		if (SCREEN_ROTATION == SCREEN_HORIZONTAL_1)
		{
			*xPos = position_array[1];
			*yPos = SCREEN_HEIGHT - position_array[0];
		} else if (SCREEN_ROTATION == SCREEN_HORIZONTAL_2)
		{
			*xPos = SCREEN_WIDTH - position_array[1];
			*yPos = position_array[0];
		}
	}
}

void checkTouchHueCircle(ColorInfo *redColor, ColorInfo *greenColor, ColorInfo *blueColor, ColorInfo *mixedColor)
{
	uint16_t xPos = 0;
	uint16_t yPos = 0;

	int isEndShowing = 0;

	while (!isEndShowing) {
		HAL_Delay(10);

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

			if (isTouchWithinCircle(redColor->circle, xPos, yPos))
			{
				addColorIntensity(redColor);
			}
			else if (isTouchWithinCircle(greenColor->circle, xPos, yPos))
			{
				addColorIntensity(greenColor);
			}
			else if (isTouchWithinCircle(blueColor->circle, xPos, yPos))
			{
				addColorIntensity(blueColor);
			} else if (isTouchWithinCircle(mixedColor->circle, xPos, yPos))
			{
				isEndShowing = 1;
			}
			mixedColor->color = convertColorToRgb565(redColor->intensity, greenColor->intensity, blueColor->intensity);

			drawRgbInfo(*redColor, *greenColor, *blueColor, *mixedColor);
		}
	}
}

void drawColorInfoPage()
{

}

void startTimer()
{
	count = 0;
	HAL_TIM_Base_Init(&htim2);
	HAL_TIM_Base_Start(&htim2);
	HAL_ADC_Start_IT(&hadc1);
}

void stopTimer()
{
	HAL_TIM_Base_DeInit(&htim2);
	HAL_ADC_Stop_IT(&hadc1);
}

void drawStudentInfoPage(ColorInfo colorInfo)
{
//	char *picArray;
//	fillScreenColor(WHITE);

	char *group = "G03";
	char *firstName = "Natchanon";
	char *lastName = "Bunyachawaset";
	char *myId = "64011113";

	Point infoPos = {SCREEN_WIDTH/2 - 50, 100};
	const int LINE_SPACEING_SIZE = 20;
	const int FONT_SIZE = 2;

	uint16_t color = colorInfo.color;

	int isEndShowing = 0;
	fillScreenColor(WHITE);
//	drawImage((const char*) testPic1, SCREEN_VERTICAL_1);
	drawTextWithColor(group, infoPos.x, infoPos.y, color, FONT_SIZE, WHITE);
	drawTextWithColor(firstName, infoPos.x, infoPos.y + LINE_SPACEING_SIZE, color, FONT_SIZE, WHITE);
	drawTextWithColor(lastName, infoPos.x, infoPos.y + 2 *LINE_SPACEING_SIZE, color, FONT_SIZE, WHITE);
	drawTextWithColor(myId, infoPos.x, infoPos.y + 3*LINE_SPACEING_SIZE, color, FONT_SIZE, WHITE);

	uint16_t xPos = 0;
	uint16_t yPos = 0;

	startTimer();
	while (!isEndShowing) {
		char txt[20];
		sprintf(txt, "Count: %d", (int) count);
		drawText(txt, 10, 10, 3);
		if (count > 4)
		{
			isEndShowing = 1;
		} else if (TP_Touchpad_Pressed())
		{
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
			isEndShowing = 1;
		}
	}
	stopTimer();
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	count++;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
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
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
	ILI9341_Init(); // initial driver setup to drive ili9341

	float temperature = 10.1;
	float humidity = 40.1;

	char temperatureString[10];
	char humidityString[10];

	uint16_t radius = 20;

	ColorInfo redColor = {{ 30, SCREEN_HEIGHT / 4 + 35, radius }, RED, 0};
	ColorInfo greenColor = {{ 30, getCircleEdgeY(redColor.circle) + radius + 15, radius }, GREEN, 0};
	ColorInfo blueColor = {{ 30, getCircleEdgeY(greenColor.circle) + radius + 15, radius }, BLUE, 0};
	ColorInfo mixedColor = {
			{ SCREEN_WIDTH / 2 - 25, 45, 25 },
			convertColorToRgb565(redColor.intensity, greenColor.intensity, blueColor.intensity),
			0
	};

	getTemperature(&temperature);
	getTemperatureString(temperature, temperatureString);

	getHumidity(&humidity);
	getHumidityString(humidity, humidityString);

	Point temperaturePosition = { 25, 30 };
	Point humidityPosition = { SCREEN_WIDTH / 2 + 10, 30 };

	int fontSize = 2;

	uint8_t cmdBuffer[3];
	uint8_t dataBuffer[8];

	char str[50];
	sprintf(str, "\n\rAM2320 I2C DEMO Starting ...");
	printOutLine(str);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		fillScreenColor(WHITE);
		setRotation(SCREEN_ROTATION);

//		drawTextByPoint(temperatureString, temperaturePosition, fontSize);
//		drawTextByPoint(humidityString, humidityPosition, fontSize);
		drawRgbInfo(redColor, greenColor, blueColor, mixedColor);
		checkTouchHueCircle(&redColor, &greenColor, &blueColor, &mixedColor);

		drawStudentInfoPage(mixedColor);

		HAL_Delay(10);
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

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
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
