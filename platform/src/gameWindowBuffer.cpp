#include <gameWindowBuffer.h>
#include <assert.h>


void resetWindowBuffer(GameWindowBuffer *gameWindowBuffer, BITMAPINFO *bitmapInfo, HWND wind)
{
	RECT rect = {};
	GetClientRect(wind, &rect);
	gameWindowBuffer->h = rect.bottom / 2;
	gameWindowBuffer->w = rect.right / 2;

	size_t rez = 2 * gameWindowBuffer->w * gameWindowBuffer->h;
	size_t max = SCREEN_BUFFER_MEMORY;
	if (2 * (gameWindowBuffer->w) * (gameWindowBuffer->h) > SCREEN_BUFFER_MEMORY)
	{
		
		assert(0);
		exit(1);
	}

	bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFO);
	bitmapInfo->bmiHeader.biWidth = gameWindowBuffer->w;
	bitmapInfo->bmiHeader.biHeight = -gameWindowBuffer->h;
	bitmapInfo->bmiHeader.biPlanes = 1;
	bitmapInfo->bmiHeader.biBitCount = 16;
	bitmapInfo->bmiHeader.biCompression = BI_RGB;
}


