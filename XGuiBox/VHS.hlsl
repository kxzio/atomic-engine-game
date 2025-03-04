Texture2D inputTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer TimeBuffer : register(b0) {
    float time;         // Текущее время
    float2 resolution;  // Разрешение экрана
};

float random(float2 uv) {
    return frac(sin(dot(uv.xy, float2(12.9898, 78.233))) * 43758.5453);
}

float3 applyNoise(float3 color, float2 uv) {
    float noise = random(uv + time) * 0.1; // Интенсивность шума
    return color + noise;
}

float3 applyScanlines(float3 color, float2 uv) {
    float scanline = 0.5 + 0.5 * sin(uv.y * resolution.y * 10.0 + time * 50.0); // Полосы
    return color * scanline;
}

float3 chromaticAberration(float2 uv, float amount) {
    float2 offset = float2(amount * sin(time * 5.0), 0.0);
    float r = inputTexture.Sample(samplerState, uv + offset).r;
    float g = inputTexture.Sample(samplerState, uv).g;
    float b = inputTexture.Sample(samplerState, uv - offset).b;
    return float3(r, g, b);
}

float4 main(float2 uv : TEXCOORD) : SV_Target {
    float3 color = inputTexture.Sample(samplerState, uv).rgb;

    // Хроматическая аберрация
    color = chromaticAberration(uv, 0.002);

    // Добавляем шум
    color = applyNoise(color, uv);

    // Полосы (scanlines)
    color = applyScanlines(color, uv);

    return float4(color, 1.0);
}
