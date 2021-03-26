#pragma once

#include "../DXFramework/BaseShader.h"
#include "../DXFramework/Light.h"

using namespace std;
using namespace DirectX;

//	The main shader class. Used for tessellation, vertex manipulation, geometry manipulation, directional diffuse and specular lighting (blinn-phong).
class SceneShader : public BaseShader
{
private:
	struct TessellationBufferType
	{
		float tessFactor;
		XMFLOAT3 padding;
	};

	struct LightBufferType
	{
		XMFLOAT4 diffuse;
		XMFLOAT4 ambient;
		XMFLOAT4 specular;
		XMFLOAT3 direction;
		float specularPower;
	};

	struct GeometryBufferType
	{
		XMFLOAT3 cameraPos;
		float timeScale;
	};

	struct HeightBufferType
	{
		float heightScale;
		XMFLOAT3 padding;
	};

public:

	SceneShader(ID3D11Device* device, HWND hwnd);
	~SceneShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view,
		const XMMATRIX &projection, ID3D11ShaderResourceView* heightmap, ID3D11ShaderResourceView* normalmap, ID3D11ShaderResourceView* texture,
			XMFLOAT3 camPos, Light* light, float time, float specularPower, float tessellationFactor, float heightScale);

	void render(ID3D11DeviceContext* deviceContext, int vertexCount);

private:
	void initShader(WCHAR* vsFilename, WCHAR* psFilename);
	void initShader(WCHAR* vsFilename, WCHAR* hsFilename, WCHAR* dsFilename, WCHAR* gsFilename, WCHAR* psFilename);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;	
	ID3D11Buffer* tessellationBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* geometryBuffer;
	ID3D11Buffer* heightBuffer;


};
