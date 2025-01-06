// FishEyeShader.fx

sampler2D g_texture : register(s0);
float2 screenCenter; // Центр экрана
float strength = 0.3f; // Сила эффекта рыбьего глаза

float4 PS_FishEye(float2 texCoord : TEXCOORD) : COLOR
{
    // Вычисляем смещение координат от центра экрана
    float2 offset = texCoord - screenCenter;
    float distance = length(offset);
    
    // Применяем искажение (эффект рыбьего глаза)
    float factor = 1.0f + strength * distance * distance;
    float2 newCoord = screenCenter + offset / factor;

    // Делаем выборку из текстуры
    return tex2D(g_texture, newCoord);
}

technique FishEyeTechnique
{
    pass P0
    {
        PixelShader = compile ps_2_0 PS_FishEye();
    }
}
