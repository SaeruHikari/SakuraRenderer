/*****************************************************************************************
								 Copyrights   SaeruHikari
CreateDate:								2019.12.17
Description:				 Sakura FrameGraph (DAG) class.
Details:              A handle of resource nodes with name and version.
*******************************************************************************************/
#pragma once
#include <string>

namespace SGraphics
{
	struct SFG_ResourceHandle
	{
		std::string name = "NULL";
		size_t version = 0;
	};
}