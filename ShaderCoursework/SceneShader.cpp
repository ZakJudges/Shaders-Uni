// SceneShader.cpp

#include "SceneShader.h"

SceneShader::SceneShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"scene_vs.cso", L"scene_hs.cso", L"scene_ds.cso", L"scene_gs.cso", L"scene_ps.cso");
}


SceneShader::~SceneShader()
{
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}
	if (geometryBuffer)
	{
		geometryBuffer->Release();
		geometryBuffer = 0;
	}

	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}

	if (heightBuffer)
	{
		heightBuffer->Release();
		heightBuffer = 0;
	}
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void SceneShader::initShader(WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC tessellationBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC geometryBufferDesc;
	D3D11_BUFFER_DESC heightBufferDesc;

	// Load vertex and pixel shaders.
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the matrix constant buffer that is in the domain shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Setup the description of the tessellation constant buffer that is in the hull shader.
	tessellationBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tessellationBufferDesc.ByteWidth = sizeof(TessellationBufferType);
	tessellationBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tessellationBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tessellationBufferDesc.MiscFlags = 0;
	tessellationBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&tessellationBufferDesc, NULL, &tessellationBuffer);


	// Setup the description of the light constant buffer that is in the pixel shader.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	// Setup the description of the Geometry constant buffer that is in the geometry shader.
	geometryBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	geometryBufferDesc.ByteWidth = sizeof(GeometryBufferType);
	geometryBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	geometryBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	geometryBufferDesc.MiscFlags = 0;
	geometryBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&geometryBufferDesc, NULL, &geometryBuffer);

	// Setup the description of the height constant buffer that is in the domain shader.
	heightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	heightBufferDesc.ByteWidth = sizeof(HeightBufferType);
	heightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	heightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	heightBufferDesc.MiscFlags = 0;
	heightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&heightBufferDesc, NULL, &heightBuffer);


}

void SceneShader::initShader(WCHAR* vsFilename, WCHAR* hsFilename, WCHAR* dsFilename, WCHAR* gsFilename, WCHAR* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
	loadGeometryShader(gsFilename);
}


void SceneShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix,
	const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* heightmap, ID3D11ShaderResourceView* normalmap, ID3D11ShaderResourceView* texture,
	XMFLOAT3 camPos, Light* light, float time, float specularPower, float tessellationFactor, float heightScale)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	TessellationBufferType* tessPtr;
	LightBufferType* lightPtr;
	GeometryBufferType* geometryPtr;
	HeightBufferType* heightPtr;
	unsigned int bufferNumber;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	// Unlock the constant buffer.
	deviceContext->Unmap(matrixBuffer, 0);
	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;
	deviceContext->GSSetConstantBuffers(bufferNumber, 1, &matrixBuffer);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(tessellationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	tessPtr = (TessellationBufferType*)mappedResource.pData;
	tessPtr->tessFactor = tessellationFactor;
	tessPtr->padding = XMFLOAT3(0.0f, 0.0f, 0.0f);
	// Unlock the constant buffer.
	deviceContext->Unmap(tessellationBuffer, 0);
	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;
	// Now set the constant buffer in the vertex shader with the updated values.
	deviceContext->HSSetConstantBuffers(bufferNumber, 1, &tessellationBuffer);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(geometryBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	geometryPtr = (GeometryBufferType*)mappedResource.pData;
	geometryPtr->cameraPos = camPos;
	geometryPtr->timeScale = time;
	// Unlock the constant buffer.
	deviceContext->Unmap(geometryBuffer, 0);
	bufferNumber = 1;
	// Now set the constant buffer in the vertex shader with the updated values.
	deviceContext->GSSetConstantBuffers(bufferNumber, 1, &geometryBuffer);

	// Send light data to pixel shader
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->diffuse = light->getDiffuseColour();
	lightPtr->ambient = light->getAmbientColour();
	lightPtr->specular = light->getSpecularColour();
	lightPtr->direction = light->getDirection();
	lightPtr->specularPower = specularPower;
	deviceContext->Unmap(lightBuffer, 0);
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &lightBuffer);

	// Send heightScale to domain shader
	deviceContext->Map(heightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	heightPtr = (HeightBufferType*)mappedResource.pData;
	heightPtr->heightScale = heightScale;
	heightPtr->padding = XMFLOAT3(0.0f, 0.0f, 0.0f);
	deviceContext->Unmap(heightBuffer, 0);
	bufferNumber = 0;
	deviceContext->DSSetConstantBuffers(bufferNumber, 1, &heightBuffer);


	// Set shader texture resource in the pixel and DOMAIN shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->DSSetShaderResources(0, 1, &heightmap);
	deviceContext->PSSetShaderResources(1, 1, &normalmap);			//////
}

void SceneShader::render(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the sampler states.
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->DSSetSamplers(0, 1, &sampleState);

	// Base render function.
	BaseShader::render(deviceContext, indexCount);
}



