#include "Object3D.h"
#include <sstream>
#include <cassert>

Object3D::MaterialInfo::MaterialInfo()
	: Ambient(D3DXVECTOR3(0.2, 0.2, 0.2))
	, Diffuse(D3DXVECTOR3(0.8, 0.8, 0.8))
	, Specular(D3DXVECTOR3(1.0, 1.0, 1.0))
	, Tf(D3DXVECTOR3(1.0, 1.0, 1.0))
	, IlluminationModel(0)
	//, Opacitiy(1.0)
	, RefractionIndex(1.0)
	, SpecularExp(8.0f)
	//, Sharpness(60.0)
	, MainTexture(NULL)
{}

Object3D::Group::Group()
	: mVertexBuffer(NULL), mFXKa(NULL), mFXKd(NULL), mFXKs(NULL), mFXSpecExp(NULL), mFXTexture(NULL)
{}

Object3D::Group::~Group() throw()
{
	SafeDelete(mVertexBuffer);
}

void Object3D::Group::AddVertices(std::vector<Vertex> vertexList)
{
	for(int i = 0; i < vertexList.size(); ++i)
		mVertices.push_back(vertexList[i]);
}

bool Object3D::Group::Finalize(ID3D10Device* device, ID3D10Effect* effect)
{
	mFXTexture = effect->GetVariableByName("gTextureBTH")->AsShaderResource();
	mFXKa = effect->GetVariableByName("gKa")->AsVector();
	mFXKd = effect->GetVariableByName("gKd")->AsVector();
	mFXKs = effect->GetVariableByName("gKs")->AsVector();
	mFXSpecExp = effect->GetVariableByName("gSExp")->AsScalar();

	if(mVertices.size() <= 0)
		return false;

	mVertexBuffer = new Buffer();
	
	BufferInformation vbDesc;
	vbDesc.type					= VertexBuffer;
	vbDesc.usage				= Buffer_Default;
	vbDesc.elementSize			= sizeof(Vertex);
	vbDesc.numberOfElements		= mVertices.size();
	vbDesc.firstElementPointer	= &mVertices[0];

	return mVertexBuffer->Initialize(device, vbDesc) == S_OK;
}

void Object3D::Group::Draw(ID3D10Device* device)
{
	if(mVertexBuffer == NULL)
		return;

	mFXTexture->SetResource(Material->MainTexture);
	mFXKa->SetFloatVector((float*)&Material->Ambient);
	mFXKd->SetFloatVector((float*)&Material->Diffuse);
	mFXKs->SetFloatVector((float*)&Material->Specular);
	mFXSpecExp->SetFloat(Material->SpecularExp);
	//effect->GetVariableByName("Tf")->AsVector()->SetFloatVector(Material->Tf);
	//effect->GetVariableByName("illum")->AsScalar()->SetInt(Material->IlluminationModel);
	//effect->GetVariableByName("refrac")->AsScalar()->SetFloat(Material->RefractionIndex);

	mVertexBuffer->MakeActive();

	device->Draw(mVertexBuffer->GetSize(), 0);
}

Object3D::Object3D(ID3D10Device* device, std::string filename, D3DXVECTOR3 position, D3DXVECTOR3 lightPos)
	: mDevice(device), mEffect(NULL), mTechnique(NULL), mVertexLayout(NULL), mFont(NULL), 
	  mPosition(position), mRotation(0.0f), mVelocity(D3DXVECTOR3(1.2, 0.5, 0.8)), mLightPosition(lightPos),
	  mFXEyePos(NULL), mFXLightPos(NULL), mFXWorld(NULL), mFXWorldViewProj(NULL)
{
	if(!Load(filename))
		return;

	CreateEffect();
	CreateVertexLayout();

	mMatrixWorld = new D3DXMATRIX();
	D3DXMatrixIdentity(mMatrixWorld);

	D3DXVec3Normalize(&mVelocity, &mVelocity);
	mVelocity *= 50;

	UpdateWorldMatrix();

	mFXEyePos = mEffect->GetVariableByName("gEyePos")->AsVector();
	mFXLightPos = mEffect->GetVariableByName("gLightPosition")->AsVector();
	mFXWorld = mEffect->GetVariableByName("gWorld")->AsMatrix();
	mFXWorldViewProj = mEffect->GetVariableByName("gWVP")->AsMatrix();
	
	for(std::map<std::string, Group>::iterator it = mGroups.begin(); it != mGroups.end(); ++it)
		it->second.Finalize(mDevice, mEffect);

	mFont = new GameFont(mDevice, "Times New Roman", 18);
}

