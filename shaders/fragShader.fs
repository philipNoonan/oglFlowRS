#version 430 core
const float PI = 3.1415926535897932384626433832795f;

layout (binding=0) uniform sampler2D currentTextureDepth;
layout (binding=1) uniform sampler2D currentTextureNormal;
layout (binding=2) uniform sampler2D currentTextureTrack;
layout (binding=3) uniform sampler2D currentTextureFlow;
layout (binding=4) uniform sampler2D currentTextureColor;
layout (binding=5) uniform sampler2D currentTextureEdges;
layout (binding=6) uniform sampler2D currentTextureDistance;

layout (binding=7) uniform isampler3D currentTextureVolume; 
layout(binding = 5, rg16i) uniform iimage3D volumeData; // texel access


in vec2 TexCoord;
in float zDepth;
in vec2 stdDev;
in float dropVertex;

layout(location = 0) out vec4 color;

uniform mat4 model;
uniform vec3 ambient;
uniform vec3 light;
uniform float irLow = 0.0f;
uniform float irHigh = 65536.0f;

uniform float slice;

uniform int texLevel;

subroutine vec4 getColor();
subroutine uniform getColor getColorSelection;

subroutine(getColor)
vec4 fromDepth()
{
    ivec2 texSize = textureSize(currentTextureDepth, texLevel);
	vec4 tColor = texelFetch(currentTextureDepth, ivec2(TexCoord.x * texSize.x, TexCoord.y * texSize.y), texLevel);
	return vec4(tColor.x, tColor.x, tColor.x, 1.0f);
}

subroutine(getColor)
vec4 fromColor()
{
    ivec2 texSize = textureSize(currentTextureColor, texLevel);
	vec4 tColor = texelFetch(currentTextureColor, ivec2(TexCoord.x * texSize.x, TexCoord.y * texSize.y), texLevel);

	//vec4 tColor = textureLod(currentTextureColor, vec2(TexCoord), texLevel);
	return vec4(tColor.xxx, 1.0);
}


subroutine(getColor)
vec4 fromRayNorm()
{
	vec4 tCol = texture(currentTextureNormal, vec2(TexCoord));
	return vec4(tCol.xy, -tCol.z, tCol.w);
}

subroutine(getColor)
vec4 fromTrack()
{
	vec4 tCol = texture(currentTextureTrack, vec2(TexCoord));
	return vec4(tCol);
}

subroutine(getColor)
vec4 fromEdges()
{
	vec2 tCol = texture(currentTextureEdges, vec2(TexCoord)).xy;
	float lCol = length(tCol);
	
	vec3 rgb;
	//if (lCol < 0.065)
	//{
	//	rgb = vec3(0.49, 0.976, 1.0);
	//}
	//else if (lCol > 0.066)
	//{
		rgb = vec3(lCol);
	//}
	//else if (lCol < 0.05)
	//{
	//	rgb = vec3(0);
    //}
	return vec4(rgb, 1.0);
}

