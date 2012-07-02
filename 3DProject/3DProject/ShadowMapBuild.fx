struct VS_INPUT
{
	float3 posL			: POSITION;
	float2 texC			: TEXCOORD;
};

struct PS_INPUT
{
	float4 posH			: SV_POSITION;
	float2 texC			: TEXCOORD;
};

SamplerState TriLinearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

cbuffer cbPerFrame
{
	float4x4 gLightWVP;
};

//// Nonnumeric values cannot be added to a cbuffer.
//Texture2D gDiffuseMap;

PS_INPUT VS(VS_INPUT vIn)
{
	PS_INPUT vOut;

	vOut.posH = mul(float4(vIn.posL, 1.0f), gLightWVP);
	vOut.texC = vIn.texC;

	return vOut;
}

technique10 BuildShadowMapTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(NULL);
	}
}