Object3D::~Object3D()
{
	SafeRelease(mEffect);
	SafeRelease(mVertexLayout);

	SafeDelete(mFont);
	SafeDelete(mMatrixWorld);
}

bool Object3D::Load(std::string filename)
{
	std::ifstream file;
	std::vector<Vertex> vertices;
	std::vector<D3DXVECTOR3> outPositions;
	std::vector<D3DXVECTOR2> outUVCoords;
	std::vector<D3DXVECTOR3> outNormals;

	Group currGroup;
	std::string currGroupName = "";

	file.open(filename.c_str(), std::ios_base::binary);
	
	if(!file.is_open())
		return false;

	while(!file.eof())
	{
		// Read first line of file.
		std::string line;
		std::getline(file, line);

		// Copy line to a stringstream and copy first word into string key
		std::stringstream streamLine;
		streamLine.str(line);
		std::string key;
		streamLine >> key;

		if(key == "mtllib")
		{
			std::string matFileName;
			streamLine >> matFileName;
			LoadMaterials(matFileName);
		}
		else if(key == "v")
		{
			D3DXVECTOR3 currPos;
			streamLine >> currPos.x;
			streamLine >> currPos.y;
			streamLine >> currPos.z;
			outPositions.push_back(currPos);
		}
		else if(key == "vt")
		{
			D3DXVECTOR2 currUV;
			streamLine >> currUV.x;
			streamLine >> currUV.y;
			currUV.y = 1 - currUV.y;
			outUVCoords.push_back(currUV);
		}
		else if(key == "vn")
		{
			D3DXVECTOR3 currNormal;
			streamLine >> currNormal.x;
			streamLine >> currNormal.y;
			streamLine >> currNormal.z;
			outNormals.push_back(currNormal);
		}
		else if(key == "g")
		{
			std::string groupName;
			streamLine >> groupName;

			std::streampos pos = file.tellg();
			std::getline(file, line);
			streamLine.str(line);
			streamLine >> key;

			if(key == "usemtl")
			{
				streamLine >> key;
				if(currGroupName != "") // It is not the first group
				{
					mGroups[currGroupName].Material = currGroup.Material;
					mGroups[currGroupName].AddVertices(vertices);
					vertices.clear();
					currGroup = Group();
				}

				assert(mMaterials.find(key) != mMaterials.end()); // Make sure the material exists
				currGroup.Material = &mMaterials[key];
				currGroupName = groupName;
			}
			else
				file.seekg(pos);
		}
		else if(key == "f")
		{
			int pos[3]; 
			int uv[3];
			int norm[3];

			for(int i = 0; i < 3; ++i)
			{
				streamLine >> pos[i];
				streamLine.ignore();
				streamLine >> uv[i];
				streamLine.ignore();
				streamLine >> norm[i];

				Vertex currVertex;
				currVertex.Position = outPositions[pos[i] - 1];
				currVertex.UV = outUVCoords[uv[i] - 1];
				currVertex.Normal = outNormals[norm[i] - 1];
				vertices.push_back(currVertex);
			}
		}
	}

	mGroups[currGroupName].Material = currGroup.Material;
	mGroups[currGroupName].AddVertices(vertices);

	return true;
}

