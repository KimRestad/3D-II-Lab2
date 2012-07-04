#include "Scene.h"
#include <sstream>

Scene::Scene(ID3D10Device* device, const int& screenWidth)
	: mDevice(device), mDepthMapIndex(0), mObject(NULL), mLightDirection(D3DXVECTOR3(-300.0f, 50.0f, -300.0f))
{
	D3DXVECTOR3& lightPosition = mLightDirection;
	mObject = new Object3D(mDevice, "bth.obj", D3DXVECTOR3(-100.0, 0.0, -100.0), lightPosition);

	ZeroMemory(&mLightViewMatrix, sizeof(D3DXMATRIX));
	ZeroMemory(&mLightProjMatrix, sizeof(D3DXMATRIX));
	CreateLightMatrices();

	// Shadow map things
	mDepthMap.push_back(CreateDepthTexture(256, 256));
	mDepthMap.push_back(CreateDepthTexture(512, 512));
	mDepthMap.push_back(CreateDepthTexture(1024, 1024));
	mDepthMap.push_back(CreateDepthTexture(2048, 2048));

	ZeroMemory(&mViewport, sizeof(D3D10_VIEWPORT));
	mViewport.TopLeftX = 0;
	mViewport.TopLeftY = 0;
	mViewport.Width = mDepthMap[mDepthMapIndex]->C_WIDTH;
	mViewport.Height = mDepthMap[mDepthMapIndex]->C_HEIGHT;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;

	mFloor.Initialize(mDevice, mDepthMap[mDepthMapIndex], D3DXVECTOR3(0, -50, 0), 512, 512);
	mScreenSquare.Initialize(mDevice, mDepthMap[mDepthMapIndex]->SRV, D3DXVECTOR2((float)screenWidth - 100, 0), 100.0f, 100.0f);
}

void Scene::Update(const GameTime& gameTime)
{
	if(GetAsyncKeyState('1'))
		ChangeDepthMap(0);
	else if(GetAsyncKeyState('2'))
		ChangeDepthMap(1);
	else if(GetAsyncKeyState('3'))
		ChangeDepthMap(2);
	else if(GetAsyncKeyState('4'))
		ChangeDepthMap(3);
	else if(GetAsyncKeyState(VK_F1))
		mFloor.SetPCF(false);
	else if(GetAsyncKeyState(VK_F2))
		mFloor.SetPCF(true);

	mObject->Update(gameTime);
	mFloor.Update();
}

void Scene::DrawShadows(const D3DXVECTOR3& eyePos)
{
	SetupShadowMapDrawing();
	mObject->DrawShadows(&(mLightViewMatrix * mLightProjMatrix), eyePos);
}

void Scene::Draw(const Camera& camera)
{
	D3DXMATRIX view, proj, vp;

	view = camera.GetViewMatrix();
	proj = camera.GetProjectionMatrix();
	vp = view * proj;

	mObject->Draw(&vp, camera.GetPos());
	mFloor.Draw(&vp, &(mLightViewMatrix * mLightProjMatrix));
	mScreenSquare.Draw();
}

std::string Scene::GetInfoString() const
{
	std::stringstream stream;
	stream << "Depth texture " << (mDepthMapIndex + 1) << ": " << mDepthMap[mDepthMapIndex]->C_WIDTH;
	stream << "x" << mDepthMap[mDepthMapIndex]->C_HEIGHT;

	if(mFloor.GetPCF())
		stream << ", PCF: ON";
	else
		stream << ", PCF: OFF";

	return stream.str();
}

void Scene::ChangeDepthMap(int newIndex)
{
	mDepthMapIndex = newIndex;

	mViewport.Width = mDepthMap[mDepthMapIndex]->C_WIDTH;
	mViewport.Height = mDepthMap[mDepthMapIndex]->C_HEIGHT;

	mFloor.SetDepthTexture(mDepthMap[mDepthMapIndex]);
	mScreenSquare.SetTexture(mDepthMap[mDepthMapIndex]->SRV);
}

DepthTexture* Scene::CreateDepthTexture(int width, int height)
{
	ID3D10DepthStencilView* dsv;
	ID3D10ShaderResourceView* srv;

	D3D10_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32_TYPELESS;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D10_USAGE_DEFAULT;
	desc.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	ID3D10Texture2D* depthTexture = NULL;
	if(FAILED(mDevice->CreateTexture2D(&desc, NULL, &depthTexture)))
		MessageBox(0, "Error Creating Depth Texture", "", 0);

	D3D10_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	if(FAILED(mDevice->CreateDepthStencilView(depthTexture, &dsvDesc, &dsv)))
		MessageBox(0, "Error Creating Depth Stencil View", "", 0);

	D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	if(FAILED(mDevice->CreateShaderResourceView(depthTexture, &srvDesc, &srv)))
		MessageBox(0, "Error Creating Depth Shader Resource View", "", 0);

	SafeRelease(depthTexture);

	return new DepthTexture(static_cast<float>(width), static_cast<float>(height), dsv, srv);
}

void Scene::CreateLightMatrices()
{
	D3DXMatrixLookAtLH(&mLightViewMatrix, &mLightDirection, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	D3DXMatrixOrthoLH(&mLightProjMatrix, 1000.0f, 1000.0f, 1.0f, 1000.0f);
}

void Scene::SetupShadowMapDrawing()
{
	ID3D10RenderTargetView* renderTargets[1] = { NULL };
	mDevice->OMSetRenderTargets(1, renderTargets, mDepthMap[mDepthMapIndex]->DSV);

	mDevice->RSSetViewports(1, &mViewport);
	mDevice->ClearDepthStencilView(mDepthMap[mDepthMapIndex]->DSV, D3D10_CLEAR_DEPTH, 1.0f, 0);
}

