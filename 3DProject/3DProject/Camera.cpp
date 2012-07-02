#include "Camera.h"
#include <cmath>

const float	Camera::C_MOVE_SPEED		= 50.0f;
const float	Camera::C_TILTING_SPEED		= 0.001f;
const float	Camera::C_ZOOM_SPEED		= 6.0f;
const float	Camera::C_ZOOM_MIN			= -200.0f;
const float	Camera::C_ZOOM_MAX			= -10.0f;

Camera::Camera(D3DXVECTOR3 position, D3DXVECTOR3 direction, D3DXVECTOR3 worldUp, const Frustrum& viewFrustrum)
	: mPosition(position), mDirection(direction), mWorldUp(worldUp), mPrevMouseX(0), mPrevMouseY(0)
{
	//mPosition = mPosition - (D3DXVec3Dot(&mPosition, &mWorldUp) * mWorldUp);

	D3DXVec3Normalize(&mDirection, &mDirection);
	CreateProjectionMatrix(viewFrustrum);
}

void Camera::Update(GameTime gameTime)
{
	// Check for movement of the camera.
	if(GetAsyncKeyState('Q'))
		MoveLeft(gameTime);
	else if(GetAsyncKeyState('E'))
		MoveRight(gameTime);

	if(GetAsyncKeyState('W'))
		MoveForward(gameTime);
	else if(GetAsyncKeyState('S'))
		MoveBack(gameTime);

	if(GetAsyncKeyState('A'))
		TurnHorizontal((float)(-gameTime.GetTimeSinceLastTick().Milliseconds) * C_TILTING_SPEED);
	else if(GetAsyncKeyState('D'))
		TurnHorizontal((float)gameTime.GetTimeSinceLastTick().Milliseconds * C_TILTING_SPEED);

	// Check for tilting of camera.
	//if(GetAsyncKeyState(VK_LBUTTON))
	//{
	//	float dx = prevInput.Mouse.x - currInput.Mouse.x;
	//	float dy = prevInput.Mouse.y - currInput.Mouse.y;
	//	//TurnHorizontal(dx * C_TILTING_SPEED);
	//	//TurnVertical(dy * C_TILTING_SPEED);
	//}
}

// Move camera backwards
void Camera::MoveBack(GameTime& gameTime)
{
	D3DXVECTOR3 forward;
	
	forward = mDirection - (D3DXVec3Dot(&mDirection, &mWorldUp) * mWorldUp);
	D3DXVec3Normalize(&forward, &forward);
	mPosition -= forward * C_MOVE_SPEED * (float)gameTime.GetTimeSinceLastTick().Seconds;
}

// Move camera forwards: walk
void Camera::MoveForward(GameTime& gameTime)
{
	D3DXVECTOR3 forward;

	forward = mDirection - (D3DXVec3Dot(&mDirection, &mWorldUp) * mWorldUp);
	D3DXVec3Normalize(&forward, &forward);
	mPosition += forward * C_MOVE_SPEED * (float)gameTime.GetTimeSinceLastTick().Seconds;
}

// Move camera to the left: strafe
void Camera::MoveLeft(GameTime& gameTime)
{
	D3DXVECTOR3 right = GetRight();
	mPosition -= right * C_MOVE_SPEED * (float)gameTime.GetTimeSinceLastTick().Seconds;
}

// Move camera to the right: strafe
void Camera::MoveRight(GameTime& gameTime)
{
	D3DXVECTOR3 right = GetRight();
	mPosition += right * C_MOVE_SPEED * (float)gameTime.GetTimeSinceLastTick().Seconds;
}

// Tilt the camera horisontally: look left/right
void Camera::TurnHorizontal(float angle)
{
	D3DXVECTOR3 up;
	D3DXVec3Cross(&up, &mDirection, &GetRight());
	D3DXMATRIX rotation;

	D3DXMatrixRotationAxis(&rotation, &up, angle);
	D3DXVec3TransformCoord(&mDirection, &mDirection, &rotation);
	D3DXVec3Normalize(&mDirection, &mDirection);
}

// Tilt the camera vertically: look up/down
void Camera::TurnVertical(float angle)
{
	D3DXVECTOR3 right = GetRight();
	D3DXMATRIX rotation;

	D3DXMatrixRotationAxis(&rotation, &right, angle);
	D3DXVec3TransformCoord(&mDirection, &mDirection, &rotation);
	D3DXVec3Normalize(&mDirection, &mDirection);
}

// Create and return the view matrix for the camera
D3DXMATRIX Camera::GetViewMatrix() const
{
	D3DXVECTOR3 right, up, eyePos;
	D3DXMATRIX view;

	eyePos = mPosition;

	right = GetRight();
	D3DXVec3Cross(&up, &mDirection, &right);
	
	view.m[0][0] = right.x;
	view.m[1][0] = right.y;
	view.m[2][0] = right.z;

	view.m[0][1] = up.x;
	view.m[1][1] = up.y;
	view.m[2][1] = up.z;

	view.m[0][2] = mDirection.x;
	view.m[1][2] = mDirection.y;
	view.m[2][2] = mDirection.z;

	view.m[0][3] = 0;
	view.m[1][3] = 0;
	view.m[2][3] = 0;
	view.m[3][3] = 1;

	view.m[3][0] = -D3DXVec3Dot(&eyePos, &right);
	view.m[3][1] = -D3DXVec3Dot(&eyePos, &up);
	view.m[3][2] = -D3DXVec3Dot(&eyePos, &mDirection);

	return view;
}

// Return the projection matrix for the camera
const D3DXMATRIX& Camera::GetProjectionMatrix() const
{
	return mProjectionMatrix;
}

