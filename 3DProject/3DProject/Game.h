#ifndef GAME_H
#define GAME_H

#include "Camera.h"
#include "D3DApplication.h"
#include "DepthTexture.h"
#include "GameTime.h"
#include "GameFont.h"
#include "Object3D.h"
#include "Floor.h"
#include "ScreenSquare.h"

// 3D II - Lab 2
class Game : public D3DApplication
{
public:
	Game(HINSTANCE applicationInstance, LPCTSTR windowTitle = "GameWindow", 
		UINT windowWidth = CW_USEDEFAULT, UINT windowHeight = CW_USEDEFAULT);
	virtual ~Game();
	virtual void Update();
	virtual void Draw();

private:
	static const int				C_FLOOR_WIDTH = 512;

	// Shadow mapping variables
	D3DXVECTOR3						mLightPos;
	D3DXMATRIX						mLightViewMatrix;
	D3DXMATRIX						mLightProjMatrix;
	Camera*							mLightCam;
	ScreenSquare					mScreenSquare;
	D3D10_VIEWPORT					mViewport;
	std::vector<DepthTexture*>		mDepthMap;
	int								mDepthMapIndex;

	// Game variables
	GameTime						mGameTime;
	GameFont*						mDefaultFont;
	Object3D*						mObject;
	Floor							mFloor;
	Camera*							mCamera;

	std::string						mFPSString;
	int								mNoFrames;
	float							mLastFrameTime;

	void CreateRenderTexture(UINT width, UINT height);
	void DrawToTexture();
	void UpdateViewportMatrix();
	D3DXVECTOR2 TransformToViewport(const D3DXVECTOR2& vector);
	//void CreateShadowMapVertices();
	void SetupShadowMapDrawing();
	void ResetTargetAndViewport();
	void CreateLightMatrices();
	DepthTexture* CreateDepthTexture(int width, int height);
	void ChangeDepthMap(int newIndex);

protected:
	virtual void ProgramLoop();
};
#endif