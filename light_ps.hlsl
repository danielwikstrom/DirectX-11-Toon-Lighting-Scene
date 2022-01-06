// Light pixel shader
// Calculate diffuse lighting for a single directional light(also texturing)

Texture2D shaderTexture : register(t0);
Texture2D normalTexture : register(t1);
SamplerState SampleType : register(s0);


cbuffer LightBuffer : register(b0)
{
	float4 ambientColor;
    float4 diffuseColor;
    float3 lightPosition;
	float time;
    float padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
	float3 position3D : TEXCOORD2;
	float3 tangent: TANGENT;
	float3 binormal: BINORMAL;
	float2 tile : TILE;
};

float4 main(InputType input) : SV_TARGET
{
	float4	textureColor;
	float4	normalMap;
	float3	normalDir;
    float3	lightDir;
    float	lightIntensity;
    float4	color;
	input.tex *= input.tile;

	normalMap = normalTexture.Sample(SampleType, input.tex);
	normalMap = (normalMap * 2) - 1;
	normalDir = (normalMap.x * input.tangent) + (normalMap.y * input.binormal) + (normalMap.z * input.normal);
	normalDir = normalize(normalDir);
	//normalDir = normalDir > 0 ? normalDir : input.normal;
	// Invert the light direction for calculations.
	lightDir = normalize(input.position3D - lightPosition);

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(normalDir, -lightDir));

	// Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
	color = ambientColor + (diffuseColor * lightIntensity); //adding ambient
	color = saturate(color);

	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	color = color * textureColor;

    return color;
}

