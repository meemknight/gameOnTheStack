#include <gameLogic.h>
#include <iostream>
#include <Windows.h>
#include <memoryStuff.h>
#include <stb_image/stb_image.h>
#include <renderer.h>
#include <camera.h>
#include <OBJ_Loader.h>

struct GameData 
{
	int positionX = 0;
	int positionY = 0;
	int fpsCounter = 0;
	float timer = 0;

	Renderer renderer;

};

static GameData data;
objl::Loader model;
gl3d::Camera camera;

bool initGameplay(FreeListAllocator &allocator)
{
	data = {};


	model.LoadFile(RESOURCES_PATH "african_head.obj");

	stbi_set_flip_vertically_on_load(true);
	data.renderer.texture.data = stbi_load(RESOURCES_PATH "african_head_diffuse.tga",
		&data.renderer.texture.w, &data.renderer.texture.h, nullptr, 3);

	camera.position = {1,-0.1,1};


	return true;
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



	glm::vec3 light_dir(0, 0, -1);


	glm::mat4 projMat = camera.getProjectionMatrix() * camera.getWorldToViewMatrix();

	static int counter = 500;

	for (int i = 0; i < model.LoadedIndices.size() / 3; i++)
	{
		//i = 941;

		std::vector<int> face = {(int)model.LoadedIndices[i * 3], (int)model.LoadedIndices[i * 3 + 1], (int)model.LoadedIndices[i * 3] + 2};
		glm::vec3 screen_coords[3];
		glm::vec3 world_coords[3];
		glm::vec4 projected_coords[3];
		glm::vec2 textureUVs[3];

		for (int j = 0; j < 3; j++)
		{
			auto v = model.LoadedVertices[face[j]];
			v.Position.X *= 2.f;
			v.Position.Y *= 2.f;
			v.Position.Z *= 2.f;
			v.Position.Z -= 1.5;

			screen_coords[j] = glm::vec3((v.Position.X + 1.f) * w / 2.f, (v.Position.Y + 1.f) * h / 2.f, v.Position.Z);
			screen_coords[j].x = floor(screen_coords[j].x);
			screen_coords[j].y = floor(screen_coords[j].y);
			world_coords[j] = {v.Position.X, v.Position.Y, v.Position.Z};

			textureUVs[j].x = v.TextureCoordinate.X;
			textureUVs[j].y = v.TextureCoordinate.Y;

			projected_coords[j] = glm::vec4(world_coords[j], 1);

			projected_coords[j] = projMat * projected_coords[j];

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
		float intensity = glm::clamp(glm::dot(n, light_dir), 0.f, 1.f);
		//if(intensity > 0)
		{

			data.renderer.clipAndRenderTriangleInClipSpace(projected_coords[0], projected_coords[1], projected_coords[2],
				textureUVs[0], textureUVs[1], textureUVs[2],
				glm::vec3(intensity));

			//mem->renderer.renderLineClipSpace(projected_coords[0], projected_coords[1], glm::vec3(1.f));
			//mem->renderer.renderLineClipSpace(projected_coords[1], projected_coords[2], glm::vec3(1.f));
			//mem->renderer.renderLineClipSpace(projected_coords[2], projected_coords[0], glm::vec3(1.f));

		}

		//if(i > 1000)
		//break;
		//for(int i=0;i<3;i++)
		//line(Vec2i(screen_coords[i].x, screen_coords[i].y), Vec2i(screen_coords[(i+1)%3].x, screen_coords[(i + 1) % 3].y), {255,255,255});
	}





	return true;
}

void closeGameLogic()
{
}





