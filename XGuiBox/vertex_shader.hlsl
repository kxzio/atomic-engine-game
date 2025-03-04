// Vertex Shader
cbuffer MatrixBuffer : register(b0)
{
    matrix projection;
    matrix view;
    matrix model;
};

struct VS_INPUT
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(input.position, model);
    output.position = mul(output.position, view);
    output.position = mul(output.position, projection);
    output.texCoord = input.texCoord;
    return output;
}
