#pragma once
#include "JobNode.h"
class JobBucket;
class JobHandle
{
	friend class JobBucket;
private:
	JobNode* node;
	JobHandle(JobNode* otherNode) : node(otherNode) {};
public:
	JobHandle();
	JobHandle(const JobHandle& other);
	JobHandle& operator=(const JobHandle& other);
	void Precede(JobHandle& other);
};