// Light pixel shader
// Calculate diffuse lighting for a single directional light(also texturing)

Texture2D shaderTexture : register(t0);
Texture2D noiseTexture : register(t1);
Texture2D alphaTexture : register(t2);
SamplerState SampleType : register(s0);


cbuffer LightBuffer : register(b0)
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightPosition;
	float padding;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 position3D : TEXCOORD2;
	float time : TIME;
};

float4 main(InputType input) : SV_TARGET
{
	float4	textureColor;
	float4	noiseTex;
	float4	alphaTex;
	float4	color;
	float2	movingTexcoord1;
	float2	movingTexcoord2;
	float2	movingTexcoord3;
	float2	noiseUV;

	movingTexcoord1 = input.tex * 2;
	movingTexcoord1.y += (input.time * 0.1f);

	movingTexcoord2 = input.tex * 4;
	movingTexcoord2.y += (input.time * 0.5f);

	movingTexcoord3 = input.tex * 6;
	movingTexcoord3.y += (input.time * 0.7f);

	float4 noise1 = noiseTexture.Sample(SampleType, movingTexcoord1);
	float4 noise2 = noiseTexture.Sample(SampleType, movingTexcoord2);
	float4 noise3 = noiseTexture.Sample(SampleType, movingTexcoord3);


	float xDistortion = 0.15f;
	float yDistiortion = 0.05f;

	noise1.x *= xDistortion;
	noise1.y *= yDistiortion;

	noise2.x *= xDistortion;
	noise2.y *= yDistiortion;

	noise3.x *= xDistortion;
	noise3.y *= yDistiortion;

	noiseTex = noise1 + noise2 + noise3;


	float distortAmount = 0.5f;
	float distortBias = 0.5f;
	float perturb = ((1.0f - input.tex.y) * distortAmount) + distortBias;
	noiseUV.xy = (noiseTex.xy * perturb) + input.tex.xy;

	alphaTex = alphaTexture.Sample(SampleType, noiseUV.xy);
	textureColor = shaderTexture.Sample(SampleType, noiseUV.xy);


	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	color = textureColor * alphaTex;
	color.a = alphaTex;

	return color;
}

