#include "JobSystem.h"
#include <thread>
#include "ConcurrentQueue.h"
#include <condition_variable>
#include <atomic>
ConcurrentQueue<JobNode*> executingNode(100);
std::vector<std::thread*> allThreads;
std::atomic<int> bucketMissionCount;
int currentBucketPos;
std::vector<JobBucket*> buckets;
std::condition_variable cv;
bool JobSystemInitialized(false);

std::mutex mainThreadWaitMutex;
std::condition_variable mainThreadWaitCV;
bool mainThreadFinished(true);

void JobSystem::UpdateNewBucket()
{
	if (currentBucketPos >= buckets.size())
	{
		mainThreadFinished = true;
		{
			std::lock_guard<std::mutex> lck(mainThreadWaitMutex);
			mainThreadWaitCV.notify_all();
		}
		return;
	}

	JobBucket* bucket = buckets[currentBucketPos];
	bucketMissionCount = bucket->jobNodesVec.size();
	executingNode.ResizeAndClear(bucket->jobNodesVec.size());
	for (int i = 0; i < bucket->jobNodesVec.size(); ++i)
	{
		JobNode* node = bucket->jobNodesVec[i];
		if (node->targetDepending <= 0)
		{
			executingNode.Push(node);

		}
	}
	bucket->jobNodesVec.clear();
	currentBucketPos++;
	UINT size = executingNode.GetSize();
	if (executingNode.GetSize() < mThreadCount) {
		for (int64_t i = 0; i < executingNode.GetSize(); ++i) {
			std::lock_guard<std::mutex> lck(JobNode::threadMtx);
			cv.notify_one();
		}
	}
	else
	{
		std::lock_guard<std::mutex> lck(JobNode::threadMtx);
		cv.notify_all();
	}
}

class JobThreadRunnable
{
public:
	JobSystem* sys;
	void operator()()
	{
		int value = (int)-1;
		while (JobSystemInitialized)
		{
			{
				std::unique_lock<std::mutex> lck(JobNode::threadMtx);
				cv.wait(lck);
			}
			JobNode* node = nullptr;
			while (executingNode.TryPop(&node))
			{
			START_LOOP:
				JobNode* nextNode = node->Execute(executingNode, cv);
				JobBucket::jobNodePool.Delete(node);
				value = bucketMissionCount.fetch_add(-1) - 1;
				if (nextNode != nullptr)
				{
					node = nextNode;
					goto START_LOOP;
				}
				if (value <= 0)
				{
					sys->UpdateNewBucket();
				}
			}

		}
	}
};

JobSystem::JobSystem(int threadCount)
{
	if (JobSystemInitialized) return;
	mThreadCount = threadCount;
	JobSystemInitialized = true;
	allThreads.resize(threadCount);

	for (int i = 0; i < threadCount; ++i)
	{
		JobThreadRunnable j;
		j.sys = this;
		allThreads[i] = new std::thread(j);
	}
}

JobSystem::~JobSystem()
{
	if (!JobSystemInitialized) return;
	JobSystemInitialized = false;
	{
		std::lock_guard<std::mutex> lck(JobNode::threadMtx);
		cv.notify_all();
	}
	for (int i = 0; i < allThreads.size(); ++i)
	{
		allThreads[i]->join();
		delete allThreads[i];
	}
}

void JobSystem::ExecuteBucket(JobBucket** bucket, int bucketCount)
{
	Wait();
	JobBucket::jobNodePool.UpdateSwitcher();
	JobNode::vectorPool.UpdateSwitcher();
	currentBucketPos = 0;
	buckets.resize(bucketCount);
	memcpy(buckets.data(), bucket, sizeof(JobBucket*) * bucketCount);
	mainThreadFinished = false;
	UpdateNewBucket();

}
void JobSystem::ExecuteBucket(JobBucket* bucket, int bucketCount)
{
	Wait();
	JobBucket::jobNodePool.UpdateSwitcher();
	JobNode::vectorPool.UpdateSwitcher();
	currentBucketPos = 0;
	buckets.resize(bucketCount);
	for (int i = 0; i < bucketCount; ++i)
	{
		buckets[i] = bucket + i;
	}
	mainThreadFinished = false;
	UpdateNewBucket();

}

void JobSystem::Wait()
{
	std::unique_lock<std::mutex> lck(mainThreadWaitMutex);
	while (!mainThreadFinished)
		mainThreadWaitCV.wait(lck);
}

