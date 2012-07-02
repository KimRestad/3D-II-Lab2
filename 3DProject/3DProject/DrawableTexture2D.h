#ifndef DRAWABLE_TEXTURE2D_H
#define DRAWABLE_TEXTURE2D_H

#include <D3DX10.h>

class DrawableTexture2D
{
public:
	DrawableTexture2D();
	~DrawableTexture2D();

	void Initialize(ID3D10Device* device, UINT width, UINT height, bool hasColorMap, DXGI_FORMAT colorFormat);
	ID3D10ShaderResourceView* GetColorMap();
	ID3D10ShaderResourceView* GetDepthMap();

	void BeginDrawing();
	void EndDrawing();

private:
	ID3D10Device*				mDevice;
	ID3D10ShaderResourceView*	mSRVColorMap;
	ID3D10RenderTargetView*		mRTVColorMap;
	ID3D10ShaderResourceView*	mSRVDepthMap;
	ID3D10DepthStencilView*		mDSVDepthMap;
	D3D10_VIEWPORT				mViewport;

	UINT						mWidth;
	UINT						mHeight;
	DXGI_FORMAT					mColorMapFormat;

	DrawableTexture2D(const DrawableTexture2D& copy);
	DrawableTexture2D& operator=(const DrawableTexture2D& copy);

	void BuildDepthMap();
	void BuildColorMap();
};
#endif