/*****************************************************************************************
							 Copyrights   SaeruHikari
CreateDate:							2019.10.21
Description:	          D3D12 Renderer Graphics Manager version v0.
Details:
*******************************************************************************************/
#pragma once
#include "SakuraD3D12GraphicsManager.hpp"
#include <memory>
#include "../../../GraphicTypes/D3D12/SD3DCamera.h"
#include "Framework/GraphicTypes/D3D12/SDx12RenderTarget.hpp"
#include "Resource/SDxResourceManager.h"
#include "../../../GraphicTypes/Debugger/DirectX/SDx12ImGuiDebugger.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace HikaD3DUtils;

namespace SGraphics
{
	class SsaoPass;
	class SGBufferPass;
	class SkySpherePass;
	class SHDR2CubeMapPass;
	class SCubeMapConvPass;
	class SBrdfLutPass;
	class STaaPass;
	class SMotionVectorPass;
	class SDeferredPass;
	class SGBufferDebugPass;

	class SDx12RenderTarget2D;


	class SakuraScene;

	namespace SPasses
	{
		enum EPasses
		{
			E_GBuffer = 0,
			E_SSAO = 1,
			E_Deferred = 2,
			E_GBufferDebug = 3,
			E_Count = 4
		};
	}

	class SDxRendererGM : public SakuraD3D12GraphicsManager
	{
	public:
		SDxRendererGM()
			:SakuraD3D12GraphicsManager() {};
		SDxRendererGM(const SDxRendererGM& rhs) = delete;
		SDxRendererGM& operator=(const SDxRendererGM& rhs) = delete;
		~SDxRendererGM() {
			if (md3dDevice != nullptr)
				FlushCommandQueue();
		}

	public:
		inline auto GetDeferredSrvCPU(int offset)
		{
			auto srvCPU = CD3DX12_CPU_DESCRIPTOR_HANDLE(GetResourceManager()->GetOrAllocDescriptorHeap(SRVs::DeferredSrvName)->GetCPUtDescriptorHandle(0));
			srvCPU.Offset(offset, CbvSrvUavDescriptorSize());
			return srvCPU;
		}
		inline auto GetDeferredSrvGPU(int offset)
		{
			auto srvGPU = CD3DX12_GPU_DESCRIPTOR_HANDLE(GetResourceManager()->GetOrAllocDescriptorHeap(SRVs::DeferredSrvName)->GetGPUtDescriptorHandle(0));
			srvGPU.Offset(offset, CbvSrvUavDescriptorSize());
			return srvGPU;
		}
	public:
		virtual bool Initialize() override;
		virtual void Draw() override;
		virtual void Finalize() override;
		virtual void Tick(double deltaTime) override;

		virtual void OnMouseDown(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y) override;
		virtual void OnMouseMove(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y) override;
		virtual void OnMouseUp(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y) override;
		virtual void OnKeyDown(double deltaTime) override;

		float* CaputureBuffer(
			ID3D12Device* device,
			ID3D12GraphicsCommandList* cmdList,
			ID3D12Resource* resourceToRead,
			size_t outChannels = 4);
	
	protected:
		virtual void OnResize(UINT Width, UINT Height) override;

		void UpdateObjectCBs();
		void UpdateMaterialCBs();
		void UpdateMainPassCB();
		void UpdateSsaoPassCB();

		void LoadTextures();
		// Create Descriptor Heaps for RenderTargetView & Depth/Stencil View
		virtual void CreateRtvAndDsvDescriptorHeaps() override;
		void BuildDescriptorHeaps();
		void BindPassResources();
		void BuildGeometry();
		void BuildGeneratedMeshes();
		void BuildFrameResources();

		// TO DELETE!
		// Build Debug Material
		void BuildMaterials();
		// Build Debug Render Items
		void BuildRenderItems();

		void InitFGFromJson(std::string jsonPath);

	private:
		SFrameResource* mCurrFrameResource = nullptr;
		int mCurrFrameResourceIndex = 0;
		UINT mCbvSrvDescriptorSize = 0;
	public:
		int CBIndex = 0;
		std::unique_ptr<SDx12ImGuiDebugger> mImGuiDebugger;

	private:
		// loose, need refactoring
		SPassConstants mMainPassCB;
		SsaoConstants mSsaoCB;
		SD3DCamera mCubeMapCamera[6];
		POINT mLastMousePos;
	protected:
		void BuildCubeFaceCamera(float x, float y, float z);
	protected:
		// Rtv indices
		int GBufferResourceSrv = 4;
		inline static const int GBufferRTNum = 4;// 1 : SSAO
		inline static const int LUTNum = 1;
		inline static const int SkyCubeMips = 8;
		inline static const int SkyCubePrefilters = 5;
		// 1 : Conv
		inline static const int SkyCubeConvFilterNum = SkyCubePrefilters + 1;
		inline static const int GBufferSrvStartAt = SkyCubeMips + SkyCubePrefilters + 1 + LUTNum;
		// Screen Efx phase Rtvs
		#define MotionVectorRtvStart 0
		inline static const int MotionVectorRtvNum = 1;
		#define TAARtvsStart  MotionVectorRtvNum + MotionVectorRtvStart
		inline static const int TAARtvsNum = 3;

