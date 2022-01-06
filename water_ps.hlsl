// Light pixel shader
// Calculate diffuse lighting for a single directional light(also texturing)

Texture2D shaderTexture : register(t0);
Texture2D patternTexture : register(t1);
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
	float2 tile : TILE;
	float waveCol : WAVECOLOR;
};

float4 main(InputType input) : SV_TARGET
{
	float4	textureColor;
	float4	patternTex;
	float3	lightDir;
	float	lightIntensity;
	float4	color;
	float2 movingTexcoord;

	movingTexcoord = input.tex;
	movingTexcoord.xy *= input.tile;
	movingTexcoord.y += input.time * 0.035f;
	// Invert the light direction for calculations.
	lightDir = normalize(input.position3D - lightPosition);

	// Calculate the amount of light on this pixel.
	//TODO: Apply light intensity in different "tiers" to simulate toon shader
	lightIntensity = saturate(dot(input.normal, -lightDir));

	// Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
	color = ambientColor + (diffuseColor * lightIntensity); //adding ambient
	color = saturate(color);

	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = shaderTexture.Sample(SampleType, movingTexcoord);
	patternTex = patternTexture.Sample(SampleType, input.tex);
	color = (color * textureColor /** (patternTex)*/ + (input.waveCol/10));
	color.a = (0.7f);

	return color;
}

