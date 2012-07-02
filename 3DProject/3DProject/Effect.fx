struct VS_INPUT
{
	float3		position	: POSITION;
	float3		normal		: NORMAL;
	float2		uv			: TEXCOORD;
};

struct PS_INPUT
{
	float4		position	: SV_POSITION;
	float3		positionW	: POSITION;
	float3		normalW		: NORMAL;
	float2		uv			: TEXCOORD;
};

RasterizerState NoCulling
{
	CullMode = None;
	//FillMode = Wireframe;
};

SamplerState linearSampler {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

cbuffer cbEveryFrame
{
	matrix gWorld;
	matrix gWVP;
	float4 gEyePos;
	float3 gLightPosition;
};

float3 gKd;
float3 gKa;
float3 gKs;
float gSExp;
//float3 Tf = float3(1.0, 1.0, 1.0);
//int illum = 1;
//float refrac = 1.0f;

Texture2D gTextureBTH;
//Texture2D gShadowMapTex;
bool gDrawLight = true;

// ************************************************************************
// ** HELPER FUNCTIONS
// ************************************************************************
float4 GetColorBasicLight(PS_INPUT input)
{
	float3 lightVec = normalize(gLightPosition - input.positionW);
	float dotProd = dot(lightVec, normalize(input.normalW));

	float4 texColor = gTextureBTH.Sample(linearSampler, input.uv);
	texColor = texColor * dotProd;
	return texColor;
}

float4 GetColorLight(PS_INPUT input)
{
	float3 lightDiffuse = float3(0.8f, 0.8f, 0.8f);
	float3 lightAmbient = float3(1.0f, 0.0f, 0.0f);
	float3 lightSpecular = float3(1.0f, 1.0f, 1.0f);

	float3 lightVec = normalize(gLightPosition - input.positionW);
	float dotLN = dot(lightVec, normalize(input.normalW));
	
	float constD = max(dotLN, 0.0f);
	float constS = 0.0f;
	float3 toEye = normalize(gEyePos - input.positionW);

	if(dotLN > 0.0f)
	{
		float3 reflectLight = reflect(-lightVec, input.normalW);
		constS = pow(max(dot(reflectLight, toEye), 0.0f), gSExp);
	}

	float3 diffuseCol = lightDiffuse * gKd * constD;
	float3 ambientCol = lightAmbient * gKa;
	float3 specularCol = lightSpecular * gKs * constS;
	
	float3 lightCol = diffuseCol + ambientCol + specularCol;
	float4 texColor = gTextureBTH.Sample(linearSampler, input.uv);

	return texColor * float4(lightCol, 1.0f);
}

// ************************************************************************
// ** SHADER FUNCTIONS
// ************************************************************************

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;

	output.position = mul(float4(input.position, 1.0), gWVP);
	output.positionW = mul(float4(input.position, 1.0), gWorld).xyz;
	output.normalW = mul(float4(input.normal, 0.0), gWorld).xyz;
	output.uv = input.uv;

	return output;
}

float4 PS(PS_INPUT input) : SV_Target0
{
	if(gDrawLight)
		return GetColorLight(input);
	else
		return GetColorBasicLight(input);
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
	}
}

