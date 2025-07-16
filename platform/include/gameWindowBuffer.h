#pragma once
#include <Windows.h>
#include "memoryStuff.h"



struct GameWindowBuffer
{
	uint16_t memory[SCREEN_BUFFER_MEMORY / 2];
	int w = 0;
	int h = 0;


	void drawAtSafe(int x, int y, unsigned char r, unsigned char g, unsigned char b)
	{
		if (x >= w || y >= h || x < 0 || y < 0)
			return;

		memory[x + y * w] = packRGB565(r, g, b);
	}

	void clear(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0)
	{
		uint16_t color = packRGB565(r, g, b);
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x)
				memory[x + y * w] = color;
	}
};


void resetWindowBuffer(GameWindowBuffer *gameWindowBuffer, BITMAPINFO *bitmapInfo, HWND wind);








