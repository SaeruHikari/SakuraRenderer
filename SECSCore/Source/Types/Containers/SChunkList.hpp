/*****************************************************************************************
							 Copyrights   SaeruHikari
Description:				 SChunkList master class.
Details:		Chunk list is a container of chunks of the same architecture.
*******************************************************************************************/
#pragma once
#include <vector>

namespace SECS
{
	struct SChunk;
	typedef std::vector<SChunk*> SChunkList;
}
