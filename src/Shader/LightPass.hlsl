struct VertexOutput
{
    float4 position : SV_Position;
};

Texture2DMS<float4> albedo : register(t0);
Texture2DMS<float4> normal : register(t1);
Texture2DMS<float4> depthTex : register(t2);
Texture2DMS<float4> posTex : register(t3);

SamplerState sState : register(s0);

cbuffer WVP : register(b0)
{
    matrix World; // This member will be an identity matrix, so we won't use it.
    matrix View;
    matrix Projection;
}

VertexOutput VertexMain(float4 position : POSITION)
{
    VertexOutput output;
    output.position = position;
    return output;
}

float4 PixelMain(VertexOutput input, uint sampleIndex : SV_SampleIndex) : SV_Target
{
    float4 finalColor;
    
    float4 albedoColor =  albedo.Load(input.position.xy, sampleIndex);
    float4 nml = normalize(normal.Load(input.position.xy, sampleIndex) * 2 - float4(1.f, 1.f, 1.f, 1.f));
    float4 position = posTex.Load(input.position.xy, sampleIndex);
    float depth = depthTex.Load(input.position.xy, sampleIndex).x;
    
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
