/*****************************************************************************************
				             Copyrights   SaeruHikari
Description:				Hika util of template packs.
Details:		
*******************************************************************************************/
#pragma once
#include <stdint.h>

class TemplatePackUtils
{
public:
	template <typename ... Ts>
	inline static uint32_t TemplatePack_SizeOfT()
	{
		uint32_t num = 0;
		TemplatePack_SizeOfT_Internal<Ts...>(num);
		return num;
	}

	template<typename ... Ts>
	inline static size_t* TemplatePack_TSizes()
	{
		size_t* sizes = new size_t[sizeof...(Ts)]();
		templatePack_TSizes_Internal<Ts...>(sizeof...(Ts), sizes);
		return sizes;
	}
	
	template <typename ...Ts>
	inline static size_t* GetHashes_Torder()
	{
		size_t* res = new size_t[sizeof...(Ts)]();
		getHash_Internal_Torder<Ts...>(sizeof...(Ts), res);
		return res;
	}

	template <typename ... Ts>
	inline static size_t* GetHashes()
	{
		size_t* res = new size_t[sizeof...(Ts)]();
		getHash_Internal<Ts...>(sizeof...(Ts), res);
		return res;
	}

	template<typename ... __Args>
	inline static bool match(size_t* _inhashes, size_t size)
	{
		if (size != sizeof...(__Args)) 
			return false;
		else
		{
			size_t* __selfHash = GetHashes<__Args...>();
			for (size_t i = 0; i < sizeof...(__Args); i++)
			{
				if (_inhashes[i] != __selfHash[i])
				{
					return false;
				}
			}
		}
		return true;
	}
public:
	template<typename ... __Args>
	struct ArcheTemplate
	{
		template<typename ... __InArgs>
		bool match()
		{
			if (sizeof...(__Args) != sizeof...(__InArgs)) return false;
			else
			{
				size_t* __inHash = GetHashes<__InArgs...>();
				size_t* __selfHash = GetHashes<__Args...>();
				for (size_t i = 0; i < sizeof...(__InArgs); i++)
				{
					if (__inHash[i] != __selfHash[i])
					{
						return false;
					}
				}
			}
			return true;
		}
	};

private:

	template<typename T>
	inline static void templatePack_TSizes_Internal(size_t _len, size_t* _targ)
	{
		_targ[_len - 1] = sizeof(T);
	}

	template<typename T, typename T2, typename ... __Ts>
	inline static void templatePack_TSizes_Internal(size_t _len, size_t* _targ)
	{
		templatePack_TSizes_Internal<T>((size_t)(_len - sizeof...(__Ts)) - 1, _targ);
		templatePack_TSizes_Internal<T2, __Ts...>(_len, _targ);
	}

	template<typename T>
	inline static void getHash_Internal_Torder(size_t _pos, size_t* _targ)
	{
		_targ[_pos - 1] = typeid(T).hash_code();
	}
	template<typename T, typename T2, typename ... __Ts>
	inline static void getHash_Internal_Torder(size_t _len, size_t* _targ)
	{
		getHash_Internal_Torder<T>((size_t)(_len - sizeof...(__Ts)) - 1, _targ);
		getHash_Internal_Torder<T2, __Ts...>(_len, _targ);
	}

	template<typename T>
	inline static void getHash_Internal(size_t _pos, size_t* _targ)
	{
		// do insert, hash small ~ big
		int i = _pos - 2;
		for (i; i >= 0 & typeid(T).hash_code() < _targ[i]; i--)
		{
			_targ[i + 1] = _targ[i];
		}
		_targ[i + 1] = typeid(T).hash_code();
	}
	template<typename T, typename T2, typename ... __Ts>
	inline static void getHash_Internal(size_t _len, size_t* _targ)
	{
		getHash_Internal<T>((size_t)(_len - sizeof...(__Ts)) - 1, _targ);
		getHash_Internal<T2, __Ts...>( _len, _targ);
	}

	template <typename T>
	inline static void TemplatePack_SizeOfT_Internal(uint32_t& num)
	{
		num += sizeof(T);
	}

	template <typename T, typename T2, typename ... Ts>
	inline static void TemplatePack_SizeOfT_Internal(uint32_t& num)
	{
		TemplatePack_SizeOfT_Internal<T>(num);
		TemplatePack_SizeOfT_Internal<T2, Ts...>(num);
	}
};