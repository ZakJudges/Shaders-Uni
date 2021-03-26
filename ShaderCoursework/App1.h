// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "../DXFramework/DXF.h"

#include "SceneShader.h"
#include "TessPlane.h"
#include "WaterShader.h"
#include "BlurShader.h"



class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in);

	bool frame();

protected:
	bool render();
	void underWaterEffect();
	void blurEffect();
	void renderScene();
	void gui();

private:
	//	Scene objects.
	TessPlane* tessMesh;
	Light* light;
	OrthoMesh* orthoMesh;

	//	Shaders.
	SceneShader* sceneShader;
	WaterShader* underWaterShader;
	BlurShader* blurShader;

	//	RenderTextures.
	RenderTexture* sceneTex;
	RenderTexture* underWaterTex;
	
	//	Shader parameters.
	float waveTime;
	float deconstructionTime;
	float _screenWidth;
	float _screenHeight;

	//	Imgui.
	bool isWireframe;
	bool underWaterEffects;
	float terrainScale;
	float heightScale;
	float specularPower;
	float tessellationFactor;
	bool reconstructing;

};

#endif