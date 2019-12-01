#define SAKURACORE_ECS_API_EXPORTS
#include "pch.h"


using namespace SECS;
using namespace SakuraGraphics;

SAKURACORE_ECS_API bool __stdcall InitWorld()
{
	auto world = SECS::SWorld::CreateSWorld("Debug");

	world->CreateEntity<DX12DeviceComponent>();
	world->IncludeSystemGroup("DX12Device");



	return true;
}
