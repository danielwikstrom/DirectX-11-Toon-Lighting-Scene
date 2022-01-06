// Colour pixel/fragment shader
// Basic fragment shader outputting a colour

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	//float3 normal : NORMAL;
	float4 colour : COLOR;
};


float4 main(InputType input) : SV_TARGET
{

	float4 textureColor;
	textureColor = shaderTexture.Sample(SampleType, input.tex);
	
	return textureColor;
}