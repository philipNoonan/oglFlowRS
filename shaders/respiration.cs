#version 430

layout(local_size_x = 8, local_size_y = 1) in;

layout(binding = 0) uniform sampler2D depthTex;
layout(binding = 1) uniform sampler2D flowTex;

layout(std430, binding = 0) buffer respirationBufferData
{
    vec4 outValue[];
};

void main()
{
    float depth = 0.0f;
    float validDepth = 0.0f;
    vec2 flow = vec2(0.0f);

    vec4 inData = outValue[gl_GlobalInvocationID.x];
    for (int i = -5; i < 5; i++)
    {
        for (int j = -5; j < 5; j++)
        {
            float tempDepth = texelFetch(depthTex, ivec2(inData.x + i, inData.y + j), 0).x;
            if (tempDepth > 0)
            {
                validDepth++;
                depth += tempDepth;
            }
            
        }
    }

    for (int i = -25; i < 25; i++)
    {
        for (int j = -25; j < 25; j++)
        {
            vec2 tempFlow = texelFetch(flowTex, ivec2(inData.x + i, inData.y + j), 0).xy;
            
            flow += tempFlow;
            

        }
    }


    outValue[gl_GlobalInvocationID.x] = vec4(inData.xy, depth / validDepth, flow.y);



}