#include "Utilities/my_lcd.h"

#include "ILI9341_Touchscreen.h"
#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"

void fillScreenColor(uint16_t color)
{
	ILI9341_Fill_Screen(color);
}

void setRotation(uint8_t rotation)
{
	ILI9341_Set_Rotation(rotation);
}

void drawText(const char *text, uint8_t x, uint8_t y, uint16_t size)
{
	ILI9341_Draw_Text(text, x, y, BLACK, size, WHITE);
}

void drawTextByPoint(const char *text, Point position, uint16_t size)
{
	drawText(text, position.x, position.y, size);
}

void drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
	ILI9341_Draw_Pixel(x, y, color);
}

void drawPixelByPoint(Point point, uint16_t color)
{
	ILI9341_Draw_Pixel(point.x, point.y, color);
}

// Based on ILI9341_Draw_Rectangle(x, y, width, height, color);
void drawHollowRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
	ILI9341_Draw_Hollow_Rectangle_Coord(x0, y0, x1, y1, color);
}

void drawHollowRectangleByCoord(Rectangle rectangle, uint16_t color)
{
	ILI9341_Draw_Hollow_Rectangle_Coord(rectangle.x0, rectangle.y0, rectangle.x1, rectangle.y1, color);
}

void drawFilledRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
	ILI9341_Draw_Filled_Rectangle_Coord(x0, y0, x1, y1, color);
}

void drawFilledRectangleByCoord(Rectangle rectangle, uint16_t color)
{
	ILI9341_Draw_Filled_Rectangle_Coord(rectangle.x0, rectangle.y0, rectangle.x1, rectangle.y1, color);
}

void drawHorizontalLine(uint16_t x, uint16_t y, uint16_t width, uint16_t color)
{
	ILI9341_Draw_Horizontal_Line(x, y, width, color);
}

void drawHorizontalLineByPoint(Point point, uint16_t width, uint16_t color)
{
	ILI9341_Draw_Horizontal_Line(point.x, point.y, width, color);
}

void drawVerticallLine(uint16_t x, uint16_t y, uint16_t width, uint16_t color)
{
	ILI9341_Draw_Vertical_Line(x, y, width, color);
}

void drawVerticalLineByPoint(Point point, uint16_t width, uint16_t color)
{
	ILI9341_Draw_Horizontal_Line(point.x, point.y, width, color);
}

void drawHollowCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color)
{
	ILI9341_Draw_Hollow_Circle(x, y, radius, color);
}

void drawHollowCircleByCoord(Circle circle, uint16_t color)
{
	ILI9341_Draw_Hollow_Circle(circle.x, circle.y, circle.radius, color);
}

void drawFilledCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color)
{
	ILI9341_Draw_Filled_Circle(x, y, radius, color);
}

void drawFilledCircleByCoord(Circle circle, uint16_t color)
{
	ILI9341_Draw_Filled_Circle(circle.x, circle.y, circle.radius, color);
}

void drawImage(const char *imageArray, uint8_t orientation)
{
	ILI9341_Draw_Image(imageArray, orientation);
}


uint16_t getCircleEdgeX(Circle circle)
{
	return circle.x + circle.radius;
}

uint16_t getCircleEdgeY(Circle circle)
{
	return circle.y + circle.radius;
}

uint16_t getCircleEdgeXNegative(Circle circle)
{
	return circle.x - circle.radius;
}


uint16_t getCircleEdgeYNegative(Circle circle)
{
	return circle.y - circle.radius;
}