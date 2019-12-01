#pragma once
#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>

namespace SakuraGraphics {

	inline std::wstring AnsiToWString(const std::string& str)
	{
		WCHAR buffer[512];
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
		return std::wstring(buffer);
	}

	class Dx12Exception
	{
	public:
		Dx12Exception() = default;
		Dx12Exception(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber)
			: ErrorCode(hr),
			FunctionName(functionName),
			Filename(filename),
			LineNumber(lineNumber) {};

		std::wstring ToString() const;

		HRESULT ErrorCode = S_OK;
		std::wstring FunctionName;
		std::wstring Filename;
		int LineNumber = -1;
	};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
	{                                                                     \
		HRESULT hr__ = (x);                                               \
		std::wstring wfn = AnsiToWString(__FILE__);                       \
		if(FAILED(hr__)) { throw Dx12Exception(hr__, L#x, wfn, __LINE__); } \
	}
#endif

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif
}