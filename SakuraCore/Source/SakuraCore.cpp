#define SAKURACORE_API_EXPORTS
#include "SakuraCore.h"
#include "Framework\Core\SCore.h"
#include "Framework\Core\Nodes\EngineNodes\SStaticMeshNode.hpp"

SAKURACORE_API bool __stdcall CreateSakuraCore(UINT GraphicsAPIMask)
{
	return (SakuraCore::SCore::CreateSakuraCore(CORE_GRAPHICS_API_CONF(GraphicsAPIMask)) != nullptr);  
}

SAKURACORE_API bool __stdcall InitSakuraGraphicsCore(HWND hwnd, UINT width, UINT height)
{
	return SakuraCore::SCore::GetSakuraCore()->SakuraInitializeGraphicsCore(hwnd, width, height);
}

SAKURACORE_API void __stdcall TickSakuraCore(double deltaTime, UINT coreMask /*= 0*/)
{
	SakuraCore::SCore::GetSakuraCore()->TickSakuraCore(deltaTime, coreMask);
}

SAKURACORE_API void __stdcall MsgSakuraCore(UINT coreMask, UINT MSG, UINT param0, UINT param1, UINT param2)
{
	if ((coreMask & SAKURA_CORE_COMPONENT_MASK_GRAPHICS) != 0)
	{
		SakuraCore::SCore::GetSakuraCore()->MsgSakuraGraphicsCore(MSG, param0, param1, param2);
	}
}

SAKURACORE_API int __stdcall Run()
{
	return SakuraCore::SCore::GetSakuraCore()->Run();
}

SAKURACORE_API void __stdcall MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return SakuraCore::SCore::GetSakuraCore()->MsgProc(hwnd, msg, wParam, lParam);
}

SAKURACORE_API void __stdcall ShutDown()
{
	return SakuraCore::SCore::GetSakuraCore()->ShutDown();
}

SAKURACORE_API UINT __stdcall GetChildrenNum(void* parent /*= nullptr*/)
{
	if (!parent)
		return 0;
	return
		((SakuraSceneNode*)parent)->GetChildNum();
}

SAKURACORE_API void* __stdcall GetSceneNode(void* parent, UINT index)
{
	return SakuraCore::SCore::GetSakuraCore()->GetNode((SakuraSceneNode*)parent, index);
}

#pragma region Export_REFLECTION
SAKURACORE_API void __stdcall GetStringProp(void* object, char* name, StringWrapper& result, char* _typename, bool bCheckDerives)
{
	if (object == nullptr || name == nullptr) return;
	const rttr::type _type = type::get_by_name(_typename);
	ISSilentObject* _object = (ISSilentObject*)(object);
	if (bCheckDerives)
	{
		auto var = (ISSilentObject*)(object);
		(rttr::type&)_type = (type::get_by_name(var->GetSClassName()));
	}
	std::string _name(name);
	rttr::variant var(_object);
	auto _prop = _type.get_property(_name);
	std::string* res = new std::string(_prop.get_value(var).to_string().c_str());
	result.data = (char*)res->c_str();
}

SAKURACORE_API void __stdcall SetStringProp(void* object, char* name, char* val, char* _typename /*= (char*)"SakuraSceneNode"*/, bool bCheckDerives)
{
	if (object == nullptr || name == nullptr) return;
	if (val == nullptr) val = (char*)("Nil");
	const rttr::type _type = type::get_by_name(_typename);
	ISSilentObject* _object = (ISSilentObject*)(object);
	if (bCheckDerives)
	{
		auto var = (ISSilentObject*)(object);
		(rttr::type&)_type = (type::get_by_name(var->GetSClassName()));
	}
	std::string _name(name);
	rttr::variant var(_object);
	std::string _val(val);
	_type.set_property_value(_name, var, _val);
}

SAKURACORE_API void __stdcall GetIntProp(void* object, char* name, int& val, char* _typename /*= (char*)"SakuraSceneNode"*/, bool bCheckDerives)
{
	if (object == nullptr || name == nullptr) return;
	const rttr::type _type = type::get_by_name(_typename);
	ISSilentObject* _object = (ISSilentObject*)(object);
	if (bCheckDerives)
	{
		auto var = (ISSilentObject*)(object);
		(rttr::type&)_type = (type::get_by_name(var->GetSClassName()));
	}
	std::string _name(name);
	rttr::variant var(_object);
	auto _prop = _type.get_property(_name);
	val = _prop.get_value(var).to_int32();
}

