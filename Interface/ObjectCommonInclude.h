#pragma once
#include "Common/Thirdparty/guid.hpp"
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "crossguid-dgb.lib")
#else
#pragma comment(lib, "crossguid.lib")
#endif