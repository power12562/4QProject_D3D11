struct VS_INPUT
{
    float4 position : POSITION;
    float2 Tex : TEXCOORD;
};

struct PS_Deferred
{
    float4 svposition : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

PS_Deferred main(VS_INPUT input)
{
    PS_Deferred output;
    
    // Apply the viewport transform matrix to the NDC position
    output.svposition = input.position;

    // Pass through texture coordinates
    output.Tex = input.Tex;  
    return output;
}