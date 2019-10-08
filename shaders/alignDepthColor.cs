#version 430

layout(local_size_x = 32, local_size_y = 32) in;

// bind images
layout(binding = 0, r16ui) readonly uniform uimage2D srcDepthMap;
layout(binding = 1, rgba8) readonly uniform image2D srcColorMap;
layout(binding = 2, r16ui) writeonly uniform uimage2D dstDepthMap;
layout(binding = 3, rgba8) writeonly uniform image2D dstColorMap;

uniform mat4 d2c;
uniform vec4 camDepth;
uniform vec4 camColor;

uniform float depthScale;



vec3 projectPointImage(vec3 p, vec4 cam)
{
    return vec3(((cam.z * p.x) / p.z) + cam.x,
                ((cam.w * p.y) / p.z) + cam.y,
                p.z);
}

//Cam is //cx, cy, 1 / fx, 1 / fy
vec3 getVertex(float z, ivec2 pix, vec4 bpCam)
{
    return vec3((float(pix.x) - bpCam.x) * z * bpCam.z, (float(pix.y) - bpCam.y) * z * bpCam.w, z);
}


void main()
{
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);

    uint depth = imageLoad(srcDepthMap, pix).x;

    float z = depth * depthScale;

    //imageStore(dstDepthMap, pix, uvec4(0));


    vec3 vertex = getVertex(z, pix, vec4(camDepth.x, camDepth.y, 1.0f / camDepth.z, 1.0f / camDepth.w));

    vec4 vertexInColor = d2c * vec4(vertex, 1.0f);

    vec3 colPix = projectPointImage(vertexInColor.xyz, camColor);

    ivec2 outPix = ivec2(colPix.x + 0.5f, colPix.y + 0.5f);



    imageStore(dstDepthMap, outPix + ivec2(0, 1), uvec4(depth));
    imageStore(dstDepthMap, outPix + ivec2(1, 1), uvec4(depth));
    imageStore(dstDepthMap, outPix + ivec2(1, 0), uvec4(depth));

    imageStore(dstDepthMap, outPix, uvec4(depth));

    imageStore(dstDepthMap, outPix - ivec2(0, 1), uvec4(depth));
    imageStore(dstDepthMap, outPix - ivec2(1, 1), uvec4(depth));
    imageStore(dstDepthMap, outPix - ivec2(1, 0), uvec4(depth));

    imageStore(dstDepthMap, outPix + ivec2(-1, 1), uvec4(depth));
    imageStore(dstDepthMap, outPix + ivec2(1, -1), uvec4(depth));

    imageStore(dstColorMap, pix, imageLoad(srcColorMap, ivec2(colPix.x + 0.5f, colPix.y + 0.5f)));



    //imageStore(dstColorMap, pix, vec4(vertex.xyz, 1.0f)); 



}