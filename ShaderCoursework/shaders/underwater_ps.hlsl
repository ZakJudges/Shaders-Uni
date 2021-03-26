Texture2D sceneTexture : register(t0);
SamplerState Sampler0 : register(s0);

cbuffer TimeBuffer : register(b0)
{
    float time;
    float3 padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


float4 main(InputType input) : SV_TARGET
{
    float4 textureColour;
    
    //  Amplitude of the sine wave.
    float height = 0.0025;
   
    //  How often the waves occur.
    float frequency = 9.42f;

    //  For each wave in the x and y directions, the texture coordinate at this pixel is multiplied by the frequency.
    //  Time is then added to introduce movement.
    float offsetX = time * 2.0f + input.tex.y * frequency;
    float offsetY = time * 2.0f + input.tex.x * frequency;

    //  The sine of the pixel offset values gives a repeating wave motion.
    float2 pixelOffset = float2(height * sin(offsetX), height * sin(offsetY));

    //  Offset the texture coordinate.
    input.tex += pixelOffset;

    //  Sample the texture at the newly offset coordinate.
    textureColour = sceneTexture.Sample(Sampler0, input.tex);

    //  Darken the pixel.
    textureColour.rgb *= 0.5;

    //  Tint the pixel blue.
    textureColour.r += 0.02f;
    textureColour.b += 0.05f;
    textureColour.g += 0.025f;

    return textureColour;
}


