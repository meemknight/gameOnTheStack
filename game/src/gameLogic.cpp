#include <gameLogic.h>
#include <iostream>
#include <Windows.h>
#include <memoryStuff.h>
#include <stb_image/stb_image.h>
#include <renderer.h>
#include <camera.h>
#include <OBJ_Loader.h>
#include <optional>
#include <glm/gtc/matrix_transform.hpp>

struct GameData 
{
	int positionX = 0;
	int positionY = 0;
	int fpsCounter = 0;
	float timer = 0;

	Renderer renderer;

};

static GameData data;
gl3d::Camera camera;
std::optional<objl::Loader> ground;
std::optional<objl::Loader> treeModel;
std::optional<objl::Loader> slender;

bool initGameplay(FreeListAllocator &allocator)
{

	data = {};

	ground = objl::Loader{};
	ground->LoadFile(RESOURCES_PATH "plane2.obj");

	treeModel = objl::Loader{};
	treeModel->LoadFile(RESOURCES_PATH "tree.obj");

	slender = objl::Loader{};
	slender->LoadFile(RESOURCES_PATH "slender.obj");

	stbi_set_flip_vertically_on_load(true);
	//data.renderer.texture.data = stbi_load(RESOURCES_PATH "african_head_diffuse.tga",
	//	&data.renderer.texture.w, &data.renderer.texture.h, nullptr, 3);

	camera.position = {1,2.4,1};


	return true;
}

void renderModel(objl::Loader &model, int w, int h, glm::mat4 &pviewProjMat, glm::vec3 light_dir)
{

	//for (int i = 0; i < model->LoadedIndices.size() / 3; i++)
	for (auto &m : model.LoadedMeshes)
	{
		auto &indices = m.Indices;
		auto &vertices = m.Vertices;
		auto &material = model.LoadedMaterials[m.materialIndex];

		glm::vec3 materialColor(material.Kd.X, material.Kd.Y, material.Kd.Z);
		//materialColor *= 2.f;
		//materialColor = glm::pow(materialColor, glm::vec3(0.5f));
		materialColor = glm::sqrt(materialColor);
		//materialColor = glm::vec3(1.f);

		for (int i = 0; i < m.Indices.size() / 3; i++)
		{


			int face[3] = {(int)indices[i * 3], (int)indices[i * 3 + 1], (int)indices[i * 3] + 2};
			glm::vec3 screen_coords[3];
			glm::vec3 world_coords[3];
			glm::vec4 projected_coords[3];
			glm::vec2 textureUVs[3];

			for (int j = 0; j < 3; j++)
			{
				auto v = vertices[face[j]];
				v.Position.X *= 2.f;
				v.Position.Y *= 2.f;
				v.Position.Z *= 2.f;
				v.Position.Z -= 1.5;

				screen_coords[j] = glm::vec3((v.Position.X + 1.f) * w / 2.f, (v.Position.Y + 1.f) * h / 2.f, v.Position.Z);
				screen_coords[j].x = floor(screen_coords[j].x);
				screen_coords[j].y = floor(screen_coords[j].y);
				world_coords[j] = {v.Position.X, v.Position.Y, v.Position.Z};

				//ztextureUVs[j].x = v.TextureCoordinate.X;
				//ztextureUVs[j].y = v.TextureCoordinate.Y;

				projected_coords[j] = glm::vec4(world_coords[j], 1);

				projected_coords[j] = pviewProjMat * projected_coords[j];

			}


			//if (intensity > 0)
			//{
			//	triangle2(screen_coords[0], screen_coords[1], screen_coords[2],
			//		glm::vec3(intensity * 255, intensity * 255, intensity * 255),
			//		textureUVs[0], textureUVs[1], textureUVs[2] );
			//
			//}else
			//{
			//	//triangle(screen_coords[0], screen_coords[1], screen_coords[2], glm::vec3(0));
			//}

			glm::vec3 n = glm::cross((world_coords[2] - world_coords[0]), (world_coords[1] - world_coords[0]));
			//Vec3f n = (world_coords[1] - world_coords[0]) ^ (world_coords[2] - world_coords[0]);
			n = glm::normalize(n);
			float intensity = glm::clamp(glm::dot(n, light_dir), 0.1f, 1.f);
			//if(intensity > 0)
			{

				data.renderer.clipAndRenderTriangleInClipSpace(projected_coords[0], projected_coords[1], projected_coords[2],
					textureUVs[0], textureUVs[1], textureUVs[2],
					glm::vec3(intensity) * materialColor);

				//mem->renderer.renderLineClipSpace(projected_coords[0], projected_coords[1], glm::vec3(1.f));
				//mem->renderer.renderLineClipSpace(projected_coords[1], projected_coords[2], glm::vec3(1.f));
				//mem->renderer.renderLineClipSpace(projected_coords[2], projected_coords[0], glm::vec3(1.f));

			}

			//if(i > 1000)
			//break;
			//for(int i=0;i<3;i++)
			//line(Vec2i(screen_coords[i].x, screen_coords[i].y), Vec2i(screen_coords[(i+1)%3].x, screen_coords[(i + 1) % 3].y), {255,255,255});

		}

	}

}

