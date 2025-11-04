#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <ivec.h>

struct CpuTexture
{
	int w = 0, h = 0;
	const unsigned char* data = 0;
};

struct GameWindowBuffer;

struct Renderer
{
	//float w = 0;  //todo remove
	//float h = 0;
	GameWindowBuffer *windowBuffer;
	CpuTexture texture;

	Renderer() {};

	void updateWindowMetrics(int w, int h);
	void updateWindowBuffer(GameWindowBuffer* b) { windowBuffer = b; }
	void clearDepth();

	void renderTriangleInClipSpace(glm::vec3 T0, glm::vec3 T1,
		glm::vec3 T2,
		glm::vec2 textureUV0, glm::vec2 textureUV1, glm::vec2 textureUV2,
		glm::vec3 color);

	void renderTriangleInClipSpaceOptimized(glm::vec3 T0, glm::vec3 T1,
		glm::vec3 T2,
		glm::vec2 textureUV0, glm::vec2 textureUV1, glm::vec2 textureUV2,
		glm::vec3 color);

	void clipAndRenderTriangleInClipSpace(glm::vec4 T0, glm::vec4 T1,
		glm::vec4 T2,
		glm::vec2 textureUV0, glm::vec2 textureUV1, glm::vec2 textureUV2,
		glm::vec3 color
	);



	void renderLineClipSpace(glm::vec2 p0, glm::vec2 p1, glm::vec3 color);



	glm::ivec2 toScreenCoords(glm::vec2 v);
	glm::vec2 toScreenCoordsFloat(glm::vec2 v);

};

