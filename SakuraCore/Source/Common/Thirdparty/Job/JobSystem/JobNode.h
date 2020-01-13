#pragma once
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "ConcurrentQueue.h"
#include "../Common/Pool.h"
#include <DirectXMath.h>
class JobHandle;
class JobThreadRunnable;
class JobBucket;

 

class JobNode
{
	friend class JobBucket;
	friend class JobSystem;
	friend class JobHandle;
	friend class JobThreadRunnable;
private:
	class VectorPool
	{
	private:
		struct Array
		{
			std::vector<JobNode*>** objs;
			std::atomic<int64_t> count;
			int64_t capacity;
		};

		Array unusedObjects[2];
		std::mutex mtx;
		bool objectSwitcher = true;
	public:
		void UpdateSwitcher();
		void Delete(std::vector<JobNode*>* targetPtr);

		std::vector<JobNode*>* New();

		VectorPool(unsigned int initCapacity);
		~VectorPool();
	};
	struct FuncStorage
	{
		alignas(__m128) char arr[sizeof(__m128) * 16];
	};
	static std::mutex threadMtx;
	static VectorPool vectorPool;
	std::atomic<unsigned int> targetDepending = 0;
	std::vector<JobNode*>* dependingEvent;
	FuncStorage stackArr;
	void* ptr = nullptr;
	void(*destructorFunc)(void*) = nullptr;
	void(*executeFunc)(void*);
	template <typename Func>
	void Create(Func&& func)
	{
		dependingEvent = vectorPool.New();
			using Storage = std::aligned_storage_t<sizeof(Func), alignof(Func)>;
		if (sizeof(Storage) >= sizeof(FuncStorage))	//Create in heap
		{
			assert(false);
			ptr = new Func(std::forward<Func>(func));
			destructorFunc = [](void* currPtr)->void
			{
				Func* fc = (Func*)currPtr;
				delete fc;
			};
		}
		else
		{
			ptr = &stackArr;
			new (ptr)Func(std::forward<Func>(func));
			destructorFunc = [](void* currPtr)->void
			{
				Func* fc = (Func*)currPtr;
				fc->~Func();
			};
		}
		executeFunc = [](void* currPtr)->void
		{
			Func* fc = (Func*)currPtr;
			(*fc)();
		};
	}
	JobNode* Execute(ConcurrentQueue<JobNode*>& taskList, std::condition_variable& cv);
	void Precede(JobNode* depending);
public:

	~JobNode();
};