subroutine(getColor)
vec4 fromPoints()
{
	if (dropVertex != 0.0f)
	{
		return vec4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	else
	{
		return vec4(0.03f, 0.98, 0.02f, 1.0);
	}
}

subroutine(getColor)
vec4 fromQuadtree()
{
	vec2 texSize = vec2(textureSize(currentTextureColor, texLevel).xy);

	//float u = (2.0 * float(gl_FragCoord.x)) / texSize.x - 1.0f; //1024.0f is the window resolution, change this to a uniform
    //float v = (2.0 * float(gl_FragCoord.y)) / texSize.y - 1.0f;

	float u = float(gl_FragCoord.x); // 0 - windowSize (1920)
    float v = float(gl_FragCoord.y); // 0 - 1080

	vec2 tFlow = textureLod(currentTextureFlow, vec2((u / 1920.0f), 1.0-(v / 1080.0f)), 0).xy;

	//return vec4(tFlow.xy, 0, 1);

		return vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float thresh = 0.5;
    float outCol = tFlow.x > thresh || tFlow.y > thresh ? 1.0 : 0.0;
	return vec4(114.0/255.0,144.0/255.0,154.0/255.0,outCol);
	//	return vec4(1,1,1,outCol);

	//return vec4(stdDev, 0 , 1);

	//vec2 smoothedRes = smoothstep(vec2(0,0), vec2(10,10), meanFlow);
	//return vec4(length(smoothedRes).xxx, 1);

	//return vec4(tFlow.x * tFlow.x, tFlow.y * tFlow.y, 0, 1); 

}

int ncols = 0;
int MAXCOLS = 60;
int colorwheel[60][3];


void setcols(int r, int g, int b, int k)
{
    colorwheel[k][0] = r;
    colorwheel[k][1] = g;
    colorwheel[k][2] = b;
}

void makecolorwheel()
{
    // relative lengths of color transitions:
    // these are chosen based on perceptual similarity
    // (e.g. one can distinguish more shades between red and yellow 
    //  than between yellow and green)
    int RY = 15;
    int YG = 6;
    int GC = 4;
    int CB = 11;
    int BM = 13;
    int MR = 6;
    ncols = RY + YG + GC + CB + BM + MR;
    //printf("ncols = %d\n", ncols);
    if (ncols > MAXCOLS) return;

    int i;
    int k = 0;
    for (i = 0; i < RY; i++) setcols(255,	   255*i/RY,	 0,	       k++);
    for (i = 0; i < YG; i++) setcols(255-255*i/YG, 255,		 0,	       k++);
    for (i = 0; i < GC; i++) setcols(0,		   255,		 255*i/GC,     k++);
    for (i = 0; i < CB; i++) setcols(0,		   255-255*i/CB, 255,	       k++);
    for (i = 0; i < BM; i++) setcols(255*i/BM,	   0,		 255,	       k++);
    for (i = 0; i < MR; i++) setcols(255,	   0,		 255-255*i/MR, k++);
}

subroutine(getColor)
vec4 fromFlow()
{
	int length = 50;
	//vec4 tFlow = textureLod(currentTextureFlow, TexCoord, texLevel);
	ivec2 texSize = textureSize(currentTextureColor, texLevel);
			vec4 tFlow = texelFetch(currentTextureFlow, ivec2(TexCoord.x * texSize.x, TexCoord.y * texSize.y), texLevel);
			//return vec4(1.0f, 0, 0, 1);
	//vec4 tColor = texelFetch(currentTextureColor, ivec2((TexCoord.x * texSize.x) + tFlow.x, (TexCoord.y * texSize.y) + tFlow.y), texLevel);    


	float mag = sqrt(tFlow.x * tFlow.x + tFlow.y * tFlow.y);
	float ang = atan(tFlow.y,  tFlow.x);

	//https://gist.github.com/KeyMaster-/70c13961a6ed65b6677d

	ang -= 1.57079632679;
    if(ang < 0.0) 
	{
		ang += 6.28318530718; 
    }
    ang /= 6.28318530718; 
	ang = 1.0 - ang; 

	//float smoothMag = smoothstep(0.25, 10.0, mag);

	// ang to rgb taken from https://stackoverflow.com/questions/15095909/from-rgb-to-hsv-in-opengl-glsl
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(ang + K.xyz) * 6.0 - K.www);

    vec3 rgb = mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), mag * ((texLevel + 1.0) / 1.0));

	//return vec4(1.1 * tFlow.x * tFlow.x, 1.1 * tFlow.y * tFlow.y, 0, 1); 

	//if (mag < 1.0f)
	//{
	//
	//}



	//return vec4(tFlow.x < 0 ? 1 : 0, tFlow.y < 0 ? 1 : 0, 0, 1);
	return vec4((1.0 - rgb)*1.0, mag > 0.05 ? (mag < 0.50 ? mag / 0.50 : 1.0) : 0.0);
	//	return vec4((1.0 - rgb)*10.0, 1.0);

}

subroutine(getColor)
vec4 fromVolume()
{
	ivec4 tData = texture(currentTextureVolume, vec3(TexCoord, slice) );
	//ivec4 tData = imageLoad(volumeData, ivec3(TexCoord.x * 128.0f, TexCoord.y * 128.0f, slice));

	return vec4(1.0f * float(tData.x) * 0.00003051944088f, 1.0f * float(tData.x) * -0.00003051944088f, 0.0, 1.0f);
	//return vec4(1.0, 0.0, 0.0, 1.0);
}

subroutine(getColor)
vec4 fromDistance()
{
	vec4 tCol = texture(currentTextureDistance, vec2(TexCoord));
	return vec4(tCol);

}


//subroutine(getColor)
//vec4 fromQuadtree()
//{
//	vec4 tCol = texture(currentTextureDistance, vec2(TexCoord));
//	return vec4(tCol);
//}

void main()
{
	//vec3 normals = normalize(cross(dFdx(vert4D.xyz), dFdy(vert4D.xyz)));



	color = getColorSelection();

}