#include "Shared.hlsli"
#ifdef VERTEX_SKINNING
cbuffer MatrixPallete : register(b3)
{
    matrix MatrixPalleteArray[128];
    matrix boneWIT[128];
}
#endif

#include "VSInput.hlsli"
PS_INPUT main(VS_INPUT input) 
{
    PS_INPUT output = (PS_INPUT)0;
    float4 pos = input.Pos;
    Matrix matWorld;
#ifdef VERTEX_SKINNING
    matWorld = mul(input.BlendWeights.x, MatrixPalleteArray[input.BlendIndecses.x]);
    matWorld += mul(input.BlendWeights.y, MatrixPalleteArray[input.BlendIndecses.y]);
    matWorld += mul(input.BlendWeights.z, MatrixPalleteArray[input.BlendIndecses.z]);
    matWorld += mul(input.BlendWeights.w, MatrixPalleteArray[input.BlendIndecses.w]);
#else
    matWorld = World;
#endif 
    pos = mul(pos, matWorld);
    pos = mul(pos, ShadowViews[0]);
    pos = mul(pos, ShadowProjections[0]);
    output.Pos = pos;
    return output;
}