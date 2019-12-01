#pragma once
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "../../Thirdparty/stb_image.h"

namespace HikaCommonUtils
{
	struct RawTexture
	{
		float* data = nullptr;
		int width, height, nrComponents;
	};
	class TextureLoader
	{
		inline static void LoadTexture(RawTexture& _target, const std::string& _path)
		{
			_target.data = stbi_loadf(_path.c_str(), &(_target.width), &(_target.height), &(_target.nrComponents), 0);
		}
	};
}
