#include "CommandBuffer.h"

void CommandBuffer::ChangeExecuteState(ExecuteType state)
{
	if (state == executeChoose) return;
	InnerCommand cmd;
	switch (state)
	{
	case NOT_EXECUTING:
		if (executeChoose == EXECUTING_GRAPHICS)
		{
			cmd.type = InnerCommand::CommandType_ExecuteGraphics;
		}
		else//Executing compute
		{
			cmd.type = InnerCommand::CommandType_ExecuteCompute;
		}
		cmd.executeCount = executeCount;
		executeCount = 0;
		executeCommands.push_back(cmd);

		break;
	case EXECUTING_GRAPHICS:
		if (executeChoose == EXECUTING_COMPUTE)
		{
			cmd.type = InnerCommand::CommandType_ExecuteCompute;
			cmd.executeCount = executeCount;
			executeCount = 0;
			executeCommands.push_back(cmd);
		}
		break;
	case EXECUTING_COMPUTE:
		if (executeChoose == EXECUTING_GRAPHICS)
		{
			cmd.type = InnerCommand::CommandType_ExecuteGraphics;
			cmd.executeCount = executeCount;
			executeCount = 0;
			executeCommands.push_back(cmd);
		}
		break;
	}
	executeChoose = state;
}

void CommandBuffer::Clear()
{
	executeCommands.clear();
	graphicsCmdLists.clear();
	executeCount = 0;
	executeChoose = NOT_EXECUTING;
}

void CommandBuffer::Submit()
{
	ChangeExecuteState(NOT_EXECUTING);
	UINT graphicsIndex = 0;
	for (auto ite = executeCommands.begin(); ite != executeCommands.end(); ++ite)
	{
		switch (ite->type)
		{
		case InnerCommand::CommandType_ExecuteGraphics:
			graphicsCommandQueue->ExecuteCommandLists(ite->executeCount, (ID3D12CommandList**)(graphicsCmdLists.data() + graphicsIndex));
			graphicsIndex += ite->executeCount;
			break;
		case InnerCommand::CommandType_ExecuteCompute:
			computeCommandQueue->ExecuteCommandLists(ite->executeCount, (ID3D12CommandList**)(graphicsCmdLists.data() + graphicsIndex));
			graphicsIndex += ite->executeCount;
			break;
		case InnerCommand::CommandType_Wait:
			graphicsCommandQueue->Wait(ite->waitFence.fence, ite->waitFence.frameIndex);
			break;
		case InnerCommand::CommandType_Signal:
			computeCommandQueue->Signal(ite->signalFence.fence, ite->signalFence.frameIndex);
			break;
		}
	}
}

void CommandBuffer::Wait(ID3D12Fence* computeFence, UINT currentFrame)
{
	ChangeExecuteState(NOT_EXECUTING);
	InnerCommand cmd;
	cmd.type = InnerCommand::CommandType_Wait;
	cmd.waitFence.fence = computeFence;
	cmd.waitFence.frameIndex = currentFrame;
	executeCommands.push_back(cmd);
}
void CommandBuffer::Signal(ID3D12Fence* computeFence, UINT currentFrame)
{
	ChangeExecuteState(NOT_EXECUTING);
	InnerCommand cmd;
	cmd.type = InnerCommand::CommandType_Signal;
	cmd.signalFence.fence = computeFence;
	cmd.signalFence.frameIndex = currentFrame;
	executeCommands.push_back(cmd);
}
void CommandBuffer::ExecuteGraphicsCommandList(ID3D12GraphicsCommandList* commandList)
{
	ChangeExecuteState(EXECUTING_GRAPHICS);
	graphicsCmdLists.push_back(commandList);
	executeCount++;
}
void CommandBuffer::ExecuteComputeCommandList(ID3D12GraphicsCommandList* commandList)
{
	ChangeExecuteState(EXECUTING_COMPUTE);
	graphicsCmdLists.push_back(commandList);
	executeCount++;
}

CommandBuffer::CommandBuffer(
	ID3D12CommandQueue* graphicsCommandQueue,
	ID3D12CommandQueue* computeCommandQueue
) : graphicsCommandQueue(graphicsCommandQueue),
computeCommandQueue(computeCommandQueue)
{

}