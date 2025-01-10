#include "../EngineShader/Shared.hlsli"
#ifdef VERTEX_SKINNING
cbuffer MatrixPallete : register(b3)
{
    matrix MatrixPalleteArray[128];
    matrix boneWIT[128];
}
#endif

#include "../EngineShader/VSInput.hlsli"
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
	Matrix matWorld;
	Matrix VP = mul(View, Projection);
	float3 normal;
	float3 tangent;
    
#ifdef VERTEX_SKINNING
    matWorld = mul(input.BlendWeights.x, MatrixPalleteArray[input.BlendIndecses.x]);
    matWorld += mul(input.BlendWeights.y, MatrixPalleteArray[input.BlendIndecses.y]);
    matWorld += mul(input.BlendWeights.z, MatrixPalleteArray[input.BlendIndecses.z]);
    matWorld += mul(input.BlendWeights.w, MatrixPalleteArray[input.BlendIndecses.w]);
#else
    matWorld = World;
#endif   
       
#ifdef VERTEX_SKINNING
    Matrix WIT;
    WIT = mul(input.BlendWeights.x, boneWIT[input.BlendIndecses.x]);
    WIT += mul(input.BlendWeights.y, boneWIT[input.BlendIndecses.y]);
    WIT += mul(input.BlendWeights.z, boneWIT[input.BlendIndecses.z]);
    WIT += mul(input.BlendWeights.w, boneWIT[input.BlendIndecses.w]);
    
	normal = normalize(mul(input.Normal, (float3x3) WIT));
	tangent = normalize(mul(input.Tangent, (float3x3) WIT));
#else
	normal = normalize(mul(input.Normal, (float3x3) WorldInverseTranspose));
	tangent = normalize(mul(input.Tangent, (float3x3) WorldInverseTranspose));
#endif  
    
	float4 worldPos = mul(input.Pos, matWorld);
	output.World = (float3) worldPos;
	output.Pos = mul(worldPos, VP);
	output.Normal = normal;
	output.Tangent = tangent;
    
    output.BiTangent = normalize(cross(output.Normal, output.Tangent));
    
    output.Tex = input.Tex;
    
    [unroll]
    for (int i = 0; i < MAX_LIGHT_COUNT; i++)
    {
        output.PositionShadows[i] = mul(float4(output.World.xyz, 1.0f), ShadowViews[i]);
        output.PositionShadows[i] = mul(output.PositionShadows[i], ShadowProjections[i]);       
    }
    return output;
}