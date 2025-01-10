#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/SkyBoxShared.hlsli"

PixelShaderInput main(float4 localPosition : POSITION)
{
    PixelShaderInput output = (PixelShaderInput)0;
    output.TexCoords = normalize(localPosition.xyz);
    
    // 역행렬에서 위치는 적용안함 카메라 회전은 반영됨
    float3 pos3 = mul(localPosition.xyz, (float3x3)View);
    float4 pos4 = mul(float4(pos3, 1.f), Projection);
    output.PositionProj = pos4;
    return output;
}