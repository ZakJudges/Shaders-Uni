#include "App1.h"

App1::App1()
{
	tessMesh = nullptr;
	light = nullptr;
	orthoMesh = nullptr;
	sceneShader = nullptr;
	underWaterShader = nullptr;
	blurShader = nullptr;
	sceneTex = nullptr;
	underWaterTex = nullptr;
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in)
{
	// Call super/parent init function
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in);

	//	Load the terrain heightmap, normal map and texture.
	textureMgr->loadTexture("heightmap", L"../res/ice_heightmap.png");
	textureMgr->loadTexture("normalmap", L"../res/ice_normal_map.jpg");
	textureMgr->loadTexture("texture", L"../res/ice_texture.jpg");

	// Create terrain object.
	tessMesh = new TessPlane(renderer->getDevice(), renderer->getDeviceContext());
	
	//	Create the main shader - tessellation, vertex manipulation, basic lighting, geometry manipulation.
	sceneShader = new SceneShader(renderer->getDevice(), hwnd);
	//	Create the post processing shaders.
	blurShader = new BlurShader(renderer->getDevice(), hwnd);
	underWaterShader = new WaterShader(renderer->getDevice(), hwnd);

	//	The render to textures used for post processing.
	sceneTex = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	underWaterTex = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	
	//	The mesh used to render the post processed scene.
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight, 0, 0);

	//	Create light (directional)
	light = new Light;
	light->setDirection(-1.0f, -1.0f, 0.0f);
	light->setAmbientColour(0.0f, 0.0f, 0.35f, 1.0f);
	light->setDiffuseColour(0.8f, 0.8f, 0.8f, 1.0f);
	light->setSpecularColour(0.9f, 0.9f, 0.9f, 1.0f);

	//	Shader parameter initialisation.
	deconstructionTime = 0.0f;
	waveTime = 0.0f;
	_screenWidth = screenWidth;
	_screenHeight = screenHeight;

	//	Imgui.
	heightScale = 6.0f;
	specularPower = 200.0f;
	terrainScale = 5.0f;
	tessellationFactor = 16.0f;
	reconstructing = true;
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();
	if (tessMesh)
	{
		delete tessMesh;
		tessMesh = 0;
	}
	if (sceneShader)
	{
		delete sceneShader;
		sceneShader = 0;
	}
	if (light)
	{
		delete light;
		light = 0;
	}
	if (sceneShader)
	{
		delete sceneShader;
		sceneShader = 0;
	}
	if (underWaterShader)
	{
		delete underWaterShader;
		underWaterShader = 0;
	}
	if (blurShader)
	{
		delete blurShader;
		blurShader = 0;
	}
	if (sceneTex)
	{
		delete sceneTex;
		sceneTex = 0;
	}
	if (underWaterTex)
	{
		delete underWaterTex;
		underWaterTex = 0;
	}
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}

	// Render the graphics.
	result = render();

	//	Time used in post processing shader.
	waveTime += timer->getTime();

	if (!reconstructing)
	{
		//	Time used in the geometry shader for the scene.
		deconstructionTime += timer->getTime();
	}
	else
	{
		//	Decrement the time to reverse the deconstruction.
		//	Time set to 0 so that the primitives will not move.
		deconstructionTime -= timer->getTime();
		if (deconstructionTime <= 0.0f)
		{
			deconstructionTime = 0.0f;
		}
	}
	

	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	//	Set the state of wireframe mode.
	renderer->setWireframeMode(isWireframe);

	//	Render the scene as normal.
	//	The scene will be rendered to the back buffer if post processing is disabled.
	//	Otherwise, the scene will be rendered to a render texture.
	renderScene();

	if (underWaterEffects)
	{
		//	Render to texture for WaterShader.
		blurEffect();
		//	Render to the back buffer using under water effect texture.
		underWaterEffect();
	}

	// Render GUI
	gui();

	//// Present the rendered scene to the screen.
	renderer->endScene();

	return true;
}