glm::vec2 hash2DToVec2(glm::vec2 p)
{
	// Large primes to decorrelate axes
	const float a = 127.1f;
	const float b = 311.7f;
	const float c = 269.5f;
	const float d = 183.3f;

	// Dot products create pseudo-randomness
	float n1 = glm::dot(p, glm::vec2(a, b));
	float n2 = glm::dot(p, glm::vec2(c, d));

	// Sine-based hash (cheap and deterministic)
	float x = glm::fract(glm::sin(n1) * 43758.5453f) * 2.0f - 1.0f;
	float y = glm::fract(glm::sin(n2) * 43758.5453f) * 2.0f - 1.0f;

	return glm::vec2(x, y);
}

bool gameplayFrame(float deltaTime, 
	int w, int h, Input &input, GameWindowBuffer &gameWindowBuffer, FreeListAllocator &allocator)
{

	data.renderer.updateWindowMetrics(w, h);
	data.renderer.updateWindowBuffer(&gameWindowBuffer);
	data.renderer.clearDepth();

	//fps counter
	//Sleep(1);
	data.fpsCounter += 1;
	data.timer += deltaTime;
	if (data.timer > 1)
	{
		data.timer -= 1;
		//std::cout << "FPS: " << data.fpsCounter << '\n';
		data.fpsCounter = 0;

		printStackUsage();
		size_t availableMemory = 0;
		size_t biggestBlock = 0;
		int freeBlocks = 0;
		allocator.calculateMemoryMetrics(availableMemory, biggestBlock, freeBlocks);

		std::cout << "Custom allocator usage: " << (1.f - ((float)availableMemory / allocator.getMemorySize())) * 100 << "% -> "
			<< (1.f - ((float)availableMemory / allocator.getMemorySize())) * 100 * 0.39 << "% of stack\n\n";

	}

	//clear screen
	gameWindowBuffer.clear();


	//draw rectangle
	//for (int i = 0; i < 100; i++)
	//	for (int j = 0; j < 100; j++)
	//	{
	//		gameWindowBuffer.drawAtSafe(i, j, 0, 255, 255);
	//	}


	//for (int i = 0; i < gameWindowBuffer.w; i++)
	//	for (int j = 0; j < gameWindowBuffer.h; j++)
	//	{
	//		gameWindowBuffer.drawAtSafe(i, j, i%256, j%256, (i*j)%256);
	//	}


#pragma region camera

	float speed = 1.2 * deltaTime;

	glm::vec3 dir = {};
	if (input.keyBoard[Button::W].held)
	{
		dir.z -= speed;
	}
	if (input.keyBoard[Button::S].held)
	{
		dir.z += speed;
	}
	if (input.keyBoard[Button::A].held)
	{
		dir.x -= speed;
	}
	if (input.keyBoard[Button::D].held)
	{
		dir.x += speed;
	}

	if (input.keyBoard[Button::Q].held)
	{
		dir.y -= speed;
	}
	if (input.keyBoard[Button::E].held)
	{
		dir.y += speed;
	}

	camera.moveFPS(dir);

	{
		static glm::dvec2 lastMousePos = {};
		if (input.rMouseButton.held)
		{
			glm::dvec2 currentMousePos = {};
			currentMousePos.x = input.cursorX;
			currentMousePos.y = input.cursorY;

			float speed = 0.8f;

			glm::vec2 delta = lastMousePos - currentMousePos;
			delta *= speed * deltaTime;

			camera.rotateCamera(delta);

			lastMousePos = currentMousePos;
		}
		else
		{
			lastMousePos.x = input.cursorX;
			lastMousePos.y = input.cursorY;

		}
	}

	camera.aspectRatio = (float)w / h;

#pragma endregion



	glm::vec3 light_dir = glm::normalize(glm::vec3(-0.5, -1, 0.1));


	glm::mat4 projMat = camera.getProjectionMatrix() * camera.getWorldToViewMatrix();

	//for (int h = 0; h < gameWindowBuffer.h; h++)
	//	for (int w = 0; w < gameWindowBuffer.w; w++)
	//	{
	//		gameWindowBuffer.drawAtUnsafe(w, h, 0.01f, 0.01, 0.01);
	//	}


	for (int x = -5; x < 5; x++)
		for (int y = -5; y < 5; y++)
		{

			glm::vec2 dither = hash2DToVec2({x, y});

			auto newMat = glm::translate(projMat, {x+ dither.x,0,y + dither.y});

			renderModel(*treeModel, w, h, newMat, light_dir);

		}
	

	{
		auto newMat = glm::scale(projMat, {1,1,1});
		renderModel(*ground, w, h, newMat, light_dir);
	}

	{
		auto newMat = glm::scale(projMat, glm::vec3(0.1f));
			 newMat = glm::translate(newMat, glm::vec3(-4,2,-0.5));
		renderModel(*slender, w, h, newMat, light_dir);
	}

	//for (int i = 0; i < gameWindowBuffer.w; i++)
	//	for (int j = 0; j < gameWindowBuffer.h; j++)
	//	{
	//		gameWindowBuffer.drawAtSafe(i, j, 205,205,205);
	//	}


	return true;
}

void closeGameLogic()
{
}





