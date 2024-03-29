#include "glhelper.h"

namespace GLHelper
{
	GLuint createTexture(GLuint ID, GLenum target, int levels, int w, int h, int d, GLint internalformat, GLenum magFilter, GLenum minFilter)
	{
		GLuint texid;

		if (ID == 0)
		{
			glGenTextures(1, &texid);
		}
		else
		{
			glDeleteTextures(1, &ID);
			texid = ID;
			glGenTextures(1, &texid);
		}

		glGenTextures(1, &texid);
		glBindTexture(target, texid);

		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		// https://stackoverflow.com/questions/15405869/is-gltexstorage2d-imperative-when-auto-generating-mipmaps
		//glTexImage2D(target, 0, internalformat, w, h, 0, format, type, 0); // cretes mutable storage that requires glTexImage2D

		if (target == GL_TEXTURE_1D)
		{
			glTexStorage1D(target, levels, internalformat, w);
		}
		else if (target == GL_TEXTURE_2D)
		{
			glTexStorage2D(target, levels, internalformat, w, h); // creates immutable storage and requires glTexSubImage2D

		}
		else if (target == GL_TEXTURE_3D || d > 0)
		{
			glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
			glTexStorage3D(target, levels, internalformat, w, h, d);
		}

		float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);

		return texid;
	}
	
	uint32_t nextPowerOfTwo(uint32_t n)
	{
		--n;

		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;

		return n + 1;
	}

	uint32_t numberOfLevels(glm::ivec3 dims)
	{
		return 1 + floor(std::log2(max3(dims.x, dims.y, dims.z)));
	}

}
