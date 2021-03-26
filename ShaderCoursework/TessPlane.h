#pragma once

#include "../DXFramework/BaseMesh.h"

class TessPlane : public BaseMesh
{

public:
	TessPlane(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 10);
	~TessPlane();
	void sendData(ID3D11DeviceContext* deviceContext);

protected:
	void initBuffers(ID3D11Device* device);
	int resolution;
};

