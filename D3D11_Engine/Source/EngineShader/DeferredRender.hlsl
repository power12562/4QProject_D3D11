
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/Light.hlsli"

Texture2D T_Albedo : register(t0); //Albedo rgb
Texture2D T_Specular : register(t1); //Specular.r + Metallic.g + Roughness.b + AO.a
Texture2D T_Normal : register(t2); //Normal.rgb
Texture2D T_Emissive : register(t3); //Emissive.rgb
Texture2D T_Depth : register(t4); //Depth.r
	
RWTexture2D<float4> T_Output : register(u0);

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

[numthreads(64, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint width, height;
	T_Output.GetDimensions(width, height);
	
	float2 uv = float2(DTid.x / (float) width, DTid.y / (float) height);
	
	//CS에서는 Load가 정확함
	//해상도가 일치한다는 약속하에
	
	float depth = T_Depth.Load(DTid).r;
	if (depth == 1.0)
	{
		return;
	}
	
	float3 albedo = T_Albedo.Load(DTid).rgb;
	float3 normal = T_Normal.Load(DTid).rgb;
	float3 emisive = T_Emissive.Load(DTid).rgb;
	float4 SMRAO = T_Specular.Load(DTid);
	
	float3 worldPosition = CalculateWorldPosition(uv, depth, IPM, IVM);
	float3 view = normalize(MainCamPos - worldPosition);
	float specular = SMRAO.r;
	float metallic = SMRAO.g;
	float roughness = SMRAO.b;
	float ambientOcclusion = SMRAO.a;
	
	float3 F0 = lerp(0.04, albedo, metallic) * specular;	
	float3 OutputColor = emisive + DefaultLit(albedo, metallic, roughness, F0, worldPosition, normal, view, ambientOcclusion);
	T_Output[DTid.xy] = float4(LinearToGammaSpace(OutputColor), 1);
	//T_Output[DTid.xy] = float4(worldPosition, 1);
	//T_Output[DTid.xy] = float4(0, (depth - 0.98) * 50, 0, 1);

}