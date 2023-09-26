#ifndef INC_UTILITIES_MY_LCD_H_
#define INC_UTILITIES_MY_LCD_H_

#include <stdio.h>
#include <stdint.h>

#define SCREEN_HEIGHT ILI9341_SCREEN_HEIGHT
#define SCREEN_WIDTH ILI9341_SCREEN_WIDTH

typedef struct _Point
{
	uint8_t x;
	uint8_t y;
} Point;

typedef struct _Rectangle
{
	uint16_t x0;
	uint16_t y0;
	uint16_t x1;
	uint16_t y1;
} Rectangle;

typedef struct _Circle
{
	uint16_t x;
	uint16_t y;
	uint16_t radius;
} Circle;

void fillScreenColor(uint16_t color);

void setRotation(uint8_t rotation);

void drawText(const char *text, uint8_t x, uint8_t y, uint16_t size);
void drawTextByPoint(const char *text, Point position, uint16_t size);

void drawPixel(uint16_t x, uint16_t y, uint16_t color);
void drawPixelByPoint(Point point, uint16_t color);

void drawHollowRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void drawHollowRectangleByCoord(Rectangle rectangle, uint16_t color);
void drawFilledRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void drawFilledRectangleByCoord(Rectangle rectangle, uint16_t colore);

void drawHorizontalLine(uint16_t x, uint16_t y, uint16_t width, uint16_t color);
void drawHorizontalLineByPoint(Point point, uint16_t width, uint16_t color);
void drawVerticallLine(uint16_t x, uint16_t y, uint16_t width, uint16_t color);
void drawVerticalLineByPoint(Point point, uint16_t width, uint16_t color);

void drawHollowCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color);
void drawHollowCircleByCoord(Circle circle, uint16_t color);
void drawFilledCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color);
void drawFilledCircleByCoord(Circle circle, uint16_t color);

void drawImage(const char *imageArray, uint8_t orientation);


uint16_t getCircleEdgeX(Circle circle);
uint16_t getCircleEdgeY(Circle circle);
uint16_t getCircleEdgeXNegative(Circle circle);
uint16_t getCircleEdgeYNegative(Circle circle);

#endif /* INC_UTILITIES_MY_LCD_H_ */