void App1::renderScene()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	if (!underWaterEffects)
	{
		//	Set the render target to be the back buffer.
		renderer->setBackBufferRenderTarget();
	}
	else
	{
		//	Set the render target to be the render to texture.
		sceneTex->setRenderTarget(renderer->getDeviceContext());
		sceneTex->clearRenderTarget(renderer->getDeviceContext(), 0.02f, 0.02f, 0.1f, 1.0f);
	}

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection matrices.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	//	Scale the scene based on user input.
	worldMatrix = XMMatrixScaling(terrainScale, 1.0f, terrainScale);

	// Send geometry data for the terrain.
	tessMesh->sendData(renderer->getDeviceContext());

	// Set shader parameters.
	sceneShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture("heightmap"), textureMgr->getTexture("normalmap"), textureMgr->getTexture("texture"),
			camera->getPosition(), light, deconstructionTime, specularPower, tessellationFactor, heightScale);

	// Render object.
	sceneShader->render(renderer->getDeviceContext(), tessMesh->getIndexCount());
}

void App1::blurEffect()
{
	XMMATRIX worldMatrix, orthoViewMatrix, orthoMatrix;

	underWaterTex->setRenderTarget(renderer->getDeviceContext());

	underWaterTex->clearRenderTarget(renderer->getDeviceContext(), 0.9f, 0.9f, 0.9f, 1.0f);

	//	Get the world matrix.
	worldMatrix = renderer->getWorldMatrix();

	//	Turn off the z-buffer to begin 2D rendering of the orthomesh.
	renderer->setZBuffer(false);

	//	Ortho matrices for 2D rendering.
	orthoMatrix = renderer->getOrthoMatrix();
	orthoViewMatrix = camera->getOrthoViewMatrix();

	//	Send orthomesh data for texture coordinates etc.
	orthoMesh->sendData(renderer->getDeviceContext());
	blurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, sceneTex->getShaderResourceView(),
		_screenWidth, _screenHeight);
	blurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	//	Turn on the z-buffer again as 2D rendering has ended.
	renderer->setZBuffer(true);
}


void App1::underWaterEffect()
{
	XMMATRIX worldMatrix, orthoViewMatrix, orthoMatrix;

	//	Reset the render target to the back buffer.
	renderer->setBackBufferRenderTarget();

	// Clear the scene.
	renderer->beginScene(0.2f, 0.2f, 0.25f, 1.0f);

	//	Get the world matrix.
	worldMatrix = renderer->getWorldMatrix();

	//	Turn off the z-buffer to begin 2D rendering of the orthomesh.
	renderer->setZBuffer(false);

	//	Ortho matrices for 2D rendering.
	orthoMatrix = renderer->getOrthoMatrix();
	orthoViewMatrix = camera->getOrthoViewMatrix();

	//	Send orthomesh data fot texture coordinates etc.
	underWaterShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, underWaterTex->getShaderResourceView(), waveTime,
		_screenWidth, _screenHeight);
	underWaterShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	//	Turn on the z-buffer again as 2D rendering has ended.
	renderer->setZBuffer(true);
}




void App1::gui()
{
	//	Force turn off on Geometry, Hull and Domain shaders.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	//	Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe", &isWireframe);
	ImGui::SliderFloat("Tessellation", &tessellationFactor, 1.0f, 64.0f, "%g");
	ImGui::SliderFloat("Height Scale", &heightScale, 1.0f, 20.0f, "%g");
	ImGui::SliderFloat("Terrain Scale", &terrainScale, 1.0f, 30.0f, "%f");
	ImGui::SliderFloat("Specular Power", &specularPower, 1.0f, 250.0f, "%g");
	ImGui::Checkbox("Under Water Effects", &underWaterEffects);
	if (!reconstructing)
	{
		ImGui::Checkbox("Reconstruct", &reconstructing);
	}
	else
	{

		ImGui::Checkbox("Deconstruct", &reconstructing);
	}

	//	Render UI
	ImGui::Render();
}

