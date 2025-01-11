Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

float2 g_NoiseScale = float2(0.5, 0.5); // Масштаб шума
float g_ScanlineIntensity = 0.8f; // Интенсивность сканирования
float g_Time; // Время для динамических эффектов

// Шум для VHS
float GetNoise(float2 uv)
{
    return frac(sin(dot(uv * 10.0f, float2(12.9898, 78.233))) * 43758.5453);
}

float4 main(float2 uv : TEXCOORD) : SV_Target
{
    float4 color = g_Texture.Sample(g_Sampler, uv);

    // Добавляем шум
    float noise = GetNoise(uv * g_NoiseScale);
    color.rgb = lerp(color.rgb, color.rgb + noise * 0.1f, 0.5f); // Линейная интерполяция для шума

    // Добавляем сканирование (горизонтальные полосы)
    float scanline = sin(uv.y * 100.0f) * g_ScanlineIntensity; // Увеличиваем частоту полос
    color.rgb *= 1.0f - scanline;

    return color;
}
