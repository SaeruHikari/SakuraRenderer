#pragma once
#include <vector>
#include "JobHandle.h"
#include "../Common/Pool.h"
class JobSystem;
class JobThreadRunnable;
class JobBucket
{
	friend class JobSystem;
	friend class JobNode;
	friend class JobHandle;
	friend class JobThreadRunnable;
public:
	JobBucket() = default;
private:
	static ConcurrentPool<JobNode> jobNodePool;
	std::vector<JobNode*> jobNodesVec;
public:
	template <typename Func>
	JobHandle GetTask(Func&& func)
	{
		JobNode* node = jobNodePool.New();
		jobNodesVec.emplace_back(node);
		node->Create<Func>(std::forward<Func>(func));
		JobHandle retValue(node);
		return retValue;
	}
	template <typename Func>
	JobHandle GetTask(Func& func)
	{
		JobNode* node = jobNodePool.New();
		jobNodesVec.emplace_back(node);
		node->Create<Func>(std::forward<Func>(func));
		JobHandle retValue(node);
		return retValue;
	}
};