bool Object3D::LoadMaterials(std::string filename)
{
	MaterialInfo currMaterial;
	std::string currMaterialName = "";
	std::ifstream file;
	file.open(filename.c_str(), std::ios_base::in);
	
	if(!file.is_open())
		return false;

	while(!file.eof())
	{
		// Read first line of file.
		std::string line;
		std::getline(file, line);

		// Copy line to a stringstream and copy first word into string key
		std::stringstream streamLine;
		std::string key;

		streamLine.str(line);
		streamLine >> key;

		if(key == "newmtl")
		{
			if(currMaterialName != "") // It is not the first material read
			{
				// Save previous material, making sure the material does not already exist in Materials
				assert(mMaterials.find(currMaterialName) == mMaterials.end());
				mMaterials[currMaterialName] = currMaterial;
			}

			// Set new material name and clear current material
			streamLine >> currMaterialName;
			currMaterial = MaterialInfo();
		}
		else if(key == "Ka") // Ambient color
		{
			streamLine >> currMaterial.Ambient.x;
			streamLine >> currMaterial.Ambient.y;
			streamLine >> currMaterial.Ambient.z;
		}
		else if(key == "Kd") // Diffuse color
		{
			streamLine >> currMaterial.Diffuse.x;
			streamLine >> currMaterial.Diffuse.y;
			streamLine >> currMaterial.Diffuse.z;
		}
		else if(key == "Ks") // Specular color
		{
			streamLine >> currMaterial.Specular.x;
			streamLine >> currMaterial.Specular.y;
			streamLine >> currMaterial.Specular.z;
		}
		else if(key == "Tf") // Transmission filter
		{
			streamLine >> currMaterial.Tf.x;
			streamLine >> currMaterial.Tf.y;
			streamLine >> currMaterial.Tf.z;
		}
		else if(key == "illum") // Illumination model
		{
			streamLine >> currMaterial.IlluminationModel;
		}
		//else if(key == "d" || key == "Tr") // Opacity
		//{
		//	streamLine >> currMaterial.Opacitiy;
		//}
		else if(key == "Ni") // Optical density
		{
			streamLine >> currMaterial.RefractionIndex;
		}
		else if(key == "Ns") // Specular exponent
		{
			streamLine >> currMaterial.SpecularExp;
		}
		//else if(key == "sharpness") // Reflection sharpness
		//{
		//	streamLine >> currMaterial.Sharpness;
		//}
		else if(key == "map_Ka" || key == "map_Kd" || key == "map_Ks")				// Only use one texture at this point
		{
			// Get the next argument on the line
			std::string textureFilename;
			streamLine >> textureFilename;

			// Make sure there is a period in the name indicating a file name. The other
			// arguments are not interesting at this point.
			while (textureFilename.find('.') == std::string::npos && !streamLine.eof())
				streamLine >> textureFilename;

			// Only try to load the texture if a filename was read
			if(textureFilename.find('.') != std::string::npos)
				D3DX10CreateShaderResourceViewFromFile(mDevice, "bthcolor.dds", NULL, NULL, &currMaterial.MainTexture, NULL);
		}
	}

	if(currMaterialName != "") // It is not the first material read
	{
		// Save previous material, making sure the material does not already exist in Materials
		assert(mMaterials.find(currMaterialName) == mMaterials.end());
		mMaterials[currMaterialName] = currMaterial;
	}
	
	return true;
}

// Compile and create the shader/effect
HRESULT Object3D::CreateEffect()
{
	HRESULT result = S_OK;								// Variable that stores the result of the functions
	UINT shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;	// Shader flags
	ID3D10Blob* errors = NULL;							// Variable to store error messages from functions
	ID3D10Blob* effect = NULL;							// Variable to store compiled (but not created) effect
	
	// Compile shader, if failed - show error message and return
	result = D3DX10CompileFromFileA("Effect.fx",	// Path and name of the effect file to compile
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
			MessageBox(0, (char*)errors->GetBufferPointer(), "OBJECT3D ERROR", 0);
			SafeRelease(errors);
		}

		return result;
	}

	result = D3DX10CreateEffectFromMemory(
						effect->GetBufferPointer(),	// Pointer to the effect in memory, gotten from the compiled effect
						effect->GetBufferSize(),	// The effect's size in memory, gotten from the compiled effect
						"Effect.fx",				// Name of the effect file
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
		MessageBox(0, "Shader creation failed!", "OBJECT3D ERROR", 0);
		return result;
	}

	return result;
}

