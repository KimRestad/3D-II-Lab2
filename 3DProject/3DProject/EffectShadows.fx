struct VS_INPUT
{
	float3		position	: POSITION;
	float3		normal		: NORMAL;
	float2		uv			: TEXCOORD;
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

cbuffer cbEveryFrame
{
	matrix gWorld;
	matrix gWVP;
	float4 gEyePos;
	float3 gLightPosition;
};

// ************************************************************************
// ** SHADER FUNCTIONS
// ************************************************************************
float4 VS(VS_INPUT input) :  SV_POSITION
{
	return mul(float4(input.position, 1.0), gWVP);
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
		SetPixelShader(NULL);

		SetRasterizerState(NoCulling);
		SetDepthStencilState(EnableDepth, 0xff);
	}
}