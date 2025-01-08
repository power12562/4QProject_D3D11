static const float PI = 3.141592654;
static const float3 Fdielectric = 0.04f;
static const float Epsilon = 1e-6;
static const float MinRoughness = 0.04f;
#define SHADOW_MAP_SIZE 8192

inline float GammaToLinearSpaceExact(float value)
{
    if (value <= 0.04045f)
        return value / 12.92f;
    else if (value < 1.0f)
        return pow((value + 0.055f) / 1.055f, 2.4f);
    else
        return pow(value, 2.2f);
}

inline float3 GammaToLinearSpaceExact(float3 value)
{
    return float3(
    GammaToLinearSpaceExact(value.r),
    GammaToLinearSpaceExact(value.g),
    GammaToLinearSpaceExact(value.b));
}

inline float3 GammaToLinearSpace(float3 rgb)
{
    return rgb * (rgb * (rgb * 0.305306011f + 0.682171111f) + 0.012522878f);

    //Precise version, useful for debugging.
    //return float3(GammaToLinearSpaceExact(rgb.r),
    //GammaToLinearSpaceExact(rgb.g),
    //GammaToLinearSpaceExact(rgb.b));
}

inline float LinearToGammaSpaceExact(float value)
{
    if (value <= 0.0f)
        return 0.0f;
    else if (value <= 0.0031308f)
        return 12.92f * value;
    else if (value < 1.0f)
        return 1.055f * pow(value, 0.4166667f) - 0.055f;
    else
        return pow(value, 0.45454545f);
}

inline float3 LinearToGammaSpaceExact(float3 value)
{
    return float3(
    LinearToGammaSpaceExact(value.r),
    LinearToGammaSpaceExact(value.g),
    LinearToGammaSpaceExact(value.b));
}

inline float3 LinearToGammaSpace(float3 rgb)
{
    rgb = max(rgb, float3(0.f, 0.f, 0.f));
    return max(1.055f * pow(rgb, 0.416666667f) - 0.055f, 0.f);

    //Precise version, useful for debugging.
    //return float3(LinearToGammaSpaceExact(rgb.r),
    //LinearToGammaSpaceExact(rgb.g),
    //LinearToGammaSpaceExact(rgb.b));
}

inline float NormalDistribution(float roughness, float NoH)
{
    roughness = max(roughness, MinRoughness);
    float a = roughness * roughness;
    float squareA = a * a;
    float var = NoH * NoH * (squareA - 1.0) + 1.0;
    return squareA / (PI * var * var);
}

inline float3 FresnelReflection(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

inline float GSchlickGGX(float NoX, float k)
{
    return NoX / (NoX * (1.0 - k) + k);
}

inline float GeometricAttenuation(float NoV, float NoL, float roughness)
{
    roughness = max(roughness, MinRoughness);
    float a = roughness + 1.0;
    float k = (a * a) / 8.0; // direct
    //float k = (roughness * roughness) * 0.5; // IBL
    
    return GSchlickGGX(NoV, k) * GSchlickGGX(NoL, k);
}

inline float3 SpecularBRDF(float D, float3 F, float G, float NoL, float NoV)
{
    return D * F * G / max(4.0f * NoL * NoV, 0.001f);
}

inline float3 SpecularIBL(float3 F0, float2 specularBRDF, float3 PrefilteredColor)
{
    return (F0 * specularBRDF.x + specularBRDF.y) * PrefilteredColor;
}

inline float3 DiffuseBRDF(float3 BaseColor, float3 F, float NoL, float Metalness)
{
    float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), Metalness);
    return float3(BaseColor * kd * NoL / PI);
}

cbuffer cbuffer_Transform : register(b0)
{
    Matrix World;
    Matrix WorldInverseTranspose;
    Matrix WVP;
}

cbuffer cb_Camera : register(b1)
{
    float3 MainCamPos;
    Matrix View;
    Matrix Projection;
    Matrix IVM;
    Matrix IPM;
};

#define MAX_LIGHT_COUNT 4
cbuffer cb_ShadowMap : register(b2)
{
    Matrix ShadowProjections[MAX_LIGHT_COUNT];
    Matrix ShadowViews[MAX_LIGHT_COUNT];
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 World : POSITION;
    float3 Normal : NORMAL0;
    float3 Tangent : NORMAL1;
    float3 BiTangent : NORMAL2;
    float2 Tex : TEXCOORD0;
    float4 PositionShadows[MAX_LIGHT_COUNT] : TEXCOORD1;
};

