// Light vertex shader
// Standard issue vertex shader, apply matrices, pass info to pixel shader

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    float time;
    float2 tile;
    float3 lightPosition;
};

cbuffer LightPosBuffer  : register(b1)
{
    float3 lightPosition1;
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 lightPos : TEXCOORD1;
    float3 normal : NORMAL;
    float3 position3D : TEXCOORD2;
    float time : TIME;
    float waveCol : WAVECOLOR;
    float2 tile : TILE;
};

OutputType main(InputType input)
{
    OutputType output;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);

    // Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

    // Calculate the normal vector against the world matrix only.
    output.normal = mul(input.normal, (float3x3)worldMatrix);

    float speed = 0.4f;
    float freq = 1;
    float amp = 0.3f;
    float t = time * speed;
    float waveLength = sin(t + output.position.x * freq) * amp + cos(t * 2 + output.position.z * freq * 2) * amp;
    output.position.y += waveLength;
    output.normal = normalize(float3(output.normal.x + waveLength, output.normal.y, output.normal.z + waveLength));
    output.waveCol = waveLength;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Normalize the normal vector.
    output.normal = normalize(output.normal);

    //Calculalte light position 
    float4 worldPos = mul(input.position, worldMatrix);
    output.lightPos.xyz = lightPosition.xyz - worldPos.xyz;
    output.lightPos = normalize(output.lightPos);

    //output.color = input.color;
    output.time = time;
    output.tile = tile;
    // world position of vertex (for point light)
    output.position3D = (float3)mul(input.position, worldMatrix);

    return output;
}