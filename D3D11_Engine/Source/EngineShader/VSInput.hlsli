struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL0;
    float3 Tangent : NORMAL1;
    float2 Tex : TEXCOORD0;
#ifdef VERTEX_SKINNING
    int4 BlendIndecses : BLENDINDICES;
    float4 BlendWeights : BLENDWEIGHT;
#endif
};