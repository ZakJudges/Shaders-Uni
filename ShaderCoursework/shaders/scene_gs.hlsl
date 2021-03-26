
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer GeometryBuffer : register(b1)
{
    float3 cameraPos;
    float time;
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TEXCOORD2;
    float3 bitangent : TEXCOORD3;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 viewDirection : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 bitangent : TEXCOORD3;
};

[maxvertexcount(3)]
void main(triangle InputType input[3]
, inout TriangleStream<OutputType> triStream)
{
    float4 vertexPosition;
    OutputType output;

    //  Loop throught each vertex in the triangle and determine the output values for use in the pixel shader.
    for (int i = 0; i < 3; i++)
    {   
        input[i].position.w = 1.0f;

        //  Calculates the viewing direction vector for use in specular calculations.
        vertexPosition = input[i].position;
        vertexPosition = mul(vertexPosition, worldMatrix);
        output.viewDirection = cameraPos.xyz - vertexPosition.xyz;
        output.viewDirection = normalize(output.viewDirection);

        output.tex = input[i].tex;

        //  Get the normals, tangents and bitangents against the world matrix.
        //  No translation is occuring here, so 3x3 matrices are used.
        output.normal = input[i].normal;
        output.normal = mul(output.normal, (float3x3) worldMatrix);
        output.normal = normalize(output.normal);

        output.tangent = input[i].tangent;
        output.tangent = mul(output.tangent, (float3x3) worldMatrix);
        output.tangent = normalize(output.tangent);

        output.bitangent = input[i].bitangent;
        output.bitangent = mul(output.bitangent, (float3x3) worldMatrix);
        output.bitangent = normalize(output.bitangent);
    
        //  Calculate a normal for the face, not the vertex.
        //  Normal must be calculated per face so that the vertices of each primitive move in the correct direction.
        float3 faceNormal = cross(input[2].position.xyz - input[0].position.xyz, input[1].position.xyz - input[0].position.xyz);
        faceNormal = normalize(faceNormal);
        output.position = input[i].position + float4(time * faceNormal, 0.0f);

        //  Calculate the position against the world, view and projection matrices.
        output.position = mul(output.position, worldMatrix);
        output.position = mul(output.position, viewMatrix);
        output.position = mul(output.position, projectionMatrix);
  
        triStream.Append(output);
    }

    triStream.RestartStrip();
}
 
