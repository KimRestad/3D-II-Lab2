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
	bool gPCF;
	float gSMWidth;
	float gSMWidthInv;
};

float gSMEpsilon = 0.001f;
Texture2D gTextureGround;
Texture2D gShadowMapTex;

// ************************************************************************
// ** HELPER FUNCTIONS
// ************************************************************************

float CalcShadowFactor(float2 uv, float depth)
{
	float shadowDepth = gShadowMapTex.Sample(pointSampler, uv).r;

	float shadowFactor = 1.0f;

	if (depth > shadowDepth)
		shadowFactor = shadowDepth;

	return shadowFactor;
}

float CalcShadowFactorPCF(float2 uv, float depth)
{
	//if(uv.x < -1.0f || uv.x > 1.0f)
		//return 0.0f;
	//if(uv.y < -1.0f || uv.y > 1.0f)
		//return 0.0f;
	//if(depth < 0.0f)
		//return 0.0f;

	float sample0 = gShadowMapTex.Sample(pointSampler, uv).r;
	float sample1 = gShadowMapTex.Sample(pointSampler, uv + float2(gSMWidthInv, 0.0f)).r;
	float sample2 = gShadowMapTex.Sample(pointSampler, uv + float2(0.0f, gSMWidthInv)).r;
	float sample3 = gShadowMapTex.Sample(pointSampler, uv + float2(gSMWidthInv, gSMWidthInv)).r;

	float depth0 = depth <= sample0 + gSMEpsilon;
	float depth1 = depth <= sample1 + gSMEpsilon;
	float depth2 = depth <= sample2 + gSMEpsilon;
	float depth3 = depth <= sample3 + gSMEpsilon;

	float2 texPos = uv * gSMWidth;

	float2 t = frac(texPos);

	return lerp(lerp(depth0, depth1, t.x), lerp(depth2, depth3, t.x), t.y);
}

// ************************************************************************
// ** SHADER FUNCTIONS
// ************************************************************************
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;

	output.position = mul(float4(input.position, 1.0f), gWVP);
	output.positionW = input.position;
	output.uv = input.uv;

	return output;
}

float4 PS(PS_INPUT input) : SV_Target0
{
	float4 texColor = gTextureGround.Sample(linearSampler, input.uv);

	// Calculate shadows
	float4 posLightWVP = mul(float4(input.positionW, 1.0f), gLightWVP);
	posLightWVP /= posLightWVP.w;

	posLightWVP.x = posLightWVP.x * 0.5f + 0.5f;
	posLightWVP.y = posLightWVP.y * -0.5f + 0.5f;

	if(gPCF)
		return texColor * CalcShadowFactorPCF(posLightWVP.xy, posLightWVP.z);
	else
		return texColor * CalcShadowFactor(posLightWVP.xy, posLightWVP.z);
}

// ************************************************************************
// ** TECHNIQUES
// ************************************************************************
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

