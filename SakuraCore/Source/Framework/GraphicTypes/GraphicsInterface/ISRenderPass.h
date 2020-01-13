#pragma once

class SCommandList;

namespace SGraphics
{
	struct ISRenderPass
	{
		virtual SCommandList* GetCmdList() = 0;
		virtual void ClearCmd() = 0;
	};
}