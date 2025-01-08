#include "Shared.hlsli"
SamplerState defaultSampler : register(s0);
SamplerState BRDF_LUTSampler : register(s1);
SamplerComparisonState ShadowMapSampler : register(s2);

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D specularTexture : register(t2);
Texture2D emissiveTexture : register(t3);
Texture2D opacityTexture : register(t4);
Texture2D metalnessTexture : register(t5);
Texture2D roughnessTexture : register(t6);
Texture2D ambientOcculusionTexture : register(t7);

TextureCube Diffuse_IBL_Texture : register(t8);
TextureCube Specular_IBL_Texture : register(t9);
Texture2D BRDF_LUT : register(t10);

Texture2D ShadowMapTextures[4] : register(t124);

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

cbuffer cb_PBRMaterial : register(b4)
{
    float4 baseColor;
    float Metalness;
    float Roughness;
    
    bool UseMetalnessMap;
    bool UseRoughnessMap;
    bool UseRMACMap;
};

#ifdef OPACITY
#else
[earlydepthstencil]
#endif
float4 main(PS_INPUT input) : SV_Target
{   
    float opacitySample = opacityTexture.Sample(defaultSampler, input.Tex).a;
    
#ifdef OPACITY
    clip( opacitySample - 0.33333333f );
#endif
    
    float4 albedoSample = albedoTexture.Sample(defaultSampler, input.Tex);
    albedoSample.rgb = GammaToLinearSpaceExact(albedoSample.rgb);
    float3 normalSample;
    normalSample.rg = normalTexture.Sample(defaultSampler, input.Tex).rg;
    normalSample.b = sqrt(1.0f - (normalSample.r * normalSample.r + normalSample.g * normalSample.g));
    float4 emissiveSample = emissiveTexture.Sample(defaultSampler, input.Tex);
    float metalnessSample = metalnessTexture.Sample(defaultSampler, input.Tex).r;
    float roughnessSample = roughnessTexture.Sample(defaultSampler, input.Tex).r;
    float ambientSample = ambientOcculusionTexture.Sample(defaultSampler, input.Tex).r;
    float4 specularSample = specularTexture.Sample(defaultSampler, input.Tex); //Specular Map
    
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
    float metalness = Metalness;
    float roughness = Roughness;
    float ambientOcculusion = ambientSample;
    float specular = specularSample.r;
        
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
          
    float3 F0 = lerp(Fdielectric, albedo, metalness); //F0 구하기
    F0 *= specular;
    
    float3 V = normalize(MainCamPos - input.World); // 뷰 방향  
    float NoV = max(0.0, dot(N, V)); // N·V 계산
       
    //3x3 PCF 샘플링    
    float2 offsets[9] =
    {
        float2(-1, -1), float2(0, -1), float2(1, -1),
            float2(-1, 0), float2(0, 0), float2(1, 0),
            float2(-1, 1), float2(0, 1), float2(1, 1)
    };
    float texelSize = 1.0 / SHADOW_MAP_SIZE;
    
    float shadowFactor[MAX_LIGHT_COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f };
    [unroll]
    for (int i = 0; i < LightsCount; i++)
    {
        //그림자 확인
        float currentShadowDepth = input.PositionShadows[i].z;
        float2 uv = input.PositionShadows[i].xy;
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
    float3 irradiance = Diffuse_IBL_Texture.Sample(defaultSampler, N).rgb;
 
    // 빛 방향은 특정 할 수 없으므로 cosLo = dot(Normal,View)을 사용한다.
    float3 F = FresnelReflection(F0, NoV);
    
    // 금속일수록 표면 산란을 제거하며 비금속일수록 표면 산란이 그대로 표현된다. 
    float3 kd = lerp(1.0 - F, 0.0, metalness);
    
    //IBL Diffuse
    float3 diffuseIBL = kd * albedo * irradiance;

    //Specular BRDF LookUpTable 이용해 구한다.
    float2 specularBRDF = BRDF_LUT.Sample(BRDF_LUTSampler, float2(NoV, roughness)).rg;

    // 원본 텍스처에서 LOD Mipmap 레벨 수를 얻는다.
    uint SpecularTextureLevels, width, height;
    Specular_IBL_Texture.GetDimensions(0, width, height, SpecularTextureLevels);
 
    // Lr( View,Normal의 반사벡터) 와 거칠기를 사용하여 반사 빛을 샘플링한다. 
    // 거칠기에 따라 뭉게진 반사 빛을 표현하기위해  LOD 선형보간이 적용된다. 
    float3 Lr = reflect(-V, N);
    float3 PrefilteredColor = Specular_IBL_Texture.SampleLevel(defaultSampler, Lr, roughness * SpecularTextureLevels).rgb;
    
    // IBL Specular 
    float3 specularIBL = SpecularIBL(F0, specularBRDF, PrefilteredColor);

    //조명 계산
    float3 ambientLighting = (diffuseIBL + specularIBL) * ambientOcculusion;  
    finalColor += ambientLighting;
    finalColor = LinearToGammaSpaceExact(finalColor);
    finalColor += emissiveSample.rgb;
    return float4(finalColor, opacitySample);
}