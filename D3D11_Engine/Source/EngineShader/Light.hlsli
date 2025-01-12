
#ifndef __Light_HLSL__
#define __Light_HLSL__

#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/BRDF.hlsli"


struct DirectionLight
{
	float4 LightColor;
	float3 LightDir;
	float LightIntensity;
	
	Matrix LightViewProj;
};

float3 GetRadiance(DirectionLight light)
{
	return light.LightColor.rgb * light.LightIntensity;
}

float3 GetDirection(DirectionLight light)
{
	return normalize(light.LightDir.xyz);
}

StructuredBuffer<DirectionLight> DirecLights : register(t16);
Texture2DArray ShadowMap : register(t20);


float3 DefaultLit(float3 albedo, float metallic, float roughness, float3 F0, float3 world, float3 N, float3 V, float ambiantOcclusion)
{
	float2 offsets[9] =
	{
		float2(-1, -1), float2(+0, -1), float2(+1, -1),
		float2(-1, +0), float2(+0, +0), float2(+1, +0),
        float2(-1, +1), float2(+0, +1), float2(+1, +1)
	};
	float3 finalColor = 0;
	uint count, width, height, lod;
	ShadowMap.GetDimensions(0, width, height, count, lod);
	
	float texelSize = 1.0f / width; // 텍셀 크기 (Shadow Map 해상도 기준)
	
	uint stride;
	DirecLights.GetDimensions(count, stride);
	
	for (uint i = 0; i < count; i++)
	{
		//라이트맵 되면.....
		float shadowFactor = 0.0f;
#if 0		
		DirectionLight light = DirecLights[i];
		float4 shadowNDC = mul(float4(world, 1.0f), light.LightViewProj);
		float2 uv;
		shadowNDC = shadowNDC / shadowNDC.w;
		float currentDepth = shadowNDC.z;
			
		uv.y = -uv.y;
		uv = uv * 0.5f + 0.5f;
		
		for (int j = 0; j < 9; j++)
		{
			float3 sampleUV = float3(uv + offsets[j] * texelSize, currentDepth);
			float shadowDepth = ShadowMap.Sample(BorderSampler, sampleUV).r;
			shadowFactor += 1 - saturate((currentDepth - shadowDepth) * 200.0f);
		}
		shadowFactor /= 9.0f;
		
#endif

		shadowFactor = 1;
		finalColor += BRDF(albedo, metallic, roughness, F0, N, V, GetDirection(DirecLights[i])) * GetRadiance(DirecLights[i]) * shadowFactor;
	}

	finalColor += BRDF_IBL(albedo, metallic, roughness, F0, N, V) * (1 - ambiantOcclusion);
	return finalColor;
}


#endif // __Light_HLSL__

