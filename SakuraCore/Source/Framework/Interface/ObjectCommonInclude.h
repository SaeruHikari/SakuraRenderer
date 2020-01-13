#pragma once
#include "Common/Thirdparty/guid.hpp"
#if defined(_X86_)
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "x86/Debug/crossguid-dgb.lib")
#else
#pragma comment(lib, "x86/Release/crossguid.lib")
#endif
#elif defined(_AMD64_)
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "x64/Debug/crossguid64-dgb.lib")
#else
#pragma comment(lib, "x64/Release/crossguid64.lib")
#endif
#endif
