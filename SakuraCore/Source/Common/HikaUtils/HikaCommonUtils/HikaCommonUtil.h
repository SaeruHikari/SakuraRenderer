#pragma once
#include <winnt.h>
#include <string>
#include <intsafe.h>
#include <stdlib.h>

namespace HikaCommonUtils
{
	inline LPCWSTR stringToLPCWSTR(std::string orig)
	{
		size_t origsize = orig.length() + 1;
		const size_t newsize = 100;
		size_t convertedChars = 0;
		wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t) * (orig.length() - 1));
		mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);

		return wcstring;
	}


	inline std::string readFileIntoString(char* filename)
	{
		std::ifstream ifile(filename);
		//将文件读入到ostringstream对象buf中
		std::ostringstream buf;
		char ch;
		while (buf && ifile.get(ch))
			buf.put(ch);
		//返回与流对象buf关联的字符串
		return buf.str();
	}


	inline std::wstring StringToWstring(const std::string str)
	{ 
		int num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
		wchar_t* wide = new wchar_t[num];
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide, num);
		std::wstring w_str(wide);
		delete[] wide;
		return w_str;
	}
	
	inline std::string WstringToString(const std::wstring wstr)
	{
		if (wstr.empty())
		{
			return std::string();
		}
#if defined WIN32
		int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], wstr.size(), NULL, 0, NULL, NULL);
		std::string ret = std::string(size, 0);
		WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], wstr.size(), &ret[0], size, NULL, NULL);
#else
		size_t size = 0;
		_locale_t lc = _create_locale(LC_ALL, "en_US.UTF-8");
		errno_t err = _wcstombs_s_l(&size, NULL, 0, &wstr[0], _TRUNCATE, lc);
		std::string ret = std::string(size, 0);
		err = _wcstombs_s_l(&size, &ret[0], size, &wstr[0], _TRUNCATE, lc);
		_free_locale(lc);
		ret.resize(size - 1);
#endif
		return ret;
	}
}