#pragma once
#include <Windows.h>
#include "memoryStuff.h"
#include <glm/glm.hpp>


struct GameWindowBuffer
{
	uint16_t memory[SCREEN_BUFFER_MEMORY / 2];
	uint16_t zBuffer[SCREEN_BUFFER_MEMORY / 2];

	int w = 0;
	int h = 0;

	void drawAtUnsafe(int x, int y, unsigned char r, unsigned char g, unsigned char b)
	{
		memory[x + y * w] = packRGB565(r, g, b);
	}

	void drawAtUnsafe(int x, int y, float r, float g, float b)
	{
		memory[x + y * w] = packRGB565(r*255, g * 255, b * 255);
	}

	void drawAtUnsafeConditionalSafe(int x, int y, float r, float g, float b, 
		bool condition)
	{
		if (x >= w || y >= h || x < 0 || y < 0)
			return;

		if (condition)
		{
			memory[x + y * w] = packRGB565(r, g, b);
		}
	}
	

	void drawAtSafe(int x, int y, unsigned char r, unsigned char g, unsigned char b)
	{
		if (x >= w || y >= h || x < 0 || y < 0)
			return;

		drawAtUnsafe(x, y, r, g, b);
	}

	void clear(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0)
	{
		uint16_t color = packRGB565(r, g, b);
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x)
				memory[x + y * w] = color;
	}

	void clearZ()
	{
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x)
			{
				zBuffer[x + y * w] = UINT16_MAX;
			}
	}

	float getDepthUnsafe(int x, int y)
	{
		return zBuffer[x + y * w] / (float)UINT16_MAX;
	}

	void setDepthUnsafe(int x, int y, float d)
	{
		d = glm::clamp(d, 0.f, 1.f);

		if (x >= w || y >= h) { return; }

		zBuffer[x + y * w] = d * UINT16_MAX;
	}
};


void resetWindowBuffer(GameWindowBuffer *gameWindowBuffer, BITMAPINFO *bitmapInfo, HWND wind);








