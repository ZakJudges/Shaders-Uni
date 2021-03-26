Texture2D texture0 : register(t0);
Texture2D normalmap : register(t1); 
SamplerState Sampler0 : register(s0);

cbuffer LightBuffer : register(b0)
{
    float4 diffuse;
    float4 ambient;
    float4 specular;
    float3 lightDirection;
    float specularPower;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 viewDirection : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 bitangent : TEXCOORD3;
};

float4 main(InputType input) : SV_TARGET
{
   float4 colour;
   float4 specularComponent;

    // Sample the pixel color from the texture.
    float4 textureColour = texture0.Sample(Sampler0, input.tex);
 
	// Invert the light direction for calculations.
     float3 lightDir = -lightDirection;

     // Set ambient light as default.
     colour = ambient;

    //  Sample normal map then transform to world space to get per pixel normals.
    float4 normalMap = normalmap.Sample(Sampler0, input.tex);
    //  Uncompress each component from [0,1] to [-1,1].
    normalMap = (2.0f * normalMap) - 1.0f;
    //  Multiply each vector by it's associated colour from the normal map to get the world space normal.
    float3 worldNormal = (normalMap.x * input.tangent) + (normalMap.y * input.bitangent) + (normalMap.z * input.normal);
    worldNormal = normalize(worldNormal);

    // Calculate the amount of light on this pixel.
    float diffuseIntensity = saturate(dot(worldNormal, lightDir));
    
    if (diffuseIntensity > 0.0f)
    {
		// Determine the final amount of diffuse colour.
        float4 diffuseComponent = saturate(diffuse * diffuseIntensity);
        colour += diffuseComponent;

        // Calculate specular intensity - blinn-phong.
        float3 halfway = normalize(lightDir + input.viewDirection); 
        float specularIntensity = pow(max(dot(worldNormal, halfway), 0.0), specularPower);

       // Calculate specular component and clamp the value.
       specularComponent = saturate(specular * specularIntensity);
		
    }

   // Multiply the texture pixel and the final diffuse color to get the final pixel color result.
    colour = colour * textureColour;

   // Finally, add the specular component to the colour.
    if (diffuseIntensity > 0.0f)
    {
        colour += saturate(specularComponent);
    }
 
    return colour;
}

