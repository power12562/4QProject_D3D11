#include "../EngineShader/Shared.hlsli"
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
    
    // ���� Ư��
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
          
    float3 F0 = lerp(Fdielectric, albedo, metalness); //F0 ���ϱ�
    F0 *= specular;
    
    float3 V = normalize(MainCamPos - input.World); // �� ����  
    float NoV = max(0.0, dot(N, V)); // N��V ���
       
    //3x3 PCF ���ø�    
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
        //�׸��� Ȯ��
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
        
            ////���Ͼ� �׸��� ���ø�
            //float sampleShadowDepth = ShadowMapTextures[i].Sample(defaultSampler, uv).r;
            //if (currentShadowDepth > sampleShadowDepth + 0.001)
            //{
            //    shadowFactor[i] = 0;
            //} 
        }
    }
    
    //���� ����
    float3 finalColor = 0;
    
    //directLighting
    for (i = 0; i < LightsCount; i++)
    {
        float3 L = normalize(-Lights[i].LightDir.xyz); // ���� ����
        float3 H = normalize(L + V); // Half Vector
        float NoL = max(0.0, dot(N, L)); // N��L ���
        float HoV = max(0.0, dot(L, H)); // H��V ���
        float NoH = max(0.0, dot(N, H)); // N��H ���
            
        // ������ �ݻ�
        float3 F = FresnelReflection(F0, HoV);

        // ���� ���� �Լ�
        float D = NormalDistribution(roughness, NoH);

        // ��⼺ ����
        float G = GeometricAttenuation(NoV, NoL, roughness);

        // Specular �ݻ�
        float3 specular = SpecularBRDF(D, F, G, NoL, NoV);

        // Diffuse �ݻ�
        float3 diffuse = DiffuseBRDF(albedo, F, NoL, metalness);

        // ���� ���
        float3 radiance = Lights[i].LightColor.rgb * Lights[i].LightIntensity;
        float3 directLighting = (diffuse + specular) * radiance * NoL;
        finalColor += shadowFactor[i] * directLighting;
    }
    
    // ǥ���� �޴� �ݱ��� ���� ���⿡�� ���� ������ ���ø��Ѵ�. Lambertian BRDF�� �����Ͽ� ���ԵǾ� �ִ�.
    float3 irradiance = Diffuse_IBL_Texture.Sample(defaultSampler, N).rgb;
 
    // �� ������ Ư�� �� �� �����Ƿ� cosLo = dot(Normal,View)�� ����Ѵ�.
    float3 F = FresnelReflection(F0, NoV);
    
    // �ݼ��ϼ��� ǥ�� ����� �����ϸ� ��ݼ��ϼ��� ǥ�� ����� �״�� ǥ���ȴ�. 
    float3 kd = lerp(1.0 - F, 0.0, metalness);
    
    //IBL Diffuse
    float3 diffuseIBL = kd * albedo * irradiance;

    //Specular BRDF LookUpTable �̿��� ���Ѵ�.
    float2 specularBRDF = BRDF_LUT.Sample(BRDF_LUTSampler, float2(NoV, roughness)).rg;

    // ���� �ؽ�ó���� LOD Mipmap ���� ���� ��´�.
    uint SpecularTextureLevels, width, height;
    Specular_IBL_Texture.GetDimensions(0, width, height, SpecularTextureLevels);
 
    // Lr( View,Normal�� �ݻ纤��) �� ��ĥ�⸦ ����Ͽ� �ݻ� ���� ���ø��Ѵ�. 
    // ��ĥ�⿡ ���� ������ �ݻ� ���� ǥ���ϱ�����  LOD ���������� ����ȴ�. 
    float3 Lr = reflect(-V, N);
    float3 PrefilteredColor = Specular_IBL_Texture.SampleLevel(defaultSampler, Lr, roughness * SpecularTextureLevels).rgb;
    
    // IBL Specular 
    float3 specularIBL = SpecularIBL(F0, specularBRDF, PrefilteredColor);

    //���� ���
    float3 ambientLighting = (diffuseIBL + specularIBL) * ambientOcculusion;  
    finalColor += ambientLighting;
    finalColor = LinearToGammaSpaceExact(finalColor);
    finalColor += emissiveSample.rgb;
    return float4(finalColor, opacitySample);
}