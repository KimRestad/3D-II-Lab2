#include "Game.h"
#include <sstream>

Game::Game(HINSTANCE applicationInstance, LPCTSTR windowTitle, UINT windowWidth, UINT windowHeight)
	: D3DApplication(applicationInstance, windowTitle, windowWidth, windowHeight), mGameTime(),
	  mNoFrames(0), mFPSString(""), mLastFrameTime(0), mLightPos(D3DXVECTOR3(-300.0f, 50.0f, -300.0f))
{
	mDefaultFont = new GameFont(mDeviceD3D, "Times New Roman", 24);
	mObject = new Object3D(mDeviceD3D, "bth.obj", D3DXVECTOR3(0.0, 0.0, 0.0), mLightPos);
	
	Frustrum camFrustrum;
	camFrustrum.aspectRatio = (float)(mScreenWidth / mScreenHeight);
	camFrustrum.farDistance = 1000.0f;
	camFrustrum.nearDistance = 1.0f;
	camFrustrum.fovY = (float)D3DX_PI * 0.3f;
	mCamera = new Camera(D3DXVECTOR3(-100.0f, 50.0f, -100.0f), D3DXVECTOR3(1.0f, -1.0f, 1.0f), D3DXVECTOR3(0.0f, 1.0f, 0.0f), camFrustrum);

	// DEBUG
	//mLightPos = mCamera->GetPos();

	// Shadow map things
	const int C_SM_WIDTH = 256;
	const int C_SM_HEIGHT = 256;

	D3D10_TEXTURE2D_DESC desc;
	desc.Width = C_SM_WIDTH;
	desc.Height = C_SM_HEIGHT;
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
	if(FAILED(mDeviceD3D->CreateTexture2D(&desc, NULL, &depthTexture)))
		MessageBox(0, "Error Creating Depth Texture", "", 0);

	D3D10_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	if(FAILED(mDeviceD3D->CreateDepthStencilView(depthTexture, &dsvDesc, &mDepthMapDSV)))
		MessageBox(0, "Error Creating Depth Stencil View", "", 0);

	D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	if(FAILED(mDeviceD3D->CreateShaderResourceView(depthTexture, &srvDesc, &mDepthMapSRV)))
		MessageBox(0, "Error Creating Depth Shader Resource View", "", 0);

	SafeRelease(depthTexture);

	ZeroMemory(&mViewport, sizeof(D3D10_VIEWPORT));
	mViewport.TopLeftX = 0;
	mViewport.TopLeftY = 0;
	mViewport.Width = C_SM_WIDTH;
	mViewport.Height = C_SM_HEIGHT;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;

	CreateLightMatrices();

	mFloor.Initialize(mDeviceD3D, mDepthMapSRV, D3DXVECTOR3(0, -50, 0), C_HALF_WIDTH * 2, C_HALF_WIDTH * 2);
	//mShadowMap.Initialize(mDeviceD3D, 256, 256, false, DXGI_FORMAT_UNKNOWN);
	mScreenSquare.Initialize(mDeviceD3D, mDepthMapSRV, D3DXVECTOR2((float)mScreenWidth - 100, 0), 100.0f, 100.0f);
}

Game::~Game()
{
}

//  What happens every loop of the program (ie updating and drawing the game)
void Game::ProgramLoop()
{
	Update();
	Draw();
}

// Update the game
void Game::Update()
{
	if(GetAsyncKeyState(VK_ESCAPE))
		Quit();

	mGameTime.Update();
	mCamera->Update(mGameTime);
	mObject->Update(mGameTime);
	mFloor.Update();

	++mNoFrames;
	mLastFrameTime += (float)mGameTime.GetTimeSinceLastTick().Seconds;

	std::stringstream stream;
	stream << "FPS: " << mNoFrames / mLastFrameTime;
	mFPSString = stream.str();

	if(mLastFrameTime > 1000.0f)
	{
		mLastFrameTime -= 1000;
		mNoFrames = 0;
	}
}

// Draw the scene
void Game::Draw()
{
	D3DXMATRIX view, proj, vp;

	view = mCamera->GetViewMatrix();
	proj = mCamera->GetProjectionMatrix();
	vp = view * proj;

	//mShadowMap.BeginDrawing();
	SetupShadowMapDrawing();
	mObject->Draw(&(mLightViewMatrix * mLightProjMatrix), mCamera->GetPos());
	//mFloor.Draw(&(mLightViewMatrix * mLightProjMatrix), &(mLightViewMatrix * mLightProjMatrix));
	
	//mShadowMap.EndDrawing();

	ResetTargetAndViewport();
	ClearScene();
	
	mObject->Draw(&vp, mCamera->GetPos());
	mFloor.Draw(&vp, &(mLightViewMatrix * mLightProjMatrix));
	mScreenSquare.Draw();

	RenderScene();
}

void Game::SetupShadowMapDrawing()
{
	ID3D10RenderTargetView* renderTargets[1] = { NULL };
	mDeviceD3D->OMSetRenderTargets(1, renderTargets, mDepthMapDSV);

	mDeviceD3D->RSSetViewports(1, &mViewport);
	mDeviceD3D->ClearDepthStencilView(mDepthMapDSV, D3D10_CLEAR_DEPTH, 1.0f, 0);
}

void Game::WrapUpShadowMapDrawing()
{
}

void Game::ResetTargetAndViewport()
{
	mDeviceD3D->OMSetRenderTargets(1, &mRenderTarget, mDepthStencilView);

	D3D10_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = mScreenWidth;
	vp.Height = mScreenHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	mDeviceD3D->RSSetViewports(1, &vp);
}

void Game::CreateLightMatrices()
{
	D3DXMatrixIdentity(&mLightViewMatrix);
	//D3DXMatrixIdentity(&mLightProjMatrix);

	/*D3DXVECTOR3 transformedCenter;
	D3DXVec3Transform(&transformedCenter,d3d*/

	D3DXMatrixLookAtLH(&mLightViewMatrix, &mLightPos, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	//D3DXMatrixOrthoLH(&mLightProjMatrix, 50.0f, 50.0f, 1.0f, 200.0f);

	ZeroMemory(&mLightProjMatrix, sizeof(mLightProjMatrix));
	
	float scaleY, scaleX, length;
	scaleY = 1.0f / std::tan((float)D3DX_PI * 0.3f * 0.5f);
	scaleX = scaleY / ((float)mScreenWidth / mScreenHeight);
	length = 1000.0f - 1.0f;

	mLightProjMatrix.m[0][0] = scaleX;
	mLightProjMatrix.m[1][1] = scaleY;
	mLightProjMatrix.m[2][2] = 1000.0f / length;
	mLightProjMatrix.m[2][3] = 1;
	mLightProjMatrix.m[3][2] = (-1.0f * 1000.0f) / length;
}