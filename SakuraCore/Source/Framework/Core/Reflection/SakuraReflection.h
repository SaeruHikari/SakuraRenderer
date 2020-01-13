/*****************************************************************************************
							 Copyrights   SaeruHikari
CreateDate:							2020.1.2
Description:	        Utils for rttr in Sakura for Editor...
Details:
*******************************************************************************************/
#pragma once
#include <rttr/type>
#include <rttr/registration>

using namespace rttr;

#define Sakura_Editor 1
#define REFLECTION_ENABLE RTTR_ENABLE 


/**/
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "x64/Debug/rttr_core.lib")
#else
#pragma comment(lib, "x64/Release/rttr_core.lib")
#endif

#ifndef REFLECTION_ENGINE
#define REFLECTION_ENGINE(...)\
	RTTR_REGISTRATION{__VA_ARGS__}
#endif // !REFLECTION_ENGINE

#if defined(Sakura_Editor)
#define REFLECTION_EDITOR(...) \
	RTTR_REGISTRATION{__VA_ARGS__}
#else
#define REFLECTION_EDITOR(...) ;
#endif
	