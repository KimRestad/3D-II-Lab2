#ifndef DEPTH_TEXTURE_H
#define DEPTH_TEXTURE_H

#include <D3D10.h>

struct DepthTexture
{
	const float						C_WIDTH;
	const float						C_HEIGHT;
	const float						C_WIDTH_INV;
	ID3D10DepthStencilView*			DSV;
	ID3D10ShaderResourceView*		SRV;

	DepthTexture(float width, float height, ID3D10DepthStencilView* dsv, ID3D10ShaderResourceView* srv)
		: C_WIDTH(width), C_HEIGHT(height), C_WIDTH_INV(1.0f / width), DSV(dsv), SRV(srv) {}
};

#endif