#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Framework/Core/Nodes/Properties/SakuraNodeProperties.h"

//Link necessary d3d12 libraries
#if defined(_X86_)
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "x86/Debug/assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "x86/Release/assimp-vc142-mt.lib")
#endif
#elif defined(_AMD64_)
#pragma comment(lib, "x64/Debug/assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "x64/Release/assimp64-vc142-mt.lib")
#endif

namespace AssimpUtils
{
	__forceinline static void LoadTransformFromAssimpMatrix(SakuraCore::STransform& trans, const aiMatrix4x4& assimpTrans)
	{
		trans.set(assimpTrans.a1, assimpTrans.a2, assimpTrans.a3,
			assimpTrans.b1, assimpTrans.b2, assimpTrans.b3,
			assimpTrans.c1, assimpTrans.c2, assimpTrans.c3,
			assimpTrans.a4, assimpTrans.b4, assimpTrans.c4);
	}
};