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

cbuffer PointLightBuffer : register(b1)
{
	float4 pointLightColor;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 lightPos : TEXCOORD1;
    float3 normal : NORMAL;
	float3 position3D : TEXCOORD2;
	float2 tile : TILE;
};

float4 main(InputType input) : SV_TARGET
{
	float4	textureColor;
	float4	normalMap;
	float3	normalDir;
    float3	lightDir;
    float	lightIntensityDir;
	float	lightIntensityPoint;
	float	lightIntensity;
	float4	pointColor;
	float4	dirColor;
    float4	color;
	input.tex *= input.tile;

	//normalDir = normalDir > 0 ? normalDir : input.normal;
	// Invert the light direction for calculations.
	lightDir = normalize(input.position3D - lightPosition);

	// Calculate the amount of light that the point light contributes.
	lightIntensityDir = saturate(dot(input.normal, -lightDir));

	//Make light intensity have 5 possible values to get a cartoon effect
	lightIntensityDir = ceil(lightIntensityDir * 5) / 5;

	dirColor = diffuseColor * lightIntensityDir;


	// Calculate the amount of light that the directional light contributes.
	lightIntensityPoint = saturate(dot(input.normal, input.lightPos));

	//Make light intensity have 5 possible values to get a cartoon effect
	lightIntensityPoint = ceil(lightIntensityPoint * 5) / 5;
	pointColor = pointLightColor * lightIntensityPoint;


	//Calculate the total amount of light boith lights contribute
	color = saturate(pointColor + dirColor);

	//Make light intensity have 5 possible values to get a cartoon effect

	// Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
	color = ambientColor + color; //adding ambient
	color = saturate(color);

	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	color = color * textureColor;

    return color;
}

