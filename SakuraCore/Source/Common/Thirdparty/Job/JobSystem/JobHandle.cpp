#include "JobHandle.h"

JobHandle::JobHandle() : node(nullptr)
{

}


JobHandle::JobHandle(const JobHandle& other)
{
	node = other.node;
}

JobHandle& JobHandle::operator=(const JobHandle& other)
{
	node = other.node;
	return *this;
}

void JobHandle::Precede(JobHandle& other)
{
	if (node == nullptr || other.node == nullptr) return;
	node->Precede(other.node);
}