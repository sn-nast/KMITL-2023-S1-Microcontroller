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
const uint16_t BRIGHT_RED = 0xff3c;
const uint16_t BRIGHT_GREEN = 0xcff9;
const uint16_t BRIGHT_BLUE = 0xe73f;
float humidity=31.0, temperature=32.0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void getTemperature(float *temperature) {
	// In celcius
	if (*temperature > 100.0)
	{
		*temperature = 0.5;
	} else
	{
		*temperature += 20.0;
	}
}

void getTemperatureText(float temperature, char *temperatureString) {
	sprintf(temperatureString, "%.1f C", temperature);
}

void getHumidity(float *humidity) {
	// In Relative Humidity, %RH
}

void getHumidityText(float humidity, char *humidityString) {
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

void drawColorIntensityBar(Rectangle colorBox, ColorInfo colorInfo, uint16_t brightColor)
{

	Rectangle colorIntensityBar = createColorIntensityBar(colorBox, colorInfo.intensity);

	drawFilledRectangleAtCoord(colorBox, brightColor);
	drawFilledRectangleAtCoord(colorIntensityBar, colorInfo.color);
}

void drawColorIntensityPercentageText(Rectangle colorBox, ColorInfo colorInfo)
{
	char text[10];
	const uint8_t GAP_X_BOX_AND_PERCENTAGE = 10;
	const uint8_t COLOR_PERCENTAGE_FONT_SIZE = 2;

	Point percentagePoint = {colorBox.x1 + GAP_X_BOX_AND_PERCENTAGE, colorBox.y0};
	Rectangle clearArea = {percentagePoint.x, percentagePoint.y, percentagePoint.x + 50, percentagePoint.y + 20};

	sprintf(text, "%d#", (int) (colorInfo.intensity * 100));

	clearScreenArea(clearArea, WHITE);
	drawTextAtPoint(text, percentagePoint, COLOR_PERCENTAGE_FONT_SIZE);
}

void drawHueCircle(ColorInfo mixedColorInfo)
{
	drawFilledCircleAtCoord(mixedColorInfo.circle, mixedColorInfo.color);
}

void drawColorInfo(ColorInfo colorInfo, uint16_t brightColor)
{
	Rectangle boxArea = createColorBox(colorInfo.circle);
	drawFilledCircleAtCoord(colorInfo.circle, colorInfo.color);
	drawColorIntensityBar(boxArea, colorInfo, brightColor);
	drawColorIntensityPercentageText(boxArea, colorInfo);
}

void drawRgbInfo(ColorInfo redColor, ColorInfo greenColor, ColorInfo blueColor, ColorInfo mixedColor)
{
	drawHueCircle(mixedColor);
	drawColorInfo(redColor, BRIGHT_RED);
	drawColorInfo(greenColor, BRIGHT_GREEN);
	drawColorInfo(blueColor, BRIGHT_BLUE);
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
	{
		return 0;
	}
}

uint8_t isTouchWithinRectangle(Rectangle rectangle, uint16_t xPos, uint16_t yPos)
{
	if (xPos >= rectangle.x0 && xPos <= rectangle.x1 && yPos <= rectangle.y1 && yPos >= rectangle.y0)
	{
		return 1;
	} else
	{
		return 0;
	}

}

void addColorIntensity(ColorInfo *colorInfo)
{
	const float MAX_INTENSITY = 1.0;
	const float STEP_INTENSITY = 0.1;

	if (colorInfo->intensity >= MAX_INTENSITY) {
		colorInfo->intensity = 0.0;
	} else
	{
		colorInfo->intensity += STEP_INTENSITY;
	}
}

void drawTouchPosition(uint16_t xPos, uint16_t yPos)
{
	char text[20];
	sprintf(text, "T: (%d,%d)", xPos, yPos);
	drawText(text, 50, 180, 3);
}

void checkTouchHueCircle(ColorInfo *redColor, ColorInfo *greenColor, ColorInfo *blueColor, ColorInfo *mixedColor)
{
	uint16_t xPos = 0;
	uint16_t yPos = 0;

	int isDisplaying = 1;

	while (isDisplaying) {
		HAL_Delay(10);

		if (TP_Touchpad_Pressed())
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

			if (isTouchWithinCircle(redColor->circle, xPos, yPos))
			{
				addColorIntensity(redColor);
				drawColorInfo(*redColor, BRIGHT_RED);
			} else if (isTouchWithinCircle(greenColor->circle, xPos, yPos))
			{
				addColorIntensity(greenColor);
				drawColorInfo(*greenColor, BRIGHT_GREEN);
			} else if (isTouchWithinCircle(blueColor->circle, xPos, yPos))
			{
				addColorIntensity(blueColor);
				drawColorInfo(*blueColor, BRIGHT_BLUE);
			} else if (isTouchWithinCircle(mixedColor->circle, xPos, yPos))
			{
				isDisplaying = 0;
			}

			mixedColor->color = convertColorToRgb565(redColor->intensity, greenColor->intensity, blueColor->intensity);
			drawHueCircle(*mixedColor);
		}
	}
}

