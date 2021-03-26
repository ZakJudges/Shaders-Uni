Texture2D heightmap : register(t0);
SamplerState Sampler0 : register(s0);

cbuffer HeightBuffer : register(b0)
{
    float heightScale;
    float3 padding;
};

struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TEXCOORD2;
    float3 bitangent : TEXCOORD3;
};

struct OutputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TEXCOORD2;
    float3 bitangent : TEXCOORD3;
};


[domain("tri")]
OutputType main(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 3> patch)
{
    float3 vertexPosition;
    float3 worldPosition;
    OutputType output;

    //  Obtain the new texel, normal, tangent and bitangent. 
    vertexPosition = uvwCoord.x * patch[0].position + uvwCoord.y * patch[1].position + uvwCoord.z * patch[2].position;
    output.tex = uvwCoord.x * patch[0].tex + uvwCoord.y * patch[1].tex + uvwCoord.z * patch[2].tex;
    output.normal = uvwCoord.x * patch[0].normal + uvwCoord.y * patch[1].normal + uvwCoord.z * patch[2].normal;
    output.tangent = uvwCoord.x * patch[0].tangent + uvwCoord.y * patch[1].tangent + uvwCoord.z * patch[2].tangent;
    output.bitangent = uvwCoord.x * patch[0].bitangent + uvwCoord.y * patch[1].bitangent + uvwCoord.z * patch[2].bitangent;

	//	Offset vertex position based on height map.
    float height = heightmap.SampleLevel(Sampler0, output.tex, 0).r;
    vertexPosition.y = height * heightScale;

    //  Pass output position to geometry shader.
    output.position = float4(vertexPosition, 1.0f);
 
    return output;

}