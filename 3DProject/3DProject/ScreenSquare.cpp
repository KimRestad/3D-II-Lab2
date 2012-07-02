#include "ScreenSquare.h"

ScreenSquare::ScreenSquare()
{
}

void ScreenSquare::Initialize(ID3D10Device* device, ID3D10ShaderResourceView* drawTexture, D3DXVECTOR2 position, float width, float height)
{
	mDevice = device;
	mDrawTexture = drawTexture;

	UpdateViewportMatrix((int)width, (int)height);

	CreateBuffer(position, width, height);
	CreateEffect();
	CreateVertexLayout();

	ID3D10ShaderResourceView *pSRView = NULL;
	D3DX10CreateShaderResourceViewFromFile(mDevice, "StoneFloor.png", NULL, NULL, &pSRView, NULL );
	mEffect->GetVariableByName("textureBG")->AsShaderResource()->SetResource(pSRView);
}

void ScreenSquare::CreateBuffer(D3DXVECTOR2 position, float width, float height)
{
	const int numVertices = 4;
	SSVertex vertices[numVertices];

	float leftX = position.x;
	float rightX = position.x + width;
	float topY = position.y;
	float bottomY = position.y + height;

	float numTexturesX = 60;
	float numTexturesY = 60;

	//vertices[0].position = TransformToViewport(D3DXVECTOR2(leftX, topY));		// Top left
	vertices[0].position = D3DXVECTOR2(0.5, 1);
	vertices[0].uv = D3DXVECTOR2(0, 0);

	//vertices[1].position = TransformToViewport(D3DXVECTOR2(rightX, topY));		// Top right
	vertices[1].position = D3DXVECTOR2(1, 1);
	vertices[1].uv = D3DXVECTOR2(1, 0);

	//vertices[2].position = TransformToViewport(D3DXVECTOR2(leftX, bottomY));	// Bottom left
	vertices[2].position = D3DXVECTOR2(0.5, 0.5);
	vertices[2].uv = D3DXVECTOR2(0, 1);

	//vertices[3].position = TransformToViewport(D3DXVECTOR2(rightX, bottomY));	// Bottom right
	vertices[3].position = D3DXVECTOR2(1, 0.5);
	vertices[3].uv = D3DXVECTOR2(1, 1);

	mBuffer = new Buffer();
	BufferInformation bufferDesc;

	bufferDesc.type = VertexBuffer;
	bufferDesc.usage = Buffer_Default;
	bufferDesc.numberOfElements = numVertices;
	bufferDesc.firstElementPointer = vertices;
	bufferDesc.elementSize = sizeof(SSVertex);

	mBuffer->Initialize(mDevice, bufferDesc);

}

void ScreenSquare::CreateEffect()
{
	HRESULT result = S_OK;								// Variable that stores the result of the functions
	UINT shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;	// Shader flags
	ID3D10Blob* errors = NULL;							// Variable to store error messages from functions
	ID3D10Blob* effect = NULL;							// Variable to store compiled (but not created) effect
	
	// Compile shader, if failed - show error message and return
	result = D3DX10CompileFromFileA("ScreenSquare.fx",		// Path and name of the effect file to compile
								   0,				// Shader macros: none needed
								   0,				// Include interface: not needed - no #include in shader
								   "",				// Shader start function, not used when compiling from file
								   "fx_4_0",		// String specifying shader model (or shader profile)
								   shaderFlags,		// Shader compile flags - how to compile the shader	
								   0,				// Effect compile flags - not used when compiling a shader
								   0,				// Thread pump interface: not needed - return only when finished
								   &effect,			// Out: Where to put the compiled effect information (pointer)
								   &errors,			// Out: Where to put the errors, if there are any (pointer)
								   NULL);			// Out: Result not needed, result is gotten from the return value
	
	if(FAILED(result))								// If failed, show error message and return
	{
		if(errors)
		{
			MessageBox(0, (char*)errors->GetBufferPointer(), "ERROR", 0);
			SafeRelease(errors);
		}
	}

	result = D3DX10CreateEffectFromMemory(
						effect->GetBufferPointer(),	// Pointer to the effect in memory, gotten from the compiled effect
						effect->GetBufferSize(),	// The effect's size in memory, gotten from the compiled effect
						"ScreenSquare.fx",			// Name of the effect file
						NULL,						// Shader macros: none needed
						NULL,						// Include interface: not needed 
						"fx_4_0",					// String specifying shader model (or shader profile)
						NULL,						// Shader constants, HLSL compile options						
						NULL,						// Effect constants, Effect compile options
						mDevice,					// Pointer to the direct3D device that will use resources
						NULL,						// Pointer to effect pool for variable sharing between effects
						NULL,						// Thread pump interface: not needed - return only when finished
						&mEffect,					// Out: where to put the created effect (pointer)
						&errors,					// Out: Where to put the errors, if there are any (pointer)
						NULL);						// Out: Result not needed, result is gotten from the return value

	if(FAILED(result))								// If failed, show error message and return
	{
		MessageBox(0, "Shader creation failed!", "ScreenSquare", 0);
	}
}

