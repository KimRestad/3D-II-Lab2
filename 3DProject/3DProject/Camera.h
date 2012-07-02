#ifndef CAMERA_H
#define CAMERA_H

#include <D3DX10.h>
#include "GameTime.h"

struct Frustrum
{
	float		nearDistance;
	float		farDistance;
	float		fovY;
	float		aspectRatio;
};

class Camera
{
public:
	Camera(D3DXVECTOR3 position, D3DXVECTOR3 direction, D3DXVECTOR3 worldUp, const Frustrum& viewFrustrum);
	void Update(GameTime gameTime);
	D3DXMATRIX GetViewMatrix() const;
	const D3DXMATRIX& GetProjectionMatrix() const;
	void CreateProjectionMatrix(const Frustrum& viewFrustrum);
	const D3DXVECTOR3& GetPos() const;
	void SetHeight(float height);

private:
	D3DXVECTOR3				mPosition;
	D3DXVECTOR3				mDirection;
	D3DXVECTOR3				mWorldUp;
	D3DXMATRIX				mProjectionMatrix;

	float					mPrevMouseX;
	float					mPrevMouseY;
	
	static const float		C_MOVE_SPEED;
	static const float		C_TILTING_SPEED;
	static const float		C_ZOOM_SPEED;
	static const float		C_ZOOM_MIN;
	static const float		C_ZOOM_MAX;

	D3DXVECTOR3 GetRight() const;
	void MoveLeft(GameTime& gameTime);
	void MoveRight(GameTime& gameTime);
	void MoveForward(GameTime& gameTime);
	void MoveBack(GameTime& gameTime);
	void TurnHorizontal(float angle);
	void TurnVertical(float angle);
};
#endif