// Create the projection matrix for the camera
void Camera::CreateProjectionMatrix(const Frustrum& viewFrustrum)
{
	ZeroMemory(&mProjectionMatrix, sizeof(mProjectionMatrix));
	
	float scaleY, scaleX, length;
	scaleY = 1.0f / std::tan(viewFrustrum.fovY * 0.5f);
	scaleX = scaleY / viewFrustrum.aspectRatio;
	length = viewFrustrum.farDistance - viewFrustrum.nearDistance;

	mProjectionMatrix.m[0][0] = scaleX;
	mProjectionMatrix.m[1][1] = scaleY;
	mProjectionMatrix.m[2][2] = viewFrustrum.farDistance / length;
	mProjectionMatrix.m[2][3] = 1;
	mProjectionMatrix.m[3][2] = (-viewFrustrum.nearDistance * viewFrustrum.farDistance) / length;
}

// Get the camera's right vector (camera x-axis)
D3DXVECTOR3 Camera::GetRight() const
{
	D3DXVECTOR3 right;
	D3DXVec3Cross(&right, &mWorldUp, &mDirection);
	D3DXVec3Normalize(&right, &right);

	return right;
}

// Get the cameras current position in the world
const D3DXVECTOR3& Camera::GetPos() const
{
	return mPosition;
}

void Camera::SetHeight(float height)
{
	mPosition.y = height;
}

//Camera::Camera(D3DXVECTOR3 position, D3DXVECTOR3 direction, D3DXVECTOR3 worldUp)
//	: mPosition(position), mDirection(direction), mSpeed(0.05f), mWorldUp(worldUp)
//{
//	D3DXVec3Normalize(&mDirection, &mDirection);
//}
//
//// Move camera backwards: walk
//void Camera::MoveBack(bool running)
//{
//	D3DXVECTOR3 forward;
//	float runFactor = 1.0f;
//	if(running)
//		runFactor = 2.0f;
//	
//	forward = mDirection - (D3DXVec3Dot(&mDirection, &mWorldUp) * mWorldUp);
//	D3DXVec3Normalize(&forward, &forward);
//	mPosition -= forward * mSpeed * runFactor;
//}
//
//// Move camera forwards: walk
//void Camera::MoveForward(bool running)
//{
//	D3DXVECTOR3 forward;
//	float runFactor = 1.0f;
//	if(running)
//		runFactor = 2.0f;
//
//	forward = mDirection - (D3DXVec3Dot(&mDirection, &mWorldUp) * mWorldUp);
//	D3DXVec3Normalize(&forward, &forward);
//	mPosition += forward * mSpeed * runFactor;
//}
//
//// Move camera to the left: strafe
//void Camera::MoveLeft(bool running)
//{
//	float runFactor = 1.0f;
//	if(running)
//		runFactor = 2.0f;
//
//	D3DXVECTOR3 right = GetRight();
//	mPosition -= right * mSpeed * runFactor;
//}
//
//// Move camera to the right: strafe
//void Camera::MoveRight(bool running)
//{
//	float runFactor = 1.0f;
//	if(running)
//		runFactor = 2.0f;
//
//	D3DXVECTOR3 right = GetRight();
//	mPosition += right * mSpeed* runFactor;
//}
//
//// Tilt the camera horisontally: look left/right
//void Camera::TurnHorizontal(float angle)
//{
//	D3DXVECTOR3 up;
//	D3DXVec3Cross(&up, &mDirection, &GetRight());
//	D3DXMATRIX rotation;
//
//	D3DXMatrixRotationAxis(&rotation, &up, angle);
//	D3DXVec3TransformCoord(&mDirection, &mDirection, &rotation);
//	D3DXVec3Normalize(&mDirection, &mDirection);
//}
//
//// Tilt the camera vertically: look up/down
//void Camera::TurnVertical(float angle)
//{
//	D3DXVECTOR3 right = GetRight();
//	D3DXMATRIX rotation;
//
//	D3DXMatrixRotationAxis(&rotation, &right, angle);
//	D3DXVec3TransformCoord(&mDirection, &mDirection, &rotation);
//	D3DXVec3Normalize(&mDirection, &mDirection);
//}
//
//// Create and return the view matrix for the camera
//D3DXMATRIX Camera::GetViewMatrix()
//{
//	D3DXVECTOR3 right, up;
//	D3DXMATRIX view;
//
//	D3DXVec3Normalize(&mDirection, &mDirection);
//	right = GetRight();
//	D3DXVec3Cross(&up, &mDirection, &right);
//	
//	view.m[0][0] = right.x;
//	view.m[1][0] = right.y;
//	view.m[2][0] = right.z;
//
//	view.m[0][1] = up.x;
//	view.m[1][1] = up.y;
//	view.m[2][1] = up.z;
//
//	view.m[0][2] = mDirection.x;
//	view.m[1][2] = mDirection.y;
//	view.m[2][2] = mDirection.z;
//
//	view.m[0][3] = 0;
//	view.m[1][3] = 0;
//	view.m[2][3] = 0;
//	view.m[3][3] = 1;
//
//	view.m[3][0] = -D3DXVec3Dot(&mPosition, &right);
//	view.m[3][1] = -D3DXVec3Dot(&mPosition, &up);
//	view.m[3][2] = -D3DXVec3Dot(&mPosition, &mDirection);
//
//	return view;
//}
//
//// Get the camera's right vector (camera x-axis)
//D3DXVECTOR3 Camera::GetRight()
//{
//	D3DXVECTOR3 right;
//	D3DXVec3Cross(&right, &mWorldUp, &mDirection);
//	D3DXVec3Normalize(&right, &right);
//
//	return right;
//}
//
//// Get the cameras current position in the world
//D3DXVECTOR3 Camera::GetPos()
//{
//	return mPosition;
//}
//
//void Camera::SetHeight(float height)
//{
//	mPosition.y = height;
//}