void ScreenSquare::CreateVertexLayout()
{
	// Create an array describing each of the elements of the vertex that are inputs to the vertex shader.
	D3D10_INPUT_ELEMENT_DESC vertexDesc[] = 
	{
		{ "POSITION",					// Semantic name, must be same as the vertex shader input semantic name
		  0,							// Semantic index, if one semantic name exists for more than one element
		  DXGI_FORMAT_R32G32_FLOAT,		// Format of the element, R32G32_FLOAT is a 32-bit 2D float vector
		  0,							// Input slot, of the 0-15 slots, through wich to send vertex data
		  0,							// AlignedByteOffset, bytes from start of the vertex to this component
		  D3D10_INPUT_PER_VERTEX_DATA,	// Input data class for this input slot
		  0 },							// 0 when slot input data class is D3D10_INPUT_PER_VERTEX_DATA
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(D3DXVECTOR2), D3D10_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D10_PASS_DESC passDesc;
	HRESULT result;

	// Get the effect technique from the effect, and save the descritption of the first pass
	mTechnique = mEffect->GetTechniqueByName("DrawTechnique");
	mTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	// Create the input layout and save it, if failed - show an error message
	result = mDevice->CreateInputLayout(
				vertexDesc,						// Description of input structure - array of element descriptions
				2,								// Number of elements in the input structure description
				passDesc.pIAInputSignature,		// Get pointer to the compiled shader
				passDesc.IAInputSignatureSize,	// The size of the compiled shader
				&mVertexLayout);				// Out: where to put the created input layout 

	if(FAILED(result))							// If layer creation fails, show an error message and return
	{
		MessageBox(0, "Input Layout creation failed!", "ScreenSquare", 0);
	}

	// Bind the input layout to the 3D device
	mDevice->IASetInputLayout(mVertexLayout);
}

D3DXVECTOR2 ScreenSquare::TransformToViewport(const D3DXVECTOR2& vector)
{
	D3DXVECTOR4 transformedVector;
	D3DXVec4Transform(&transformedVector, &D3DXVECTOR4(vector.x, vector.y, 1, 0), &mViewportMatrix);

	return D3DXVECTOR2(transformedVector.x, transformedVector.y);
}

void ScreenSquare::UpdateViewportMatrix(int newWidth, int newHeight)
{
	mViewportMatrix.m[0][0] = 2.0f / newWidth;
	mViewportMatrix.m[0][1] = 0.0f;
	mViewportMatrix.m[0][2] = 0.0f;
	mViewportMatrix.m[0][3] = 0.0f;

	mViewportMatrix.m[1][0] = 0.0f;
	mViewportMatrix.m[1][1] = -2.0f / newHeight;
	mViewportMatrix.m[1][2] = 0.0f;
	mViewportMatrix.m[1][3] = 0.0f;

	mViewportMatrix.m[2][0] = -1.0f;
	mViewportMatrix.m[2][1] = 1.0f;
	mViewportMatrix.m[2][2] = 1.0f;
	mViewportMatrix.m[2][3] = 0.0f;

	mViewportMatrix.m[3][0] = 0.0f;
	mViewportMatrix.m[3][1] = 0.0f;
	mViewportMatrix.m[3][2] = 0.0f;
	mViewportMatrix.m[3][3] = 1.0f;
}

void ScreenSquare::Draw()
{
	SetTexture();
	mBuffer->MakeActive();

	mDevice->IASetInputLayout(mVertexLayout);
	mDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	D3D10_TECHNIQUE_DESC techDesc;
	mTechnique->GetDesc(&techDesc);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		mTechnique->GetPassByIndex(p)->Apply(0);
		mDevice->Draw(mBuffer->GetSize(), 0);
	}

	ClearTexture();
}

void ScreenSquare::SetTexture()
{
	mEffect->GetVariableByName("textureBG")->AsShaderResource()->SetResource(mDrawTexture);
}

void ScreenSquare::ClearTexture()
{
	ID3D10ShaderResourceView* nullTexture = NULL;

	mEffect->GetVariableByName("textureBG")->AsShaderResource()->SetResource(nullTexture);
}