#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>

class CommandBuffer
{
private:
	ID3D12CommandQueue* graphicsCommandQueue;
	ID3D12CommandQueue* computeCommandQueue;
	std::vector<ID3D12GraphicsCommandList*> graphicsCmdLists;
	struct Fence
	{
		ID3D12Fence* fence;
		UINT64 frameIndex;
	};
	struct ExecuteCommand
	{
		UINT start;
		UINT count;
	};
	struct InnerCommand
	{
		enum CommandType
		{
			CommandType_ExecuteGraphics,
			CommandType_ExecuteCompute,
			CommandType_Wait,
			CommandType_Signal
		};
		CommandType type;
		union
		{
			UINT executeCount;
			Fence waitFence;
			Fence signalFence;
		};
	};
	std::vector<InnerCommand> executeCommands;
	enum ExecuteType
	{
		NOT_EXECUTING,
		EXECUTING_GRAPHICS,
		EXECUTING_COMPUTE
	};
	ExecuteType executeChoose = NOT_EXECUTING;
	UINT executeCount = 0;
	void ChangeExecuteState(ExecuteType state);
public:
	ID3D12CommandQueue* GetGraphicsQueue() const {
		return graphicsCommandQueue;
	}
	ID3D12CommandQueue* GetComputeQueue() const {
		return computeCommandQueue;
	}
	void Wait(ID3D12Fence* computeFence, UINT currentFrame);
	void Signal(ID3D12Fence* computeFence, UINT currentFrame);
	void ExecuteGraphicsCommandList(ID3D12GraphicsCommandList* commandList);
	void ExecuteComputeCommandList(ID3D12GraphicsCommandList* commandList);
	void Submit();
	void Clear();
	CommandBuffer(
		ID3D12CommandQueue* graphicsCommandQueue,
		ID3D12CommandQueue* computeCommandQueue
	);
};