// FishEyeShader.fx

sampler2D g_texture : register(s0);
float2 screenCenter; // ����� ������
float strength = 0.3f; // ���� ������� ������� �����

float4 PS_FishEye(float2 texCoord : TEXCOORD) : COLOR
{
    // ��������� �������� ��������� �� ������ ������
    float2 offset = texCoord - screenCenter;
    float distance = length(offset);
    
    // ��������� ��������� (������ ������� �����)
    float factor = 1.0f + strength * distance * distance;
    float2 newCoord = screenCenter + offset / factor;

    // ������ ������� �� ��������
    return tex2D(g_texture, newCoord);
}

technique FishEyeTechnique
{
    pass P0
    {
        PixelShader = compile ps_2_0 PS_FishEye();
    }
}