void startTimerInStudentInfoPage()
{
	count = 0;
	HAL_TIM_Base_Init(&htim2);
	HAL_TIM_Base_Start(&htim2);
	HAL_ADC_Start_IT(&hadc1);
}

void stopTimerInStudentInfoPage()
{
	HAL_TIM_Base_DeInit(&htim2);
	HAL_ADC_Stop_IT(&hadc1);
}

void startTimerOnSensor()
{
	HAL_TIM_Base_Start_IT(&htim3);
}

void stopTimerOnSensor()
{
	HAL_TIM_Base_Stop_IT(&htim3);
}

void drawColorInfoPage(ColorInfo *redColor, ColorInfo *greenColor, ColorInfo *blueColor, ColorInfo *mixedColor)
{
	fillScreenColor(WHITE);
	setRotation(SCREEN_ROTATION);

	startTimerOnSensor();
	drawRgbInfo(*redColor, *greenColor, *blueColor, *mixedColor);
	checkTouchHueCircle(redColor, greenColor, blueColor, mixedColor);
	stopTimerOnSensor();
}

void drawStudentInfoText(Point endImagePoint, uint16_t textColor)
{
	const int LINE_SPACEING_SIZE = 20;
	const int FONT_SIZE = 2;

	char *group = "Group No.3";
	char *firstName = "Natchanon";
	char *lastName = "Bunyachawaset";
	char *myId = "64011113";

	Point infoPos =
	{
			endImagePoint.x + 10,
			50
	};

	drawTextWithColor(group, infoPos.x, infoPos.y, textColor, FONT_SIZE, WHITE);
	drawTextWithColor(firstName, infoPos.x, infoPos.y + LINE_SPACEING_SIZE, textColor, FONT_SIZE, WHITE);
	drawTextWithColor(lastName, infoPos.x, infoPos.y + 2 *LINE_SPACEING_SIZE, textColor, FONT_SIZE, WHITE);
	drawTextWithColor(myId, infoPos.x, infoPos.y + 3*LINE_SPACEING_SIZE, textColor, FONT_SIZE, WHITE);
}

void drawStudentInfoPage(ColorInfo colorInfo, Image image)
{

	fillScreenColor(WHITE);
	setRotation(SCREEN_ROTATION);

	Point endImagePoint = {
			image.drawPoint.x + image.width,
			image.drawPoint.y + image.height
	};
	Rectangle imageArea = getImageArea(image);

	drawImageAtPoint(image, SCREEN_ROTATION);
	drawStudentInfoText(endImagePoint, colorInfo.color);

	uint16_t xPos = 0;
	uint16_t yPos = 0;

	startTimerInStudentInfoPage();

	int isDisplaying = 1;
	while (isDisplaying) {
		char txt[20];
		sprintf(txt, "Count: %d", (int) count);
		drawText(txt, 10, 200, 3);
		if (count > 4)
		{
			isDisplaying = 0;
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

			if (isTouchWithinRectangle(imageArea, xPos, yPos))
			{
				isDisplaying = 0;
			}
		}
	}

	stopTimerInStudentInfoPage();
}

