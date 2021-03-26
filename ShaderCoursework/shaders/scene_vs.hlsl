
struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TEXCOORD1;
    float3 bitangent : TEXCOORD2;
};

OutputType main(InputType input)
{
    OutputType output;

    //  Pass necessary values to hull shader.
    output.position = input.position;
    output.tex = input.tex;
    output.normal = input.normal;
    //  The terrain is a plane initially so all tangents are the same for each vertex. The same goes for bitangents.
    output.tangent = float3(1.0f, 0.0f, 0.0f);
    output.bitangent = float3(0.0f, 0.0f, -1.0f);
    
    return output;
}
