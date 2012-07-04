#ifndef GAME_H
#define GAME_H

#include "Camera.h"
#include "D3DApplication.h"
#include "GameTime.h"
#include "GameFont.h"
#include "Scene.h"

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
	GameTime						mGameTime;
	GameFont*						mDefaultFont;
	Scene*							mScene;
	Camera*							mCamera;

	std::string						mFPSString;
	int								mNoFrames;
	float							mLastFrameTime;

	void ResetTargetAndViewport();

protected:
	virtual void ProgramLoop();
};
#endif