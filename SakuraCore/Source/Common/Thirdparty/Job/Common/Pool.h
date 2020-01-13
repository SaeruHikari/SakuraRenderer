#pragma once
#include <vector>
#include <atomic>
#include <mutex>
#include "MetaLib.h"

template <typename T>
class Pool
{
private:
	std::vector<T*> allPtrs;
	std::vector<void*> allocatedPtrs;
	int capacity;
	void AllocateMemory()
	{
		using StorageT = Storage<T, 1>;
		StorageT* ptr = reinterpret_cast<StorageT*>(malloc(sizeof(StorageT) * capacity));
		for (int i = 0; i < capacity; ++i)
		{
			allPtrs.push_back(reinterpret_cast<T*>(ptr + i));
		}
		allocatedPtrs.push_back(ptr);
	}
public:
	Pool(int capa) : capacity(capa)
	{
		allPtrs.reserve(capa);
		allocatedPtrs.reserve(10);
		AllocateMemory();
	}
	
	template <typename... Args>
	T* New(Args... args)
	{
		if (allPtrs.size() <= 0)
			AllocateMemory();
		T* value = allPtrs[allPtrs.size() - 1];
		allPtrs.erase(allPtrs.end() - 1);
		new (value)T(args...);
		return value;
	}

	void Delete(T* ptr)
	{
		ptr->~T();
		allPtrs.push_back(ptr);
	}

	~Pool()
	{
		for (int i = 0; i < allocatedPtrs.size(); ++i)
		{
			free(allocatedPtrs[i]);
		}
	}
};

template <typename T>
class ConcurrentPool
{
private:
	typedef Storage<T, 1> StorageT;
	struct Array
	{
		StorageT** objs;
		std::atomic<int64_t> count;
		int64_t capacity;
	};

	Array unusedObjects[2];
	std::mutex mtx;
	bool objectSwitcher = true;
public:
	void UpdateSwitcher()
	{
		if (unusedObjects[objectSwitcher].count < 0) unusedObjects[objectSwitcher].count = 0;
		objectSwitcher = !objectSwitcher;
	}

	void Delete(T* targetPtr)
	{
		targetPtr->~T();
		Array* arr = unusedObjects + !objectSwitcher;
		int64_t currentCount = arr->count++;
		if (currentCount >= arr->capacity)
		{
			std::lock_guard<std::mutex> lck(mtx);
			//			lock
			{
				if (currentCount >= arr->capacity)
				{
					int64_t newCapacity = arr->capacity * 2;
					StorageT** newArray = new StorageT*[newCapacity];
					memcpy(newArray, arr->objs, sizeof(StorageT*) * arr->capacity);
					delete arr->objs;
					arr->objs = newArray;
					arr->capacity = newCapacity;
				}
			}
		}
		arr->objs[currentCount] = (StorageT*)targetPtr;
	}
	template <typename ... Args>
	T* New(Args... args)
	{
		Array* arr = unusedObjects + objectSwitcher;
		int64_t currentCount = --arr->count;
		T* t;
		if (currentCount >= 0)
		{
			t = (T*)arr->objs[currentCount];

		}
		else
		{
			t = (T*)malloc(sizeof(StorageT));
		}
		new (t)T(args...);
		return t;
	}

	ConcurrentPool(unsigned int initCapacity)
	{
		if (initCapacity < 3) initCapacity = 3;
		unusedObjects[0].objs = new StorageT*[initCapacity];
		unusedObjects[0].capacity = initCapacity;
		unusedObjects[0].count = initCapacity / 2;
		for (unsigned int i = 0; i < unusedObjects[0].count; ++i)
		{
			unusedObjects[0].objs[i] = (StorageT*)malloc(sizeof(StorageT));
		}
		unusedObjects[1].objs = new StorageT*[initCapacity];
		unusedObjects[1].capacity = initCapacity;
		unusedObjects[1].count = initCapacity / 2;
		for (unsigned int i = 0; i < unusedObjects[1].count; ++i)
		{
			unusedObjects[1].objs[i] = (StorageT*)malloc(sizeof(StorageT));
		}
	}
	~ConcurrentPool()
	{
		for (int64_t i = 0; i < unusedObjects[0].count; ++i)
		{
			delete unusedObjects[0].objs[i];

		}
		delete unusedObjects[0].objs;
		for (int64_t i = 0; i < unusedObjects[1].count; ++i)
		{
			delete unusedObjects[1].objs[i];

		}
		delete unusedObjects[1].objs;
	}
};
