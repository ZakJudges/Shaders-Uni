

#include "../DXFramework/BaseShader.h"

using namespace std;
using namespace DirectX;


class BlurShader : public BaseShader
{

public:
	struct ScreenSizeBufferType
	{
		float screenWidth;
		float screenHeight;
		XMFLOAT2 padding;
	};

	BlurShader(ID3D11Device* device, HWND hwnd);
	~BlurShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection,
		ID3D11ShaderResourceView* texture, float _screenWidth, float _screenHeight);
	void render(ID3D11DeviceContext* deviceContext, int vertexCount);

private:
	void initShader(WCHAR*, WCHAR*);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* screenSizeBuffer;
	ID3D11SamplerState* sampleState;
};

