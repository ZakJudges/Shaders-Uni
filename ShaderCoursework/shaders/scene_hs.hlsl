cbuffer tessellationBuffer : register(b0)
{
    float tessellationFactor;
    float3 padding;
};

struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TEXCOORD1;
    float3 bitangent : TEXCOORD2;
};

struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct OutputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TEXCOORD2;
    float3 bitangent : TEXCOORD3;
};

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 3> inputPatch)
{
    ConstantOutputType output;

    //  Set the tessellation factor to be user-input defined.
    float tessFactors = (int) tessellationFactor;

    // Set the tessellation factors for the three edges of the triangle.
    output.edges[0] = tessFactors;
    output.edges[1] = tessFactors;
    output.edges[2] = tessFactors;

    // Set the tessellation factor for tessallating inside the triangle.
    output.inside = tessFactors;

    return output;
}


[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 3> patch, uint pointId : SV_OutputControlPointID)
{
    OutputType output;

    // Pass necessary control point values to domain shader to be dealt with after tessellation.
    output.position = patch[pointId].position;
    output.tex = patch[pointId].tex;
    output.normal = patch[pointId].normal;
    output.tangent = patch[pointId].tangent;
    output.bitangent = patch[pointId].bitangent;

    return output;
}