SAKURACORE_API void __stdcall SetIntProp(void* object, char* name, int val, char* _typename /*= (char*)"SakuraSceneNode"*/, bool bCheckDerives /*= true*/)
{
	if (object == nullptr || name == nullptr) return;
	const rttr::type _type = type::get_by_name(_typename);
	ISSilentObject* _object = (ISSilentObject*)(object);
	if (bCheckDerives)
	{
		auto var = (ISSilentObject*)(object);
		(rttr::type&)_type = (type::get_by_name(var->GetSClassName()));
	}
	std::string _name(name);
	rttr::variant var(_object);
	_type.set_property_value(_name, var, val);
}

SAKURACORE_API void __stdcall GetUintProp(void* object, char* name, UINT& val, char* _typename /*= (char*)"SakuraSceneNode"*/, bool bCheckDerives /*= true*/)
{
	if (object == nullptr || name == nullptr) return;
	const rttr::type _type = type::get_by_name(_typename);
	ISSilentObject* _object = (ISSilentObject*)(object);
	if (bCheckDerives)
	{
		auto var = (ISSilentObject*)(object);
		(rttr::type&)_type = (type::get_by_name(var->GetSClassName()));
	}
	std::string _name(name);
	rttr::variant var(_object);
	auto _prop = _type.get_property(_name);
	val = _prop.get_value(var).to_uint32();
}

SAKURACORE_API void __stdcall SetUintProp(void* object, char* name, UINT val, char* _typename /*= (char*)"SakuraSceneNode"*/, bool bCheckDerives /*= true*/)
{
	if (object == nullptr || name == nullptr) return;
	const rttr::type _type = type::get_by_name(_typename);
	ISSilentObject* _object = (ISSilentObject*)(object);
	if (bCheckDerives)
	{
		auto var = (ISSilentObject*)(object);
		(rttr::type&)_type = (type::get_by_name(var->GetSClassName()));
	}
	std::string _name(name);
	rttr::variant var(_object);
	_type.set_property_value(_name, var, val);
}

SAKURACORE_API void __stdcall GetFloatProp(void* object, char* name, float& val, char* _typename /*= (char*)"SakuraSceneNode"*/, bool bCheckDerives /*= true*/)
{
	if (object == nullptr || name == nullptr) return;
	const rttr::type _type = type::get_by_name(_typename);
	ISSilentObject* _object = (ISSilentObject*)(object);
	if (bCheckDerives)
	{
		auto var = (ISSilentObject*)(object);
		(rttr::type&)_type = (type::get_by_name(var->GetSClassName()));
	}
	std::string _name(name);
	rttr::variant var(_object);
	auto _prop = _type.get_property(_name);
	val = _prop.get_value(var).to_float();
}

SAKURACORE_API void __stdcall SetFloatProp(void* object, char* name, float val, char* _typename /*= (char*)"SakuraSceneNode"*/, bool bCheckDerives /*= true*/)
{
	if (object == nullptr || name == nullptr) return;
	const rttr::type _type = type::get_by_name(_typename);
	ISSilentObject* _object = (ISSilentObject*)(object);
	if (bCheckDerives)
	{
		auto var = (ISSilentObject*)(object);
		(rttr::type&)_type = (type::get_by_name(var->GetSClassName()));
	}
	std::string _name(name);
	rttr::variant var(_object);
	_type.set_property_value(_name, var, val);
}

SAKURACORE_API void __stdcall GetEnumProp(void* node, char* name, StringWrapper& valToString, char* _typename /*= (char*)"SakuraSceneNode"*/, bool bCheckDerives /*= true*/)
{
	// To do...
}

SAKURACORE_API void __stdcall SetEnumProp(void* node, char* name, char* valToString, char* _typename /*= (char*)"SakuraSceneNode"*/, bool bCheckDerives /*= true*/)
{
	// To do...
}

