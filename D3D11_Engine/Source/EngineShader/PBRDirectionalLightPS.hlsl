#include "../EngineShader/Shared.hlsli"
SamplerState linearSampler : register(s0);
SamplerComparisonState ShadowMapSampler : register(s1);

Texture2D AlbedoTexture : register(t0);     //Albedo rgb
Texture2D SpecularTexture : register(t1);   //Specular.r + Metallic.g + Roughness.b + AO.a
Texture2D NormalTexture : register(t2);     //Normal.rgb
Texture2D EmissiveTexture : register(t3);   //Emissive.rgb
Texture2D DepthTexture : register(t4);      //Depth.r

TextureCube Diffuse_IBL_Texture : register(t8);
TextureCube Specular_IBL_Texture : register(t9);
Texture2D BRDF_LUT : register(t10);

Texture2D ShadowMapTextures[4] : register(t124);

cbuffer cbuffer_Transform : register(b0)
{
    float scaleX;
    float scaleY;
    float offsetX;
    float offsetY;
}

cbuffer cb_Light : register(b3)
{
    struct
    {
        float4 LightColor;
        float3 LightDir;
        float LightIntensity;
    }
    Lights[MAX_LIGHT_COUNT];
    int LightsCount;
}

struct PS_Deferred
{
    float4 svposition : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

inline float3 ReconstructWorldPosition(float2 uv, float depth)
{
    // Step 1: Reconstruct NDC
    float2 NDC = float2(uv.x * 2.0f - 1.0f, (1.0f - uv.y) * 2.0f - 1.0f);
    float4 clipSpacePos = float4(NDC, depth, 1.0f);

    // Step 2: Convert to View Space
    float4 viewSpacePos = mul(clipSpacePos, IPM);
    viewSpacePos = viewSpacePos / viewSpacePos.w;

    // Step 3: Convert to World Space
    float4 worldPos = mul(viewSpacePos, IVM);

    return worldPos.xyz;
}

inline float4 ComputePositionShadow(float3 world, matrix shadowView, matrix shadowProjection)
{
    float4 PositionShadows;
    PositionShadows = mul(float4(world, 1.0f), shadowView);
    PositionShadows = mul(PositionShadows, shadowProjection);
    return PositionShadows;
}

float4 main(PS_Deferred input) : SV_Target
{   
    float3 N = NormalTexture.Sample(linearSampler, input.Tex).rgb;
    N = normalize(N);
   
    float3 albedo = AlbedoTexture.Sample(linearSampler, input.Tex).rgb;
    float3 emissive = EmissiveTexture.Sample(linearSampler, input.Tex).rgb;
    float4 SMRAO = SpecularTexture.Sample(linearSampler, input.Tex);
    
    float specular =  SMRAO.r;
    float metalness = SMRAO.g;
    float roughness = SMRAO.b;
    float ambientOcculusion = SMRAO.a;
        
    float Depth = DepthTexture.Sample(linearSampler, input.Tex).r;
    clip(1.f - Epsilon - Depth);
    
    float2 WorldUV;
    WorldUV.x = (input.Tex.x + offsetX) * scaleX;
    WorldUV.y = (input.Tex.y + offsetY) * scaleY;
    float3 World = ReconstructWorldPosition(WorldUV, Depth);
    
    float4 PositionShadows[MAX_LIGHT_COUNT];
    [unroll]
    for (int i = 0; i < MAX_LIGHT_COUNT; i++)
    {
        PositionShadows[i] = ComputePositionShadow(World, ShadowViews[i], ShadowProjections[i]);
    }
    
    float3 F0 = lerp(Fdielectric, albedo, metalness); //F0 구하기
    F0 *= specular;
    
    float3 V = normalize(MainCamPos - World); // 뷰 방향  
    float NoV = max(0.0, dot(N, V)); // N·V 계산
       
    //3x3 PCF 샘플링    
    float2 offsets[9] =
    {
        float2(-1, -1), float2(0, -1), float2(1, -1),
        float2(-1, 0),  float2(0, 0),  float2(1, 0),
        float2(-1, 1),  float2(0, 1),  float2(1, 1)
    };
    float texelSize = 1.0 / SHADOW_MAP_SIZE;
    
    float shadowFactor[MAX_LIGHT_COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f };
    [unroll]
    for (i = 0; i < LightsCount; i++)
    {
        //그림자 확인
        float currentShadowDepth = PositionShadows[i].z;
        float2 uv = PositionShadows[i].xy;
        uv.y = -uv.y;
        uv = uv * 0.5f + 0.5f;
        
        if (uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0)
        {       
            [unroll]
            for (int j = 0; j < 9; j++)
            {
                float2 sampleUV = uv + offsets[j] * texelSize;
                shadowFactor[i] += ShadowMapTextures[i].SampleCmpLevelZero(ShadowMapSampler, sampleUV, currentShadowDepth - 0.001);
            }
            shadowFactor[i] = shadowFactor[i] / 9.0f;
        
            ////리니어 그림자 샘플링
            //float sampleShadowDepth = ShadowMapTextures[i].Sample(defaultSampler, uv).r;
            //if (currentShadowDepth > sampleShadowDepth + 0.001)
            //{
            //    shadowFactor[i] = 0;
            //} 
        }
    }
    
    //최종 계산용
    float3 finalColor = 0;
    
    //directLighting
    for (i = 0; i < LightsCount; i++)
    {
        float3 L = normalize(-Lights[i].LightDir.xyz); // 광원 방향
        float3 H = normalize(L + V); // Half Vector
        float NoL = max(0.0, dot(N, L)); // N·L 계산
        float HoV = max(0.0, dot(L, H)); // H·V 계산
        float NoH = max(0.0, dot(N, H)); // N·H 계산
            
        // 프레널 반사
        float3 F = FresnelReflection(F0, HoV);

        // 법선 분포 함수
        float D = NormalDistribution(roughness, NoH);

        // 폐쇄성 감쇠
        float G = GeometricAttenuation(NoV, NoL, roughness);

        // Specular 반사
        float3 specular = SpecularBRDF(D, F, G, NoL, NoV);

        // Diffuse 반사
        float3 diffuse = DiffuseBRDF(albedo, F, NoL, metalness);

        // 조명 계산
        float3 radiance = Lights[i].LightColor.rgb * Lights[i].LightIntensity;
        float3 directLighting = (diffuse + specular) * radiance * NoL;
        finalColor += shadowFactor[i] * directLighting;
    }
    
    // 표면이 받는 반구의 여러 방향에서 오는 광량을 샘플링한다. Lambertian BRDF를 가정하여 포함되어 있다.
    float3 irradiance = Diffuse_IBL_Texture.Sample(linearSampler, N).rgb;
 
    // 빛 방향은 특정 할 수 없으므로 cosLo = dot(Normal,View)을 사용한다.
    float3 F = FresnelReflection(F0, NoV);
    
    // 금속일수록 표면 산란을 제거하며 비금속일수록 표면 산란이 그대로 표현된다. 
    float3 kd = lerp(1.0 - F, 0.0, metalness);
    
    //IBL Diffuse
    float3 diffuseIBL = kd * albedo * irradiance;

    //Specular BRDF LookUpTable 이용해 구한다.
    float2 specularBRDF = BRDF_LUT.Sample(linearSampler, float2(NoV, roughness)).rg;

    // 원본 텍스처에서 LOD Mipmap 레벨 수를 얻는다.
    uint SpecularTextureLevels, width, height;
    Specular_IBL_Texture.GetDimensions(0, width, height, SpecularTextureLevels);
 
    // Lr( View,Normal의 반사벡터) 와 거칠기를 사용하여 반사 빛을 샘플링한다. 
    // 거칠기에 따라 뭉게진 반사 빛을 표현하기위해  LOD 선형보간이 적용된다. 
    float3 Lr = reflect(-V, N);
    float3 PrefilteredColor = Specular_IBL_Texture.SampleLevel(linearSampler, Lr, roughness * SpecularTextureLevels).rgb;
    
    // IBL Specular 
    float3 specularIBL = SpecularIBL(F0, specularBRDF, PrefilteredColor);

    //조명 계산
    float3 ambientLighting = (diffuseIBL + specularIBL) * ambientOcculusion;  
    finalColor += ambientLighting;
    
    finalColor = LinearToGammaSpaceExact(finalColor);
    finalColor += emissive;
    return float4(finalColor, 1);
}