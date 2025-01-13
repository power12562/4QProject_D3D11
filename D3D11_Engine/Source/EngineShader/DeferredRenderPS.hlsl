
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/Light.hlsli"

Texture2D T_Albedo : register(t0); //Albedo rgb
Texture2D T_Specular : register(t1); //Specular.r + Metallic.g + Roughness.b + AO.a
Texture2D T_Normal : register(t2); //Normal.rgb
Texture2D T_Emissive : register(t3); //Emissive.rgb
Texture2D T_Depth : register(t4); //Depth.r

float3 CalculateWorldPosition(float2 uv, float depth, float4x4 invProjectionMatrix, float4x4 cameraWorldMatrix)
{
    // NDC 좌표로 변환 (-1 ~ 1 범위)
	float2 ndc = float2(uv.x * 2.0 - 1.0, (1.0 - uv.y) * 2.0 - 1.0);

    // 뷰 공간에서의 위치 (투영 변환의 반대 방향)
	float4 clipSpacePos = float4(ndc, depth, 1.0);
	float4 viewSpacePos = mul(clipSpacePos, invProjectionMatrix);
	viewSpacePos = viewSpacePos / viewSpacePos.w; // 뷰 공간으로 변환

    // 월드 공간으로 변환
	float4 worldSpacePos = mul(viewSpacePos, cameraWorldMatrix);

	return worldSpacePos.xyz;
}


struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

[earlydepthstencil]
float4 main(VS_OUTPUT input) : SV_TARGET
{
	float depth = T_Depth.SampleLevel(ClampSampler, input.texcoord, 0).r;
	clip(1.0 - depth - Epsilon	);
	
	float3 albedo = T_Albedo.SampleLevel(ClampSampler, input.texcoord, 0).rgb;
	float3 normal = T_Normal.SampleLevel(ClampSampler, input.texcoord, 0).rgb;
	float3 emisive = T_Emissive.SampleLevel(ClampSampler, input.texcoord, 0).rgb;
	float4 SMRAO = T_Specular.SampleLevel(ClampSampler, input.texcoord, 0);
	
	float3 worldPosition = CalculateWorldPosition(input.texcoord, depth, IPM, IVM);
	float3 view = normalize(MainCamPos - worldPosition);
	float specular = SMRAO.r;
	float metallic = SMRAO.g;
	float roughness = SMRAO.b;
	float ambientOcclusion = SMRAO.a;
	float3 F0 = lerp(0.04, albedo, metallic) * specular;

	float3 OutputColor = emisive + DefaultLit(albedo, metallic, roughness, F0, worldPosition, normal, view, ambientOcclusion);
	return float4(LinearToGammaSpace(OutputColor), 1);
}