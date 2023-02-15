struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 normal : NORMAL0;
};

cbuffer WVP : register(b0)
{
    matrix Model;
    matrix View;
    matrix Projection;
}

VertexOutput VertexMain(float4 position : POSITION, float2 uv : TEXCOORD, float4 normal : NORMAL)
{
    VertexOutput output;
    output.position = mul(position, Model);
    output.position = mul(output.position, View);
    output.position = mul(output.position, Projection);
    output.uv = uv;
    output.normal = normalize(mul(normal, Model));
    return output;
}

Texture2D modelTex : register(t0);
SamplerState sState : register(s0);

struct PixelOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
};

PixelOutput PixelMain(VertexOutput input)
{
    PixelOutput output;
    output.albedo = modelTex.Sample(sState, float2(input.uv.x, 1 - input.uv.y));
    float3 normal = float3(input.normal.xyz * 0.5f + float3(0.5f, 0.5f, 0.5f)); // Pack our normals
    output.normal = float4(normal, 1.f);
    return output;
}