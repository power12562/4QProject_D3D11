#include "../EngineShader/Shared.hlsli"
SamplerState defaultSampler : register(s0);

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D specularTexture : register(t2);
Texture2D emissiveTexture : register(t3);
Texture2D opacityTexture : register(t4);
Texture2D metalnessTexture : register(t5);
Texture2D roughnessTexture : register(t6);
Texture2D ambientOcculusionTexture : register(t7);

cbuffer cb_PBRMaterial : register(b4)
{
    float4 baseColor;
    float Metalness;
    float Roughness;
    
    bool UseMetalnessMap;
    bool UseRoughnessMap;
    bool UseRMACMap;
};

struct GBuffer 
{
    float4 Albedo   : SV_Target0;   //Albedo.rgb
    float4 Specular : SV_Target1;   //Metallic.r + Specular.g + Roughness.b + AO.a
    float4 Normal   : SV_Target2;   //Normal.rgb
    float4 Emissive : SV_Target3;   //Emissive.rgb
};

[earlydepthstencil]
GBuffer main(PS_INPUT input)
{
    float3 albedoSample = GammaToLinearSpace(albedoTexture.Sample(defaultSampler, input.Tex).rgb);
    float3 normalSample;
    normalSample.rg = normalTexture.Sample(defaultSampler, input.Tex).rg;
    normalSample.b = sqrt(1.0f - (normalSample.r * normalSample.r + normalSample.g * normalSample.g));
    float4 emissiveSample = emissiveTexture.Sample(defaultSampler, input.Tex);
    float metalnessSample = metalnessTexture.Sample(defaultSampler, input.Tex).r;
    float4 specularSample = specularTexture.Sample(defaultSampler, input.Tex).r;
    float roughnessSample = roughnessTexture.Sample(defaultSampler, input.Tex).r;
    float ambientSample = ambientOcculusionTexture.Sample(defaultSampler, input.Tex).r;
    
    input.Normal = normalize(input.Normal);
    float3 N;
    if (Epsilon < length(normalSample))
    {
        float3x3 TBN = float3x3(input.Tangent, input.BiTangent, input.Normal);
        N = normalize(mul(normalSample, TBN));
    }
    else
        N = input.Normal;
        
    // 재질 특성
    float specular = specularSample.r;
    float metalness = Metalness;
    float roughness = Roughness;
    float ambientOcculusion = ambientSample;
    
    if (UseMetalnessMap)
        metalness = metalnessSample;
    else if (UseRMACMap)
        metalness = specularSample.g;
    
    if (UseRoughnessMap)
        roughness = roughnessSample;
    else if (UseRMACMap)
        roughness = specularSample.b;
          
    if (UseRMACMap)
        ambientOcculusion = specularSample.a;
      
    float3 albedo = albedoSample.rgb * baseColor.rgb;
    
    GBuffer final;    
    final.Albedo.rgb = albedo.rgb;
    final.Albedo.a = 1;
    
    //Specular.r + Metallic.g +  + Roughness.b + AO.a
    final.Specular.r = specular;
    final.Specular.g = metalness;
    final.Specular.b = roughness;
    final.Specular.a = ambientOcculusion;
    
    final.Normal.rgb = N;
    final.Normal.a = 1;
    
    final.Emissive.rgb = emissiveSample.rgb;
    final.Emissive.a = 1;

    return final;
}