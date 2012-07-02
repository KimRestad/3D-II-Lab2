#include "DrawableTexture2D.h"
#include "Globals.h"

DrawableTexture2D::DrawableTexture2D()
	: mWidth(0), mHeight(0), mColorMapFormat(DXGI_FORMAT_UNKNOWN),
	  mDevice(0), mSRVColorMap(0), mRTVColorMap(0), mSRVDepthMap(0),
	  mDSVDepthMap(0)
{
	ZeroMemory(&mViewport, sizeof(D3D10_VIEWPORT));
}

DrawableTexture2D::~DrawableTexture2D()
{
	SafeRelease(mSRVColorMap);
	SafeRelease(mRTVColorMap);
	SafeRelease(mSRVDepthMap);
	SafeRelease(mDSVDepthMap);
}

void DrawableTexture2D::Initialize(ID3D10Device* device, UINT width, UINT height, bool hasColorMap, DXGI_FORMAT colorFormat)
{
	mDevice = device;
	mWidth = width;
	mHeight = height;
	mColorMapFormat = colorFormat;			// Specify pixel format of color buffer
	
	BuildDepthMap();
	
	if(hasColorMap)
		BuildColorMap();
	
	// The part (subrectangle) of the rendertarget to draw to, (0, 0, width, height) draws to entire render target
	mViewport.TopLeftX = 0;
	mViewport.TopLeftY = 0;
	mViewport.Width = width;
	mViewport.Height = height;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;
}

ID3D10ShaderResourceView* DrawableTexture2D::GetColorMap()
{
	return mSRVColorMap;
}

ID3D10ShaderResourceView* DrawableTexture2D::GetDepthMap()
{
	return mSRVDepthMap;
}

void DrawableTexture2D::BuildDepthMap()
{
	HRESULT result					= S_OK;
	ID3D10Texture2D* depthMap = 0;

	D3D10_TEXTURE2D_DESC textureDesc;
	textureDesc.Width				= mWidth;
	textureDesc.Height				= mHeight;
	textureDesc.MipLevels			= 1;
	textureDesc.ArraySize			= 1;
	textureDesc.Format				= DXGI_FORMAT_R32_TYPELESS;
	textureDesc.SampleDesc.Count	= 1;
	textureDesc.SampleDesc.Quality	= 0;
	textureDesc.Usage				= D3D10_USAGE_DEFAULT;
	textureDesc.BindFlags			= D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags		= 0;
	textureDesc.MiscFlags			= 0;

	result = mDevice->CreateTexture2D(&textureDesc, 0, &depthMap);

	D3D10_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	result = mDevice->CreateDepthStencilView(depthMap, &dsvDesc, &mDSVDepthMap);
	
	D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	result = mDevice->CreateShaderResourceView(depthMap, &srvDesc, &mSRVDepthMap);
	
	// View saves a reference to the texture so we can
	// release our reference.
	SafeRelease(depthMap);
}

void DrawableTexture2D::BuildColorMap()
{
	HRESULT result			= S_OK;
	ID3D10Texture2D* colorMap = 0;

	D3D10_TEXTURE2D_DESC textureDesc;
	textureDesc.Width				= mWidth;
	textureDesc.Height				= mHeight;
	textureDesc.MipLevels			= 0;
	textureDesc.ArraySize			= 1;
	textureDesc.Format				= mColorMapFormat;
	textureDesc.SampleDesc.Count	= 1;
	textureDesc.SampleDesc.Quality	= 0;
	textureDesc.Usage				= D3D10_USAGE_DEFAULT;
	textureDesc.BindFlags			= D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags		= 0;
	textureDesc.MiscFlags			= D3D10_RESOURCE_MISC_GENERATE_MIPS;
	
	result = mDevice->CreateTexture2D(&textureDesc, 0, &colorMap);
	
	// Null description means to create a view to all mipmap levels
	// using the format the texture was created with.
	result = mDevice->CreateRenderTargetView(colorMap, 0, &mRTVColorMap);
	result = mDevice->CreateShaderResourceView(colorMap, 0, &mSRVColorMap);

	// View saves a reference to the texture so we can
	// release our reference.
	SafeRelease(colorMap);
}

void DrawableTexture2D::BeginDrawing()
{
	ID3D10RenderTargetView* renderTargets[1] = { mRTVColorMap };
	mDevice->OMSetRenderTargets(1, renderTargets, mDSVDepthMap);

	mDevice->RSSetViewports(1, &mViewport);
	
	const float colorBlack[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	// only clear if we actually created a color map.
	if(mRTVColorMap)
		mDevice->ClearRenderTargetView(mRTVColorMap, colorBlack);
	
	mDevice->ClearDepthStencilView(mDSVDepthMap, D3D10_CLEAR_DEPTH, 1.0f, 0);
}

void DrawableTexture2D::EndDrawing()
{
	if(mSRVColorMap)
		mDevice->GenerateMips(mSRVColorMap);
}