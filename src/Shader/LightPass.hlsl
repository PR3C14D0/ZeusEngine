struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

Texture2D albedo : register(t0);
Texture2D normal : register(t1);

SamplerState sState : register(s0);

VertexOutput VertexMain(float4 position : POSITION, float2 uv : TEXCOORD)
{
    VertexOutput output;
    output.position = position;
    output.uv = uv;
    return output;
}

float4 PixelMain(VertexOutput input) : SV_Target {
    return albedo.Sample(sState, input.uv);
}
