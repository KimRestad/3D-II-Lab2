struct VS_INPUT
{
	float3		position	: POSITION;
	float2		uv			: UV;
};

struct PS_INPUT
{
	float4		position	: SV_POSITION;
	float3		positionW	: POSITION;
	float2		uv			: UV;
};

RasterizerState NoCulling
{
	CullMode = None;
	//FillMode = Wireframe;
};

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
	DepthFunc = LESS_EQUAL;
};

SamplerState linearSampler {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState pointSampler {
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
};

cbuffer cbEveryFrame
{
	matrix gWVP;
	matrix gLightWVP;
};

Texture2D gTextureGround;
Texture2D gShadowMapTex;

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;

	output.position = mul(float4(input.position, 1.0f), gWVP);
	output.positionW = input.position;
	//output.position = float4(input.position, 1.0);
	output.uv = input.uv;

	return output;
}

float4 PS(PS_INPUT input) : SV_Target0
{
	float4 texColor = gTextureGround.Sample(linearSampler, input.uv);

	// Calculate shadows
	float4 posLightWVP = mul(float4(input.positionW, 1.0f), gLightWVP);
	posLightWVP = posLightWVP / posLightWVP.w;

	posLightWVP.x = posLightWVP.x * 0.5f + 0.5f;
	posLightWVP.y = posLightWVP.y * -0.5f + 0.5f;

	float shadowDepth = gShadowMapTex.Sample(pointSampler, posLightWVP.xy);

	float shadowFactor = 1.0f;

	if (posLightWVP.z > shadowDepth)
		shadowFactor = shadowDepth;

	return texColor * shadowFactor;
}

technique10 DrawTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));

		SetRasterizerState(NoCulling);
		SetDepthStencilState(EnableDepth, 0xff);
	}
}

