#pragma once
#include <mutex>
#include "JobBucket.h"
class JobBucket;
class JobThreadRunnable;
class JobSystem
{
	friend class JobThreadRunnable;
private:
	void UpdateNewBucket();
	int mThreadCount;
public:
	JobSystem(int threadCount);
	void ExecuteBucket(JobBucket** bucket, int bucketCount);
	void ExecuteBucket(JobBucket* bucket, int bucketCount);
	void Wait();
	~JobSystem();
};

