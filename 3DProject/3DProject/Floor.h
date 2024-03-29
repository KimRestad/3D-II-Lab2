#ifndef FLOOR_H
#define FLOOR_H

#include <D3DX10.h>
#include "Buffer.h"
#include "DepthTexture.h"

struct FloorVertex
{
	D3DXVECTOR3			position;
	D3DXVECTOR2			uv;
};

class Floor
{
public:
	Floor();
	~Floor();
	void Initialize(ID3D10Device* device, DepthTexture* depthTexture, D3DXVECTOR3 position, int width, int depth);
	void Update();
	void Draw(D3DXMATRIX* vpMatrix, D3DXMATRIX* lightWVP);
	void SetDepthTexture(DepthTexture* newDepthTexture);
	void SetPCF(bool newPCF);
	const bool& GetPCF() const;

private:
	ID3D10Device*							mDevice;
	Buffer*									mVertexBuffer;
	ID3D10Effect*							mEffect;
	ID3D10EffectTechnique*					mTechnique;
	ID3D10InputLayout*						mVertexLayout;
	D3DXVECTOR3								mPosition;
	bool									mPCF;

	DepthTexture*							mDepthTexture;
	ID3D10EffectShaderResourceVariable*		mfxDepthTextureVar;
	ID3D10EffectMatrixVariable*				mfxWVP;
	ID3D10EffectMatrixVariable*				mfxLightWVP;
	ID3D10EffectScalarVariable*				mfxPCF;
	ID3D10EffectScalarVariable*				mfxSMWidth;
	ID3D10EffectScalarVariable*				mfxSMWidthInv;

	static const int			C_NUM_VERTICES;
	static const char*			C_FILENAME;

	HRESULT CreateEffect();
	HRESULT CreateVertexLayout();
};
#endif