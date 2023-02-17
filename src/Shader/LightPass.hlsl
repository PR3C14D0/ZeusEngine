struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

Texture2D albedo : register(t0);
Texture2D normal : register(t1);
Texture2D depthTex : register(t2);
Texture2D posTex : register(t3);

SamplerState sState : register(s0);

cbuffer WVP : register(b0)
{
    matrix World; // This member will be an identity matrix, so we won't use it.
    matrix View;
    matrix Projection;
}

VertexOutput VertexMain(float4 position : POSITION, float2 uv : TEXCOORD)
{
    VertexOutput output;
    output.position = position;
    output.uv = uv;
    return output;
}

float4 PixelMain(VertexOutput input) : SV_Target {
    float4 finalColor;
    
    float4 albedoColor =  albedo.Sample(sState, input.uv);
    float4 nml = normalize(normal.Sample(sState, input.uv) * 2 - float4(1.f, 1.f, 1.f, 1.f));
    float4 position = posTex.Sample(sState, input.uv);
    float depth = depthTex.Sample(sState, input.uv).x;
    
    float4 lightPos = float4(0.f, 1.f, -1.f, 1.f);
    float4 lightDir = normalize(lightPos - position);
    float NdotL = dot(lightDir, nml);
    
    float diffuseColor = saturate(NdotL);
    float4 ambientColor = .1f;
    float4 lightColor = float4(1.f, 1.f, 0.f, 1.f);
    
    finalColor = (saturate(diffuseColor + ambientColor) * lightColor) * albedoColor;
    
    float4 light2Pos = float4(0.f, -1.f, 0.f, 1.f);
    float4 light2Dir = normalize(light2Pos - position);
    float4 light2Color = float4(0.f, 0.f, 1.f, 1.f);
    float NdotL2 = dot(light2Dir, nml);
    float diffuse2Color = saturate(NdotL2);
    
    float4 final2Color = (saturate(diffuse2Color + ambientColor) * light2Color) * albedoColor;
    
    albedoColor = finalColor + final2Color;
    
    return albedoColor;
}
