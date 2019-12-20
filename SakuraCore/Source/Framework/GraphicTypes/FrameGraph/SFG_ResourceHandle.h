/*****************************************************************************************
								 Copyrights   SaeruHikari
CreateDate:								2019.12.17
Description:				 Sakura FrameGraph (DAG) class.
Details:              A handle of resource nodes with name and version.
*******************************************************************************************/
#pragma once
#include <string>
#include <type_traits>

using namespace SGraphics;

namespace SGraphics
{
	struct SFG_ResourceHandle
	{
		std::string name = "NULL";
		std::string writer = "NULL";

		__forceinline bool Is(const std::string& _name)
		{
			return name == _name;
		}
		bool operator==(const SFG_ResourceHandle& a) const
		{
			return (name + writer) == (a.name + a.writer);
		}
	};
}

namespace std
{
	template<>
	struct hash<SFG_ResourceHandle>
	{
	public:
		std::size_t operator()(const SFG_ResourceHandle& c) const
		{
			hash<std::string> h;
			return h(c.name + c.writer);
		}
	};
}