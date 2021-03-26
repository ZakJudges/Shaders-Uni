#pragma once


#include "../DXFramework/BaseShader.h"

using namespace std;
using namespace DirectX;


class WaterShader : public BaseShader
{
	struct TimeBufferType
	{
		float time;
		XMFLOAT3 padding;
	};

public:

	WaterShader(ID3D11Device* device, HWND hwnd);
	~WaterShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture
		, float time, float screenWidth, float screenHeight);
	void render(ID3D11DeviceContext* deviceContext, int vertexCount);

private:
	void initShader(WCHAR*, WCHAR*);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* timeBuffer;
};

