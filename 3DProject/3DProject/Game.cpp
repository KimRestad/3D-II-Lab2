#include "Game.h"
#include <sstream>

Game::Game(HINSTANCE applicationInstance, LPCTSTR windowTitle, UINT windowWidth, UINT windowHeight)
	: D3DApplication(applicationInstance, windowTitle, windowWidth, windowHeight), mGameTime(),
	  mNoFrames(0), mFPSString(""), mLastFrameTime(0), mScene(NULL), mCamera(NULL)
{
	Frustrum camFrustrum;
	camFrustrum.aspectRatio = (float)(mScreenWidth / mScreenHeight);
	camFrustrum.farDistance = 1000.0f;
	camFrustrum.nearDistance = 1.0f;
	camFrustrum.fovY = (float)D3DX_PI * 0.3f;
	mCamera = new Camera(D3DXVECTOR3(-100.0f, 50.0f, -100.0f), D3DXVECTOR3(3.0f, -1.0f, 3.0f), 
						 D3DXVECTOR3(0.0f, 1.0f, 0.0f), camFrustrum);

	mScene = new Scene(mDeviceD3D, mScreenWidth);
	mDefaultFont = new GameFont(mDeviceD3D, "Times New Roman", 21);
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
	mScene->Update(mGameTime);

	++mNoFrames;
	mLastFrameTime += (float)mGameTime.GetTimeSinceLastTick().Milliseconds;

	if(mLastFrameTime > 1000.0f)
	{
		std::stringstream stream;
		stream << mScene->GetInfoString() << "\n";
		stream << "FPS: " << mNoFrames;
		mFPSString = stream.str();

		mLastFrameTime -= 1000;
		mNoFrames = 0;
	}
}

// Draw the scene
void Game::Draw()
{
	mScene->DrawShadows(mCamera->GetPos());

	ResetTargetAndViewport();
	ClearScene();

	mScene->Draw(*mCamera);

	RECT textPos = { 0, 0, 300, 300 };
	mDefaultFont->WriteText(mFPSString, &textPos, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f), GameFont::Left, GameFont::Top);

	RenderScene();
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