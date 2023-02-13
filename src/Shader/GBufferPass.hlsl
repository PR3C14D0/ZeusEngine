struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 normal : NORMAL0;
};

VertexOutput VertexMain(float4 position : POSITION, float2 uv : TEXCOORD, float4 normal : NORMAL)
{
    VertexOutput output;
    output.position = position;
    output.uv = uv;
    output.normal = normal;
    return output;
}

struct PixelOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
};

PixelOutput PixelMain(VertexOutput input)
{
    PixelOutput output;
    output.albedo = float4(input.uv.xy, 0.f, 1.f);
    output.normal = input.normal;
    return output;
}