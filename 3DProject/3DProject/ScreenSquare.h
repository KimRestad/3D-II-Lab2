#ifndef SCREEN_SQUARE_H
#define SCREEN_SQUARE_H

#include <D3DX10.h>

#include "Buffer.h"

class ScreenSquare
{
public:
	ScreenSquare();

	void Initialize(ID3D10Device* device, ID3D10ShaderResourceView* drawTexture, D3DXVECTOR2 position, float width, float height);
	void Draw();

private:
	struct SSVertex
	{
		D3DXVECTOR2				position;
		D3DXVECTOR2				uv;
	};

	ID3D10Device*				mDevice;
	ID3D10Effect*				mEffect;
	ID3D10EffectTechnique*		mTechnique;
	Buffer*						mBuffer;
	ID3D10InputLayout*			mVertexLayout;
	ID3D10ShaderResourceView*	mDrawTexture;

	D3DXMATRIX				mViewportMatrix;

	void CreateBuffer(D3DXVECTOR2 position, float width, float height);
	void CreateEffect();
	void CreateVertexLayout();
	D3DXVECTOR2 TransformToViewport(const D3DXVECTOR2& vector);
	void UpdateViewportMatrix(int newWidth, int newHeight);
	void SetTexture();
	void ClearTexture();
};
#endif