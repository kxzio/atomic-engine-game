// Pixel Shader
Texture2D texture0 : register(t0); // ������� ��������
SamplerState sampler0 : register(s0); // �������

// ��������� ������� VHS
cbuffer VHS_Params : register(b0)
{
    float time; // ����� ��� �������� (��� �������� �����)
    float noiseStrength; // ���� ����
    float scanLineFrequency; // ������� ����������� �����
};

float4 main(float2 texCoord : TEXCOORD) : SV_Target
{
    // ���������� ����
    float noise = (frac(sin(dot(texCoord.xy, float2(12.9898, 78.233))) * 43758.5453));
    noise = (noise - 0.5) * noiseStrength;

    // �������� ��������� (��� VHS)
    float2 offset = float2(sin(texCoord.y * 30.0 + time) * 0.01, cos(texCoord.y * 20.0 + time) * 0.01);

    // �������� ���� �� �������� � ������ ���������
    float4 color = texture0.Sample(sampler0, texCoord + offset + noise);

    // ���������� ����������� �����
    if (fmod(texCoord.y * 100.0, 1.0) < 0.05)
    {
        color.rgb *= 0.5; // ���������� �����
    }

    // ���������� ��������� ����
    return color;
}
