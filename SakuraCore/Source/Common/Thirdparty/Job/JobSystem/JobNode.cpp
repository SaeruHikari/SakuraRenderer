#include "JobNode.h"
std::mutex JobNode::threadMtx;
JobNode::VectorPool JobNode::vectorPool(100);
JobNode::~JobNode()
{
	if (destructorFunc != nullptr)
		destructorFunc(ptr);
	dependingEvent->clear();
	vectorPool.Delete(dependingEvent);
}

JobNode* JobNode::Execute(ConcurrentQueue<JobNode*>& taskList, std::condition_variable& cv)
{
	executeFunc(ptr);
	std::vector<JobNode*>::iterator ite = dependingEvent->begin();
	JobNode* nextNode = nullptr;
	while(ite != dependingEvent->end())
	{
		JobNode* node = *ite;
		unsigned int dependingCount = --node->targetDepending;
		if (dependingCount == 0)
		{
			nextNode = node;
			++ite;
			break;
		}
		++ite;
	}
	for (; ite != dependingEvent->end(); ++ite)
	{
		JobNode* node = *ite;
		unsigned int dependingCount = --node->targetDepending;
		if (dependingCount == 0)
		{
			taskList.Push(node);
			{
				std::lock_guard<std::mutex> lck(threadMtx);
				cv.notify_one();
			}
		}
	}
	return nextNode;
}

void JobNode::Precede(JobNode* depending)
{
	depending->targetDepending++;
	dependingEvent->emplace_back(depending);
}

void JobNode::VectorPool::UpdateSwitcher()
{
	if (unusedObjects[objectSwitcher].count < 0) unusedObjects[objectSwitcher].count = 0;
	objectSwitcher = !objectSwitcher;
}

void JobNode::VectorPool::Delete(std::vector<JobNode*>* targetPtr)
{
	Array* arr = unusedObjects + !objectSwitcher;
	int64_t currentCount = arr->count++;
	if (currentCount >= arr->capacity)
	{
		std::lock_guard<std::mutex> lck(mtx);
		//			lock
		if (currentCount >= arr->capacity)
		{
			int64_t newCapacity = arr->capacity * 2;
			std::vector<JobNode*>** newArray = new std::vector<JobNode*>*[newCapacity];
			memcpy(newArray, arr->objs, sizeof(std::vector<JobNode*>*) * arr->capacity);
			delete arr->objs;
			arr->objs = newArray;
			arr->capacity = newCapacity;
		}
	}
	arr->objs[currentCount] = targetPtr;
}

std::vector<JobNode*>* JobNode::VectorPool::New()
{
	Array* arr = unusedObjects + objectSwitcher;
	int64_t currentCount = --arr->count;
	std::vector<JobNode*>* t;
	if (currentCount >= 0)
	{
		t = (std::vector<JobNode*>*)arr->objs[currentCount];

	}
	else
	{
		t = new std::vector<JobNode*>;
		t->reserve(20);
	}

	return t;
}

JobNode::VectorPool::VectorPool(unsigned int initCapacity)
{
	if (initCapacity < 3) initCapacity = 3;
	unusedObjects[0].objs = new std::vector<JobNode*>*[initCapacity];
	unusedObjects[0].capacity = initCapacity;
	unusedObjects[0].count = initCapacity / 2;
	for (unsigned int i = 0; i < unusedObjects[0].count; ++i)
	{
		unusedObjects[0].objs[i] = new std::vector<JobNode*>;// (StorageT*)malloc(sizeof(StorageT));
		unusedObjects[0].objs[i]->reserve(20);
	}
	unusedObjects[1].objs = new std::vector<JobNode*>*[initCapacity];
	unusedObjects[1].capacity = initCapacity;
	unusedObjects[1].count = initCapacity / 2;
	for (unsigned int i = 0; i < unusedObjects[1].count; ++i)
	{
		unusedObjects[1].objs[i] = new std::vector<JobNode*>;
		unusedObjects[1].objs[i]->reserve(20);
	}
}
JobNode::VectorPool::~VectorPool()
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