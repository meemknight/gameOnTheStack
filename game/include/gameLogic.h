#pragma once
#include <input.h>
#include <gameWindowBuffer.h>
#include <freeListAllocator.h>


bool initGameplay(FreeListAllocator &allocator);

bool gameplayFrame(float deltaTime, int w, int h, Input &input, 
	GameWindowBuffer &gameWindowBuffer, FreeListAllocator &allocator);

void closeGameLogic();



