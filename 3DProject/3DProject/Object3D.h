#ifndef OBJECT3D_H
#define OBJECT3D_H

#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <D3D10.h>

#include "Globals.h"
#include "Buffer.h"
#include "GameFont.h"
#include "GameTime.h"

class Object3D
{
public:
	Object3D(ID3D10Device* device, std::string filename, D3DXVECTOR3 position, D3DXVECTOR3 lightPos);
	~Object3D();
	
	void Update(GameTime gameTime);
	void Draw(D3DXMATRIX* vpMatrix, D3DXVECTOR3 eyePos);

private:
	struct Vertex
	{
		D3DXVECTOR3			Position;
		D3DXVECTOR3			Normal;
		D3DXVECTOR2			UV;
	};

	struct MaterialInfo
	{
		D3DXVECTOR3 Ambient;					// Ka coefficient, default (0.2, 0.2, 0.2)
		D3DXVECTOR3 Diffuse;					// Kd coefficient, default (0.8, 0.8, 0.8)
		D3DXVECTOR3 Specular;					// Ks coefficient, default (1.0, 1.0, 1.0)
		D3DXVECTOR3 Tf;							// Transmission filter, how much of each color to pass through
		int IlluminationModel;					// illum, 0-10
		//float Opacitiy;						// d or Tr, 1.0-> fully opaque, 0.0-> fully transparent
		float RefractionIndex;					// Ni, optical density, (0.001)1.0-10.0, 1.0-> light doesn't bend
		float SpecularExp;						// Ns, ~0-1000, High value-> concentrated highlight
		//float Sharpness;						// Sharpness of reflections, 0-1000, default 60.0
		ID3D10ShaderResourceView* MainTexture;	// Texture that can be sent to the shaders

		MaterialInfo();
	};

	struct Group
	{
	public:
		MaterialInfo* Material;
		Buffer*						mVertexBuffer;
		std::vector<Vertex>			mVertices;

		Group();
		~Group() throw();
		void AddVertices(std::vector<Vertex> vertexList);
		bool Finalize(ID3D10Device* device, ID3D10Effect* effect);
		void Draw(ID3D10Device* device);

	private:
		ID3D10EffectShaderResourceVariable* mFXTexture;
		ID3D10EffectVectorVariable* mFXKa;
		ID3D10EffectVectorVariable* mFXKd;
		ID3D10EffectVectorVariable* mFXKs;
		ID3D10EffectScalarVariable* mFXSpecExp;
		/*Group(const Group&);
		Group& operator=(const Group&);*/
	};

	std::map<std::string, MaterialInfo> mMaterials;
	std::map<std::string, Group> mGroups;

	ID3D10Device*				mDevice;
	ID3D10Effect*				mEffect;
	ID3D10EffectTechnique*		mTechnique;

	ID3D10InputLayout*			mVertexLayout;
	GameFont*					mFont;

	D3DXMATRIX*					mMatrixWorld;
	D3DXVECTOR3					mPosition;
	D3DXVECTOR3					mVelocity;
	float						mRotation;
	D3DXVECTOR3					mLightPosition;

	ID3D10EffectMatrixVariable* mFXWorld;
	ID3D10EffectMatrixVariable* mFXWorldViewProj;
	ID3D10EffectVectorVariable* mFXLightPos;
	ID3D10EffectVectorVariable* mFXEyePos;

	bool Load(std::string filename);
	bool LoadMaterials(std::string filename);

	HRESULT CreateEffect();
	HRESULT CreateVertexLayout();
	void UpdateWorldMatrix();
};
#endif