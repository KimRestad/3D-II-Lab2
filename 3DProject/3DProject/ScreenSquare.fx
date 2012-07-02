struct VS_INPUT
{
	float2		position	: POSITION;
	float2		uv			: TEXCOORD;
};

struct PS_INPUT
{
	float4		position	: SV_POSITION;
	float2		uv			: TEXCOORD;
};

RasterizerState NoCulling
{
	CullMode = None;
};

SamplerState linearSampler {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

Texture2D textureBG;

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;

	output.position = float4(float3(input.position, 0.0), 1.0);
	output.uv = input.uv;

	return output;
}

float4 PS(PS_INPUT input) : SV_TARGET0
{
	float depth = textureBG.Sample(linearSampler, input.uv);
	float A = 1000.0f / (1000.0f - 1.0f);
	depth = -A / (depth - A);
	depth *= 0.001f;

	//float A = (1000.0f - 1.0f);
	//float B = 1.0f;
//
	//depth = depth * A + B;

	//depth = pow(depth, 20);

	return float4(depth, depth, depth, 1.0f);
}

technique10 DrawTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));

		SetRasterizerState(NoCulling);
	}
}

