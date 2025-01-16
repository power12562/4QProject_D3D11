#ifndef __BRDF_HLSL__
#define __BRDF_HLSL__

#include "../EngineShader/Shared.hlsli"

Texture2D BRDF_LUT : register(t30);
TextureCube Diffuse_IBL : register(t31);
TextureCube Specular_IBL : register(t32);


float3 BRDF(float3 albedo, float metalic, float roughness, float3 F0, float3 N, float3 V, float3 L)
{
	//float3 L = normalize(-Lights[i].LightDir.xyz); // ���� ����
	float3 H = normalize(-L + V); // Half Vector
	float NoL = max(0.0, dot(N, -L)); // N��L ���
	float HoV = max(0.0, dot(H, V)); // H��V ���
	float NoH = max(0.0, dot(N, H)); // N��H ���
	float NoV = max(0.0, dot(N, V)); // N��V ���
        
	// ������ �ݻ�
	float3 F = FresnelReflection(F0, HoV);

	// ���� ���� �Լ�
	float D = NormalDistribution(max(0.1f, roughness), NoH);

	// ��⼺ ����
	float G = GeometricAttenuation(NoV, NoL, roughness);

	// Specular �ݻ�
	float3 specular = SpecularBRDF(D, F, G, NoL, NoV);

	// Diffuse �ݻ�
	float3 diffuse = DiffuseBRDF(albedo, F, NoL, metalic);

	// ���� ���
	float3 directLighting = (diffuse + specular) * NoL;
	
	return directLighting;
}

float3 BRDF_IBL(float3 albedo, float metalic, float roughness, float3 F0, float3 N, float3 V)
{
	float NoV = max(0.0, dot(N, V)); // N��V ���
	
	float3 irradiance = Diffuse_IBL.SampleLevel(DefaultSampler, N, 0).rgb;
	float3 F = FresnelReflection(F0, NoV);
	float3 kd = lerp(1.0 - F, 0.0, metalic);
	float3 diffuseIBL = kd * albedo * irradiance;
	float2 specularBRDF = BRDF_LUT.SampleLevel(ClampSampler, float2(NoV, roughness), 0).rg;
	
	uint SpecularTextureLevels, width, height;
	Specular_IBL.GetDimensions(0, width, height, SpecularTextureLevels);
	
	float3 Lr = reflect(-V, N);
	float3 PrefilteredColor = Specular_IBL.SampleLevel(DefaultSampler, Lr, roughness * SpecularTextureLevels).rgb;
	
	float3 specularIBL = SpecularIBL(F0, specularBRDF, PrefilteredColor);
	float3 ambientLighting = (diffuseIBL + specularIBL);
	return ambientLighting;
}

#endif // __BRDF_HLSL__