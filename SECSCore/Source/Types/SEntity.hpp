// SEntity is the Entity part of ECS. 
// Contains to size_t to describe index and version info.
#pragma once

namespace SECS
{
	class SWorld;
	struct SEntity
	{
		// Only entity manager can set Index and version value.
		friend class SEntityManager;
		friend class SWorld;
		friend class SChunk;
	public:
		inline size_t GetIndex()
		{
			return Index;
		}
		inline size_t GetGeneration()
		{
			return generation;
		}

		virtual ~SEntity() {};
	private:
		// Index of SEntity
	    size_t Index = -1;
		int generation = -1;
	};
}

