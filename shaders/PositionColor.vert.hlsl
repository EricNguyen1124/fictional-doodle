struct Input
{
    float3 Position: TEXCOORD0;
};

struct Output
{
    float4 Color: TEXCOORD0;
    float4 Position: SV_Position;
};

Output main(Input input)
{
    Output output;
    output.Color = float4(1,0,0,1);
    output.Position = float4(input.Position, 1.0f);
    return output;
}