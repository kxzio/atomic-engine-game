// Вершинный шейдер
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
    output.position = mul(input.position, viewProjection); // Преобразование в мировое пространство
    output.texcoord = input.texcoord;
    return output;
}

// Пиксельный шейдер для эффекта Bloom
Texture2D g_Texture : register(t0); // Основная текстура сцены
SamplerState g_Sampler : register(s0); // Самплер для текстуры

// Для фильтрации ярких пикселей
float BloomThreshold = 0.5f; // Порог для яркости
float BloomIntensity = 500.5f; // Интенсивность Bloom эффекта

float4 PS_Main(PS_INPUT input) : SV_Target
{
    // Получаем цвет из текстуры
    float4 color = g_Texture.Sample(g_Sampler, input.texcoord);

    // Применяем порог для выделения ярких участков
    if (color.r < BloomThreshold && color.g < BloomThreshold && color.b < BloomThreshold)
    {
        color.rgb = float3(0.0f, 0.0f, 0.0f); // Отключаем ненужные пиксели
    }
    else
    {
        color.rgb *= BloomIntensity; // Усиливаем яркость
    }

    return color;
}

// Простой шейдер размытия для эффекта Bloom
Texture2D g_BloomTexture : register(t1); // Текстура с яркими участками для Bloom
SamplerState g_SamplerBloom : register(s1); // Самплер для Bloom текстуры

// Размытие изображения
float4 PS_Bloom(PS_INPUT input) : SV_Target
{
    float4 bloomColor = g_BloomTexture.Sample(g_SamplerBloom, input.texcoord);

    // Применяем простое размытие
    bloomColor.rgb = bloomColor.rgb * 0.5f; // Модификация для эффекта свечения

    return bloomColor;
}

// Основной шейдер, который применяет Bloom и размывает изображение
technique BloomEffect
{
    pass P0
    {
        // Вершинный шейдер
        SetVertexShader(CompileShader(vs_4_0, VS_Main()));

        // Пиксельный шейдер для Bloom эффекта
        SetPixelShader(CompileShader(ps_4_0, PS_Main()));
    }

    pass P1
    {
        // Вершинный шейдер
        SetVertexShader(CompileShader(vs_4_0, VS_Main()));

        // Пиксельный шейдер для размытия
        SetPixelShader(CompileShader(ps_4_0, PS_Bloom()));
    }
}
