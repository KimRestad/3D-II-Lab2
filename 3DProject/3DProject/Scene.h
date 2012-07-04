#ifndef SCENE_H
#define SCENE_H

#include <D3D10.h>
#include <vector>
#include <string>

#include "Object3D.h"
#include "Floor.h"
#include "ScreenSquare.h"
#include "GameTime.h"
#include "Camera.h"

class Scene
{
public:
	Scene(ID3D10Device* device, const int& screenWidth);
	void Update(const GameTime& gameTime);
	void DrawShadows(const D3DXVECTOR3& eyePos);
	void Draw(const Camera& camera);

	std::string GetInfoString() const;

private:
	// Light variables
	D3DXVECTOR3						mLightDirection;
	D3DXMATRIX						mLightViewMatrix;
	D3DXMATRIX						mLightProjMatrix;
	
	// Depth map variables
	D3D10_VIEWPORT					mViewport;
	std::vector<DepthTexture*>		mDepthMap;
	int								mDepthMapIndex;

	ID3D10Device*					mDevice;
	Object3D*						mObject;
	Floor							mFloor;
	ScreenSquare					mScreenSquare;

	void ChangeDepthMap(int newIndex);
	DepthTexture* CreateDepthTexture(int width, int height);
	void CreateLightMatrices();
	void SetupShadowMapDrawing();
};
#endif