// Build vertex layout
HRESULT Object3D::CreateVertexLayout()
{
	// Create an array describing each of the elements of the vertex that are inputs to the vertex shader.
	D3D10_INPUT_ELEMENT_DESC vertexDesc[] = 
	{
		{ "POSITION",					// Semantic name, must be same as the vertex shader input semantic name
		  0,							// Semantic index, if one semantic name exists for more than one element
		  DXGI_FORMAT_R32G32B32_FLOAT,	// Format of the element, R32G32B32_FLOAT is a 32-bit 3D float vector
		  0,							// Input slot, of the 0-15 slots, through wich to send vertex data
		  0,							// AlignedByteOffset, bytes from start of the vertex to this component
		  D3D10_INPUT_PER_VERTEX_DATA,	// Input data class for this input slot
		  0 },							// 0 when slot input data class is D3D10_INPUT_PER_VERTEX_DATA
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 3, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,  sizeof(float) * 6, D3D10_INPUT_PER_VERTEX_DATA, 0 }
	};

		D3D10_PASS_DESC passDesc;
		HRESULT result;

		// Get the effect technique from the effect, and save the descritption of the first pass
		mTechnique = mEffect->GetTechniqueByName("DrawTechnique");
		mTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

		// Create the input layout and save it, if failed - show an error message
		result = mDevice->CreateInputLayout(
					vertexDesc,						// Description of input structure - array of element descriptions
					3,								// Number of elements in the input structure description
					passDesc.pIAInputSignature,		// Get pointer to the compiled shader
					passDesc.IAInputSignatureSize,	// The size of the compiled shader
					&mVertexLayout);				// Out: where to put the created input layout 

		if(FAILED(result))							// If layer creation fails, show an error message and return
		{
			MessageBox(0, "Input Layout creation failed!", "OBJECT3D ERROR", 0);
			return result;
		}

		// Bind the input layout to the 3D device
		mDevice->IASetInputLayout(mVertexLayout);

		return result;
}

void Object3D::Update(GameTime gameTime)
{
	mRotation += gameTime.GetTimeSinceLastTick().Seconds;
	mPosition += mVelocity * gameTime.GetTimeSinceLastTick().Seconds;

	if(mPosition.x < -256 || mPosition.x > 256)
		mVelocity.x = -mVelocity.x;
	/*if(mPosition.y < 0 || mPosition.y > 50)
		mVelocity.y = -mVelocity.y;*/
	if(mPosition.y < 0)
		mVelocity.y = abs(mVelocity.y);
	if(mPosition.y > 50)
		mVelocity.y = -abs(mVelocity.y);
	if(mPosition.z < -256 || mPosition.z > 256)
		mVelocity.z = -mVelocity.z;

	UpdateWorldMatrix();

	if(GetAsyncKeyState(VK_F1))
		mEffect->GetVariableByName("gDrawLight")->AsScalar()->SetBool(false);
	else if(GetAsyncKeyState(VK_F2))
		mEffect->GetVariableByName("gDrawLight")->AsScalar()->SetBool(true);
}

void Object3D::Draw(D3DXMATRIX* vpMatrix, D3DXVECTOR3 eyePos)
{
	mDevice->IASetInputLayout(mVertexLayout);
	mDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3DXMATRIX wvp = (*mMatrixWorld) * (*vpMatrix);

	mFXEyePos->SetFloatVector((float*)&eyePos);
	mFXLightPos->SetFloatVector((float*)&mLightPosition);
	mFXWorld->SetMatrix((float*)mMatrixWorld);
	mFXWorldViewProj->SetMatrix((float*)wvp);

	D3D10_TECHNIQUE_DESC techDesc;
	mTechnique->GetDesc(&techDesc);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		mTechnique->GetPassByIndex(p)->Apply(0);
		
		for(std::map<std::string, Group>::iterator it = mGroups.begin(); it != mGroups.end(); ++it)
			it->second.Draw(mDevice);
	}
}

void Object3D::UpdateWorldMatrix()
{
	// Update rotation in matrix
	float cosA = std::cos(mRotation);
	float sinA = std::sin(mRotation);

	mMatrixWorld->m[0][0] = cosA;
	mMatrixWorld->m[0][2] = -sinA;
	mMatrixWorld->m[2][0] = sinA;
	mMatrixWorld->m[2][2] = cosA;

	// Update position in matrix
	mMatrixWorld->m[3][0] = mPosition.x;
	mMatrixWorld->m[3][1] = mPosition.y;
	mMatrixWorld->m[3][2] = mPosition.z;
}