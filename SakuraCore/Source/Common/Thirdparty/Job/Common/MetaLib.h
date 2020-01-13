#pragma once
#include <unordered_map>
typedef unsigned int UINT;
template <typename T, UINT size>
class Storage
{
	alignas(T) char c[size * sizeof(T)];
};
template <typename T>
class Storage<T, 0>
{};

template <typename... Args>
class AlignedTuple;
template<>
class AlignedTuple<>
{
public:
	AlignedTuple(size_t* ptr, int count) {}
	AlignedTuple(size_t* ptr) {}
};
template <typename T, typename... Args>
class AlignedTuple<T, Args...> : public AlignedTuple<Args...>
{
public:
	Storage<T, 1> value;
	AlignedTuple(size_t* ptr, int count) :
		AlignedTuple<Args...>(ptr, count - 1)
	{
		count -= 1;
		if (count < 0) return;
		AlignedTuple<T, Args...>* classPtr = nullptr;
		ptr[count] = (size_t)&classPtr->value;
	}
	AlignedTuple(size_t* ptr) :
		AlignedTuple<Args...>(ptr, sizeof...(Args))
	{
		int count = sizeof...(Args);
		if (count < 0) return;
		AlignedTuple<T, Args...>* classPtr = nullptr;
		ptr[count] = (size_t)&classPtr->value;
	}
};

template <typename F, unsigned int count>
struct LoopClass
{
	static void Do(F&& f)
	{
		LoopClass<F, count - 1>::Do(std::move(f));
		f(count);
	}
};

template <typename F>
struct LoopClass<F, 0>
{
	static void Do(F&& f)
	{
		f(0);
	}
};

template <typename F, unsigned int count>
void InnerLoop(F& function)
{
	LoopClass<F, count - 1>::Do(std::move(function));
}

template <typename F, unsigned int count>
void InnerLoop(F&& function)
{
	LoopClass<F, count - 1>::Do(std::move(function));
}
template <typename K, typename V>
class Dictionary
{
public:
	struct KVPair
	{
		K key;
		V value;
	};
	std::unordered_map<K, UINT> keyDicts;
	std::vector<KVPair> values;
	void Reserve(UINT capacity);
	V* operator[](K& key);

	void Add(K& key, V& value);
	void Remove(K& key);

	void Clear();
};
template <typename K, typename V>
void Dictionary<K,V>::Reserve(UINT capacity)
{
	keyDicts.reserve(capacity);
	values.reserve(capacity);
}
template <typename K, typename V>
V* Dictionary<K,V>::operator[](K& key)
{
	auto&& ite = keyDicts.find(key);
	if (ite == keyDicts.end()) return nullptr;
	return &values[ite->second].value;
}
template <typename K, typename V>
void Dictionary<K, V>::Add(K& key, V& value)
{
	keyDicts.insert_or_assign(key, std::move(values.size()));
	values.push_back({ std::move(key), std::move(value) });
}
template <typename K, typename V>
void Dictionary<K, V>::Remove(K& key)
{
	auto&& ite = keyDicts.find(key);
	if (ite == keyDicts.end()) return;
	KVPair& p = values[ite->second];
	p = values[values.size() - 1];
	keyDicts[p.key] = ite->second;
	values.erase(values.end() - 1);
	keyDicts.erase(ite->first);
}
template <typename K, typename V>
void Dictionary<K, V>::Clear()
{
	keyDicts.clear();
	values.clear();
}