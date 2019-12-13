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

	namespace SRenderLayers
	{
		enum ERenderLayer
		{
			E_Opaque = 0,
			E_ScreenQuad = 1,
			E_GBufferDebug = 2,
			E_SKY = 3,
			E_Cube = 4,
			E_Count = 5
		};
	}
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

	private:
		std::vector<std::unique_ptr<SFrameResource>> mFrameResources;
		SFrameResource* mCurrFrameResource = nullptr;
		int mCurrFrameResourceIndex = 0;
		UINT mCbvSrvDescriptorSize = 0;
	public:
		int CBIndex = 0;
		std::unordered_map<std::string, std::unique_ptr<Dx12MeshGeometry>> mGeometries;
		std::unordered_map<std::string, OpaqueMaterial*> mMaterials;

		// List of all the render items
		std::vector<std::unique_ptr<SDxRenderItem>> mAllRitems;
		// Render items divided by PSO
		std::vector<SDxRenderItem*> mRenderLayers[SRenderLayers::E_Count];
	private:
		// loose, need refactoring
		SPassConstants mMainPassCB;
		SsaoConstants mSsaoCB;
		SD3DCamera mCamera;
		SD3DCamera mCubeMapCamera[6];
		POINT mLastMousePos;
	protected:
		void BuildCubeFaceCamera(float x, float y, float z);
	protected:
		// Rtv indices
		int GBufferResourceSrv = 0;
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
		SDx12RenderTarget2D* mMotionVectorRT;
		SDx12RenderTarget2D* mBrdfLutRT2D;

		SDx12RenderTarget2D** GBufferRTs;
		SDx12RenderTarget2D** mTaaRTs;

		std::shared_ptr<SGBufferPass> mGbufferPass = nullptr;
		std::shared_ptr<SsaoPass> mSsaoPass = nullptr;
		std::shared_ptr<SkySpherePass> mDrawSkyPass = nullptr;
		std::shared_ptr<STaaPass> mTaaPass = nullptr;
		std::shared_ptr<SMotionVectorPass> mMotionVectorPass = nullptr;
		std::shared_ptr<SDeferredPass> mDeferredPass = nullptr;
		std::shared_ptr<SGBufferDebugPass> mGBufferDebugPass = nullptr;

		struct SRVs
		{
			inline static const std::string GBufferSrvName = "GBufferSrv";
			inline static const std::string CaptureSrvName = "CaptureSrv";
			inline static const std::string DeferredSrvName = "DeferredSrv";
			inline static const std::string ScreenEfxSrvName = "ScreenEfxSrv";
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
				L"Textures/FlameThrower_ALB.dds",
				L"Textures/FlameThrower_RMA.dds",
				L"Textures/FlameThrower_RMA.dds",
				L"Textures/FlameThrower_NRM.dds",
				L"Textures/grasscube1024.dds",
				L"Textures/venice_sunset_4k.hdr"
			};
		};
		struct Meshes
		{
			inline static const std::string GunPath = "Models/gun.fbx";
			inline static const std::string FlamePath = "Models/FlameThrower.fbx";
			inline static const std::string UrnPath = "Models/Urn.fbx";
			inline static std::string CurrPath = FlamePath;
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
		std::vector<ID3D12Resource*> mDeferredSrvResources;
		std::vector<ID3D12Resource*> mGBufferSrvResources;
		std::vector<ID3D12Resource*> mSsaoSrvResources;
		std::vector<ID3D12Resource*> mTaaResources;
		std::vector<ID3D12Resource*> mConvAndPrefilterSkyCubeResource[SkyCubeConvFilterNum];
		std::vector<ID3D12Resource*> mSkyCubeResource;

		// SRenderTargetCubeMultiLevels<5>
		SDx12RenderTargetCube* mConvAndPrefilterCubeRTs[SkyCubeConvFilterNum];
		SDx12RenderTargetCube* mSkyCubeRT[SkyCubeMips];

		//TAA
		inline static const int TAA_SAMPLE_COUNT = 8;
		inline static const float TAA_JITTER_DISTANCE = 1.f;
	};

}
	