	protected:
		struct SRVs
		{
			inline static const std::string GBufferSrvName = "GBufferSrv";
			inline static const std::string CaptureSrvName = "CaptureSrv";
			inline static const std::string DeferredSrvName = "DeferredSrv";
			inline static const std::string ScreenEfxSrvName = "ScreenEfxSrv";
			inline static const std::string ImGuiSrvName = "ImGuiSrv";
		};
		struct RTVs
		{
			inline static const std::string DefaultRtvName = "DefaultRtv";
			inline static const std::string DeferredRtvName = "DeferredRtv";
			inline static const std::string CaptureRtvName = "CaptureRtv";
			inline static const std::string ScreenEfxRtvName = "ScreenEfxRtv";
		};
		struct Textures
		{
			inline static const std::vector<std::string> texNames =
			{
				"DiffTex",
				"RoughTex",
				"SpecTex",
				"NormalTex",
				"SkyCubeMap",
				"HDRTexture"
			};
			inline static const std::vector<std::wstring> texFilenames =
			{
				L"Resources/Textures/FlameThrower_ALB.dds",
				L"Resources/Textures/FlameThrower_RMA.dds",
				L"Resources/Textures/FlameThrower_RMA.dds",
				L"Resources/Textures/FlameThrower_NRM.dds",
				L"Resources/Textures/grasscube1024.dds",
				L"Resources/Textures/venice_sunset_4k.hdr"
			};
		};
		struct InitPasses
		{
			inline static const std::string BrdfLutPassName = "BrdfLutPass";
			inline static const std::string HDR2CubeMapPassName = "HDR2CubePass";
			inline static const std::string CubeMapConvPassName = "CubemapConvPass";
		};
		struct ConsistingPasses
		{
			inline static const std::string GBufferPassName = "GbufferPass";
			inline static const std::string MotionVectorPassName = "MotionVectorPass";
			inline static const std::string DeferredPassName = "DeferredPass";
			inline static const std::string TaaPassName = "TaaPass";
			inline static const std::string SsaoPassName = "SsaoPass";
			inline static const std::string SkySpherePassName = "SkySpherePass";
			inline static const std::string GBufferDebugPassName = "GBufferDebugPass";
		};
		struct Meshes
		{
			inline static const std::string GunPath = "Resources/Models/gun.fbx";
			inline static const std::string FlamePath = "Resources/Models/FlameThrower.fbx";
			inline static const std::string UrnPath = "Resources/Models/Urn.fbx";
			inline static const std::string SponzaPath = "Resources/Models/SM_Sponza.fbx";
			inline static const std::string GuitarPath = "Resources/Models/Guitar.fbx";
			inline static std::string CurrPath = SponzaPath;
		};
		struct RT2Ds
		{
			inline static const std::string MotionVectorRTName = "MotionVectorRT";
			inline static std::vector<std::string> TAARTNames;
			inline static std::vector<std::string> GBufferRTNames;
			inline static std::string BrdfLutRTName = "BrdfLutRT";
			RT2Ds()
			{
				TAARTNames.resize(TAARtvsNum);
				GBufferRTNames.resize(GBufferRTNum);
				for (size_t i = 0; i < TAARtvsNum; i++)
				{
					std::string NameI = "TAARTName" + std::to_string(i);
					TAARTNames[i] = (NameI);
				}
				for (size_t i = 0; i < GBufferRTNum; i++)
				{
					std::string NameI = "GBufferRTName" + std::to_string(i);
					GBufferRTNames[i] = NameI;
				}
			}
		};
		inline static RT2Ds rt2ds{};
		static struct RT3Ds
		{
			inline static std::vector<std::string> SkyCubeRTNames;
			inline static std::vector<std::string> ConvAndPrefilterNames;
			RT3Ds()
			{
				SkyCubeRTNames.resize(SkyCubeMips);
				ConvAndPrefilterNames.resize(SkyCubeConvFilterNum);
				for (size_t i = 0; i < SkyCubeMips; i++)
				{
					std::string NameI = "SkyCubeRTName" + std::to_string(i);
					SkyCubeRTNames[i] = (NameI);
				}
				for (int j = 0; j < SkyCubeConvFilterNum; j++)
				{
					std::string NameI = "ConvAndPrefilterRTName" + std::to_string(j);
					ConvAndPrefilterNames[j] = (NameI);
				}
			}
		};
		inline static RT3Ds rt3ds{};

		// Helper Containers
		std::vector<std::string> GBufferPassResources;
		std::vector<ID3D12Resource*> mConvAndPrefilterSkyCubeResource[SkyCubeConvFilterNum];
		std::vector<ID3D12Resource*> mSkyCubeResource;

		SDx12RenderTarget2D* mMotionVectorRT;
		SDx12RenderTarget2D* mBrdfLutRT2D;
		SDx12RenderTarget2D** GBufferRTs;
		SDx12RenderTarget2D** mTaaRTs;
		// SRenderTargetCubeMultiLevels<5>
		SDx12RenderTargetCube* mConvAndPrefilterCubeRTs[SkyCubeConvFilterNum];
		SDx12RenderTargetCube* mSkyCubeRT[SkyCubeMips];
		//TAA
		inline static const int TAA_SAMPLE_COUNT = 8;
		inline static const float TAA_JITTER_DISTANCE = .5f;
	};

}
	