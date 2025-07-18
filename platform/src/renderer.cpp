#include "renderer.h"
#include <glm/glm.hpp>
#include <algorithm>
#include "gameWindowBuffer.h"
#undef min
#undef max

#include <cmath>  // for std::isfinite

bool isFiniteVec(const glm::vec3 &v)
{
	return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

template<class T>
T min(T a, T b, T c)
{
	return std::min(std::min(a, b), c);
}

template<class T>
T max(T a, T b, T c)
{
	return std::max(std::max(a, b), c);
}

template<class T>
T min(T a, T b, T c, T d)
{
	return std::min(min(a, b, c), d);
}

template<class T>
T max(T a, T b, T c, T d)
{
	return std::max(max(a, b, c), d);
}


inline void barycentric(glm::vec2 p, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, float& u, float& v)
{
	float x = p.x;
	float y = p.y;

	float a = p0.x - p1.x;
	float b = p0.x - p2.x;
	float c = p0.x - x;
	float d = p0.y - p1.y;
	float e = p0.y - p2.y;
	float f = p0.y - y;

	float div = (a * e - b * d);

	if (div == 0)
	{
		u = 0;
		v = 0;
	}
	else
	{
		u = (c * e - b * f) / div;
		v = (f * a - d * c) / div;
	}

};

void Renderer::updateWindowMetrics(int w, int h)
{
	if (this->w == w && this->h == h) { return; }

	this->w = w; this->h = h;


}

void Renderer::clearDepth()
{
	windowBuffer->clearZ();
}

//todo move this to a stage before
auto depthCalculation = [](float z)->float
{
	float closePlane = -10; float farPlane = -40;
	float val = ((1.f / z - 1.f / closePlane) / (1.f / farPlane - 1.f / closePlane));
	//float val = (z - closePlane) / (farPlane - closePlane);

	if (val < 0 || val > 1)
	{
		return -1.f;
	}

	val = glm::clamp(val, 0.f, 1.f);
	return val;
};

/*
void Renderer::renderTriangleInClipSpace(glm::vec3 T0, glm::vec3 T1, glm::vec3 T2,
	glm::vec2 textureUV0, glm::vec2 textureUV1, glm::vec2 textureUV2,
	glm::vec3 color)
{
	glm::fvec2 clipMinF = { min(T0.x, T1.x, T2.x), min(T0.y, T1.y, T2.y) };
	glm::fvec2 clipMaxF = { max(T0.x, T1.x, T2.x), max(T0.y, T1.y, T2.y) };

	//clipMinF = glm::clamp(clipMinF, { -1,-1 }, { 1, 1 });
	//clipMaxF = glm::clamp(clipMaxF, { -1,-1 }, { 1, 1 });

	glm::ivec2 clipMin = toScreenCoords(clipMinF);
	glm::ivec2 clipMax = toScreenCoords(clipMaxF);

	clipMin = glm::clamp(clipMin, { 0,0 }, { w - 1 , h - 1 });
	clipMax = glm::clamp(clipMax, { 0,0 }, { w - 1 , h - 1 });

	//this are in -1 1 space
	float z0 = T0.z;
	float z1 = T1.z;
	float z2 = T2.z;

	//this are in pixel screen space
	glm::vec2 t0 = toScreenCoordsFloat( glm::vec2{ T0.x, T0.y });
	glm::vec2 t1 = toScreenCoordsFloat( glm::vec2{ T1.x, T1.y });
	glm::vec2 t2 = toScreenCoordsFloat( glm::vec2{ T2.x, T2.y });

	//x, y are in pixel screen space
	for (int y = clipMin.y; y <= clipMax.y; y++)
	{
		for (int x = clipMin.x; x <= clipMax.x; x++)
		{
			float u = 0;
			float v = 0;
			barycentric(glm::vec2(x, y), t0, t1, t2, u, v);

			if (u < 0 || v < 0 || u + v > 1)
			{
				continue;
			}

			glm::vec2 uvs[3] = { textureUV0, textureUV1, textureUV2 };
			float tu = (1 - u - v) * uvs[0].x + u * uvs[1].x + v * uvs[2].x;
			float tv = (1 - u - v) * uvs[0].y + u * uvs[1].y + v * uvs[2].y;

			tu *= texture.w;
			tv *= texture.h;
			int itu = floor(tu);
			int itv = floor(tv);
			unsigned char r = texture.data[(itu + itv * texture.w) * 3 + 0];
			unsigned char g = texture.data[(itu + itv * texture.w) * 3 + 1];
			unsigned char b = texture.data[(itu + itv * texture.w) * 3 + 2];


			float z = (1 - u - v) * z0 + u * z1 + v * z2; //trilinear interpolation
			//float depth = depthCalculation(z);
			float depth = z;
			//if (depth <= -1) { continue; }
			//todo clip behind camera

			//float light = color.r / 255.f;
			//float light = 1;

			if (depth < windowBuffer->getDepthUnsafe(x, y) && depth >= 0 && depth <= 1) 
			{
				windowBuffer->setDepthUnsafe(x, y, depth);
				//windowBuffer->drawAt(x, y, r * light, g * light, b * light);
				windowBuffer->drawAtUnsafe(x, y, color.r * r, color.g * g, color.b * b);
			};

		}
	}

}
*/

void Renderer::clipAndRenderTriangleInClipSpace(glm::vec4 T0, glm::vec4 T1, glm::vec4 T2,
	glm::vec2 UV0, glm::vec2 UV1, glm::vec2 UV2, glm::vec3 color)
{
	struct Vertex
	{
		glm::vec4 pos;
		glm::vec2 uv;
	};

	// The near plane in clip space: z + w >= 0
	auto isInside = [](const glm::vec4 &v)
	{
		return v.z + v.w >= 0.0f;
	};

	auto interpolate = [](const Vertex &a, const Vertex &b) -> Vertex
	{
		float tA = a.pos.z + a.pos.w;
		float tB = b.pos.z + b.pos.w;
		float t = tA / (tA - tB); // intersection t on the edge ab

		glm::vec4 pos = a.pos + t * (b.pos - a.pos);
		glm::vec2 uv = a.uv + t * (b.uv - a.uv);
		return {pos, uv};
	};

	Vertex inVerts[3] = {{T0, UV0}, {T1, UV1}, {T2, UV2}};
	std::vector<Vertex> input = {inVerts[0], inVerts[1], inVerts[2]};
	std::vector<Vertex> output;

	// Sutherland–Hodgman style clipping against z + w >= 0
	for (int i = 0; i < 3; ++i)
	{
		const Vertex &A = input[i];
		const Vertex &B = input[(i + 1) % 3];

		bool insideA = isInside(A.pos);
		bool insideB = isInside(B.pos);

		if (insideA && insideB)
		{
			// Both inside
			output.push_back(B);
		}
		else if (insideA && !insideB)
		{
			// A in, B out -> add intersection
			output.push_back(interpolate(A, B));
		}
		else if (!insideA && insideB)
		{
			// A out, B in -> add intersection and B
			output.push_back(interpolate(A, B));
			output.push_back(B);
		}
		// else: both out -> discard edge
	}

	if (output.size() < 3) return; // triangle fully clipped

	// Triangulate the polygon back into triangles (fan-style)
	const Vertex &v0 = output[0];
	for (size_t i = 1; i + 1 < output.size(); ++i)
	{
		const Vertex &v1 = output[i];
		const Vertex &v2 = output[i + 1];

		glm::vec3 p0 = glm::vec3(v0.pos) / v0.pos.w;
		glm::vec3 p1 = glm::vec3(v1.pos) / v1.pos.w;
		glm::vec3 p2 = glm::vec3(v2.pos) / v2.pos.w;

		if (!isFiniteVec(p0) || !isFiniteVec(p1) || !isFiniteVec(p2)) continue;

		// Optional: backface culling here if desired
		float area =
			p0.x * p1.y - p0.y * p1.x +
			p1.x * p2.y - p1.y * p2.x +
			p2.x * p0.y - p2.y * p0.x;

		if (area <= 0) continue;

		renderTriangleInClipSpaceOptimized(p0, p1, p2,
			v0.uv, v1.uv, v2.uv,
			color);
	}
}


void Renderer::renderTriangleInClipSpaceOptimized(glm::vec3 T0, glm::vec3 T1, glm::vec3 T2,
	glm::vec2 textureUV0, glm::vec2 textureUV1, glm::vec2 textureUV2,
	glm::vec3 color)
{
	glm::fvec2 clipMinF = {min(T0.x, T1.x, T2.x), min(T0.y, T1.y, T2.y)};
	glm::fvec2 clipMaxF = {max(T0.x, T1.x, T2.x), max(T0.y, T1.y, T2.y)};

	if (clipMaxF.x < -1 || clipMinF.x > 1 ||
		clipMaxF.y < -1 || clipMinF.y > 1) return;

	clipMinF = glm::clamp(clipMinF, { -1,-1 }, { 1, 1 });
	clipMaxF = glm::clamp(clipMaxF, { -1,-1 }, { 1, 1 });

	glm::ivec2 clipMin = toScreenCoords(clipMinF);
	glm::ivec2 clipMax = toScreenCoords(clipMaxF);

	clipMin = glm::clamp(clipMin, {0,0}, {w - 1 , h - 1});
	clipMax = glm::clamp(clipMax, {0,0}, {w - 1 , h - 1});

	int area = (clipMax.x - clipMin.x + 1) * (clipMax.y - clipMin.y + 1);
	if (area > 5000 * 5000) return; // hard stop

	//this are in -1 1 space
	float z0 = T0.z;
	float z1 = T1.z;
	float z2 = T2.z;

	//this are in pixel screen space
	glm::vec2 t0 = toScreenCoordsFloat(glm::vec2{T0.x, T0.y});
	glm::vec2 t1 = toScreenCoordsFloat(glm::vec2{T1.x, T1.y});
	glm::vec2 t2 = toScreenCoordsFloat(glm::vec2{T2.x, T2.y});

	//x, y are in pixel screen space
	for (int y = clipMin.y; y <= clipMax.y; y++)
	{
		for (int x = clipMin.x; x <= clipMax.x; x++)
		{
			float u = 0;
			float v = 0;
			barycentric(glm::vec2(x, y), t0, t1, t2, u, v);

			if (bool(u < 0) || bool(v < 0) || bool(u + v > 1))
			{
				continue;
			}

			float z = (1 - u - v) * z0 + u * z1 + v * z2; //trilinear interpolation
			float depth = z;
			bool passDepth = (bool(depth >= 0) && bool(depth <= 1) && bool(depth < windowBuffer->
				getDepthUnsafe(x, y)) );

			if (!passDepth) { continue; }

			glm::vec2 uvs[3] = {textureUV0, textureUV1, textureUV2};
			float tu = (1 - u - v) * uvs[0].x + u * uvs[1].x + v * uvs[2].x;
			float tv = (1 - u - v) * uvs[0].y + u * uvs[1].y + v * uvs[2].y;

			tu = glm::clamp(tu, 0.f, 1.f);
			tv = glm::clamp(tv, 0.f, 1.f);

			tu *= texture.w;
			tv *= texture.h;
			int itu = floor(tu);
			int itv = floor(tv);
			unsigned char r = texture.data[(itu + itv * texture.w) * 3 + 0];
			unsigned char g = texture.data[(itu + itv * texture.w) * 3 + 1];
			unsigned char b = texture.data[(itu + itv * texture.w) * 3 + 2];


			//if (depth <= -1) { continue; }
			//todo clip behind camera

			//float light = color.r / 255.f;
			//float light = 1;

			{

				//float stub = 0;
				//float *zBufferStub[2] = {&stub, &zBuffer[0]};

				windowBuffer->setDepthUnsafe(x, y, depth);

				//zBufferStub[passDepth][(x + y * (int)w)*passDepth] = depth;

				//windowBuffer->drawAt(x, y, r * light, g * light, b * light);
				windowBuffer->drawAtUnsafeConditionalSafe(x, h-y-1, color.r * r, color.g * g, color.b * b, true);
			};

		}
	}

}


glm::vec2 Renderer::toScreenCoordsFloat(glm::vec2 v)
{
	glm::vec2 sizes = { w, h };
	v += glm::vec2{ 1.f,1.f };
	v *= sizes;
	v /= 2.f;
	return v;
}


glm::ivec2 Renderer::toScreenCoords(glm::vec2 v)
{
	return toScreenCoordsFloat(v);
}

//old version
/*
void Renderer::clipAndRenderTriangleInClipSpace(glm::vec4 T0, 
	glm::vec4 T1, glm::vec4 T2, glm::vec2 textureUV0, 
	glm::vec2 textureUV1, glm::vec2 textureUV2, glm::vec3 color)
{

	auto isBadW = [](float w)
	{
		constexpr float MIN_W = 1e-4f;
		return !std::isfinite(w) || std::abs(w) < MIN_W;
	};

	if (isBadW(T0.w) || isBadW(T1.w) || isBadW(T2.w)) return;

	//discard triangles behind camera
	if (T0.z < -T0.w && T1.z < -T1.w && T2.z < -T2.w)
		return;

	glm::vec3 t0 = glm::vec3(T0) / T0.w;
	glm::vec3 t1 = glm::vec3(T1) / T1.w;
	glm::vec3 t2 = glm::vec3(T2) / T2.w;

	if (!isFiniteVec(t0) || !isFiniteVec(t1) || !isFiniteVec(t2)) return;

#pragma region backface cull

	float areaParalelogram =
		t0.x * t1.y - t0.y * t1.x +
		t1.x * t2.y - t1.y * t2.x +
		t2.x * t0.y - t2.y * t0.x;

	if (areaParalelogram <= 0) { return; }

#pragma endregion



	//renderTriangleInClipSpace(t0, t1, t2, textureUV0, textureUV1, textureUV2, color);
	renderTriangleInClipSpaceOptimized(t0, t1, t2, textureUV0, textureUV1, textureUV2, color);


}
*/

void Renderer::renderLineClipSpace(glm::vec2 p0, glm::vec2 p1, glm::vec3 color)
{

	glm::ivec2 point0 = toScreenCoords(p0);
	glm::ivec2 point1 = toScreenCoords(p1);

	int x0 = point0.x;
	int y0 = point0.y;
	int x1 = point1.x;
	int y1 = point1.y;

	bool transpose = false;
	if (std::abs(x1 - x0) < std::abs(y1 - y0))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		transpose = true;
	}

	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	int dx = x1 - x0;
	int dy = y1 - y0;
	float derror = std::abs(dy / (float)dx);
	float error = 0;
	int y = y0;

	if (transpose)
	{
		for (int x = x0; x < x1; x++)
		{
			windowBuffer->drawAtSafe(y, x, color.r * 255.f, color.g * 255.f, color.b * 255.f);
			error += derror;
			if (error > .5f)
			{
				y += (y1 > y0 ? 1 : -1);
				error -= 1.f;
			}
		}
	}
	else
	{
		for (int x = x0; x < x1; x++)
		{
			windowBuffer->drawAtSafe(x, y, color.r * 255.f, color.g * 255.f, color.b * 255.f);
			error += derror;
			if (error > .5f)
			{
				y += (y1 > y0 ? 1 : -1);
				error -= 1.f;
			}
		}
	}


}

