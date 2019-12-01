#pragma once

#ifdef SAKURACORE_ECS_API_EXPORTS
#define SAKURACORE_ECS_API __declspec(dllexport)
#else
#define SAKURACORE_ECS_API __declspec(dllimport)
#endif

extern "C"
{
	SAKURACORE_ECS_API bool __stdcall InitWorld();
}