SAKURACORE_API void __stdcall GetSVectorProp(void* object, char* name, SVectorWrapper& vecVal, char* _typename /*= (char*)"SakuraSceneNode"*/, bool bCheckDerives /*= true*/)
{
	if (object == nullptr || name == nullptr) return;
	const rttr::type _type = type::get_by_name(_typename);
	ISSilentObject* _object = (ISSilentObject*)(object);
	if (bCheckDerives)
	{
		auto var = (ISSilentObject*)(object);
		(rttr::type&)_type = (type::get_by_name(var->GetSClassName()));
	}
	std::string _name(name);
	rttr::variant var(_object);
	auto _prop = _type.get_property(_name);
	SakuraMath::SVector vec = _prop.get_value(var).convert<SakuraMath::SVector>();
	vecVal = *((SVectorWrapper*)(&vec));
}

SAKURACORE_API void __stdcall SetSVectorProp(void* object, char* name, SVectorWrapper vecVal, char* _typename /*= (char*)"SakuraSceneNode"*/, bool bCheckDerives /*= true*/)
{
	if (object == nullptr || name == nullptr) return;
	const rttr::type _type = type::get_by_name(_typename);
	ISSilentObject* _object = (ISSilentObject*)(object);
	if (bCheckDerives)
	{
		auto var = (ISSilentObject*)(object);
		(rttr::type&)_type = (type::get_by_name(var->GetSClassName()));
	}
	std::string _name(name);
	rttr::variant var(_object);
	SakuraMath::SVector* vec = (SakuraMath::SVector*)&vecVal;
	_type.set_property_value(_name, var, *vec);
}

SAKURACORE_API void __stdcall GetSubmemberProp(void* object, char* name, void*& result, char* _typename /*= (char*)"SakuraSceneNode"*/, bool bCheckDerives /*= true*/)
{
	if (object == nullptr || name == nullptr) return;
	const rttr::type _type = type::get_by_name(_typename);
	ISSilentObject* _object = (ISSilentObject*)(object);
	if (bCheckDerives)
	{
		auto var = (ISSilentObject*)(object);
		(rttr::type&)_type = (type::get_by_name(var->GetSClassName()));
	}
	std::string _name(name);
	rttr::variant var(_object);
	auto _prop = _type.get_property(_name);
	auto val = _prop.get_value(var);
	if (!val.get_type().is_derived_from<ISSilentObject>())
	{
		result = nullptr;
	}
	else
		result = val.convert<ISSilentObject*>();
}

SAKURACORE_API void __stdcall GetObjectProperties(void* object, SPropertyDataArrayWrapper& result,
	char* _typename /*= (char*)"SakuraSceneNode"*/, bool bCheckDerives /*= true*/)
{
	const rttr::type& _type = type::get_by_name(_typename);
	if (bCheckDerives)
	{
		auto var = (ISSilentObject*)(object);
		(rttr::type&)_type = (type::get_by_name(var->GetSClassName()));
	}
	auto props = _type.get_properties();
	result.num += props.size();
	result.wrappers = new SPropertyDataWrapper*[result.num];
	size_t i = 0;
	for (auto& prop : props)
	{
		SPropertyDataWrapper* wrapper = new SPropertyDataWrapper();
		std::string* res = nullptr;
		std::string&& _t = prop.get_name().to_string();
		_t += '\0';
		res = &_t;
		wrapper->PropName = new char[res->size()];
		wrapper->PropDescription = new char[res->size()];
		memcpy(wrapper->PropName, res->c_str(), res->size());
		memcpy(wrapper->PropDescription, res->c_str(), res->size());

		_t = prop.get_type().get_name().to_string();
		_t += '\0';
		res = &_t;
		wrapper->PropType = new char[res->size()];
		memcpy(wrapper->PropType, res->c_str(), res->size());

		_t = _type.get_name().to_string();
		_t += '\0';
		res = &_t;
		wrapper->SourceType = new char[res->size()];
		memcpy(wrapper->SourceType, res->c_str(), res->size());

		result.wrappers[i] = wrapper;
		i++;
	}
}
#pragma endregion 