void drawTemperatureTextAtPoint(float temperature, Point temperaturePosition)
{
	char temperatureText[10];
	Rectangle temperatureArea = { temperaturePosition.x, temperaturePosition.y, temperaturePosition.x + 82, temperaturePosition.y + 20};

	clearScreenArea(temperatureArea, WHITE);
	getTemperatureText(temperature, temperatureText);
	drawTextAtPoint(temperatureText, temperaturePosition, 2);
}

void drawHumidityTextAtPoint(float humidity, Point humidityPosition)
{
	char temperatureText[10];
	Rectangle temperatureArea = { humidityPosition.x, humidityPosition.y, humidityPosition.x + 82, humidityPosition.y + 20};

	clearScreenArea(temperatureArea, WHITE);
	getHumidityText(humidity, temperatureText);
	drawTextAtPoint(temperatureText, humidityPosition, 2);
}


void setLedBacklightIntensity(PwmInfo *ledBacklight, float dutyCycle)
{
	ledBacklight->timer->Instance->CCMR3 = (10000-1) * dutyCycle;
	HAL_TIM_PWM_Start(ledBacklight->timer, ledBacklight->timerChannel);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if(hadc == &hadc1)
	{
		count++;
	};
}

void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef *htim)
{
	static int countTimer = 0;
	static float temperature = 10.5;
	static float humidity = 20.5;

	if (htim == &htim3)
	{
		countTimer++;

		Point temperaturePosition = { 25, 30 };
		Point humidityPosition = { SCREEN_WIDTH / 2 + 10, 30 };

		getTemperature(&temperature);
		drawTemperatureTextAtPoint(temperature, temperaturePosition);

		getHumidity(&humidity);
		drawHumidityTextAtPoint(humidity, humidityPosition);
	}
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
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
	ILI9341_Init(); // initial driver setup to drive ili9341

	const uint16_t COLOR_CIRCLE_RADIUS = 22;

	ColorInfo redColor = {
			{ 30, SCREEN_HEIGHT / 4 + 35, COLOR_CIRCLE_RADIUS },
			RED, 0
	};
	ColorInfo greenColor = {
			{ 30, getCircleEdgeY(redColor.circle) + COLOR_CIRCLE_RADIUS + 15, COLOR_CIRCLE_RADIUS },
			GREEN, 0
	};
	ColorInfo blueColor = {
			{ 30, getCircleEdgeY(greenColor.circle) + COLOR_CIRCLE_RADIUS + 15, COLOR_CIRCLE_RADIUS },
			BLUE, 0
	};
	ColorInfo mixedColor = {
			{ SCREEN_WIDTH / 2 - 25, 45, 25 },
			convertColorToRgb565(redColor.intensity, greenColor.intensity, blueColor.intensity),
			0
	};

	char str[50];
	uint8_t cmdBuffer[3];
	uint8_t dataBuffer[8];

	Point pandaPicuturePoint = { 30, 30 };
	Image pandaPicture = {
			(const char*) pandaPic,
			pandaPicuturePoint,
			120,
			181
	};

	Point sunPicturePoint = {30, 30};
	Image sunPicture = {
			(const char*) sunPic,
			sunPicturePoint,
			120,
			160
	};

	sprintf(str, "\n\rAM2320 I2C DEMO Starting ...");
	printOutLine(str);

//	AM2320_setSensorValue(cmdBuffer);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//		AM2320 Sensor
//		delay(1000);
//		sprintf(str, "Temp = %4.1f \tHumidity = %4.1f", temperature, humidity);
//		printOutLine(str);
//		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
//
//		AM2320_startSensor(&hi2c1, cmdBuffer, dataBuffer);
//		AM2320_calculateValue(&temperature, &humidity, dataBuffer);

		drawColorInfoPage(&redColor, &greenColor, &blueColor, &mixedColor);
		drawStudentInfoPage(mixedColor, sunPicture);
		delay(10);
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
