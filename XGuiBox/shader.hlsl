// ��������� ������
struct VS_INPUT
{
    float4 position : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

cbuffer ViewProjectionBuffer : register(b0)
{
    matrix viewProjection;
};

PS_INPUT VS_Main(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(input.position, viewProjection); // �������������� � ������� ������������
    output.texcoord = input.texcoord;
    return output;
}

// ���������� ������ ��� ������� Bloom
Texture2D g_Texture : register(t0); // �������� �������� �����
SamplerState g_Sampler : register(s0); // ������� ��� ��������

// ��� ���������� ����� ��������
float BloomThreshold = 0.5f; // ����� ��� �������
float BloomIntensity = 500.5f; // ������������� Bloom �������

float4 PS_Main(PS_INPUT input) : SV_Target
{
    // �������� ���� �� ��������
    float4 color = g_Texture.Sample(g_Sampler, input.texcoord);

    // ��������� ����� ��� ��������� ����� ��������
    if (color.r < BloomThreshold && color.g < BloomThreshold && color.b < BloomThreshold)
    {
        color.rgb = float3(0.0f, 0.0f, 0.0f); // ��������� �������� �������
    }
    else
    {
        color.rgb *= BloomIntensity; // ��������� �������
    }

    return color;
}

// ������� ������ �������� ��� ������� Bloom
Texture2D g_BloomTexture : register(t1); // �������� � ������ ��������� ��� Bloom
SamplerState g_SamplerBloom : register(s1); // ������� ��� Bloom ��������

// �������� �����������
float4 PS_Bloom(PS_INPUT input) : SV_Target
{
    float4 bloomColor = g_BloomTexture.Sample(g_SamplerBloom, input.texcoord);

    // ��������� ������� ��������
    bloomColor.rgb = bloomColor.rgb * 0.5f; // ����������� ��� ������� ��������

    return bloomColor;
}

// �������� ������, ������� ��������� Bloom � ��������� �����������
technique BloomEffect
{
    pass P0
    {
        // ��������� ������
        SetVertexShader(CompileShader(vs_4_0, VS_Main()));

        // ���������� ������ ��� Bloom �������
        SetPixelShader(CompileShader(ps_4_0, PS_Main()));
    }

    pass P1
    {
        // ��������� ������
        SetVertexShader(CompileShader(vs_4_0, VS_Main()));

        // ���������� ������ ��� ��������
        SetPixelShader(CompileShader(ps_4_0, PS_Bloom()));
    }
}
