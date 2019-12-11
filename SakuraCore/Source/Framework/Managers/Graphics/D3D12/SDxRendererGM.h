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
		inline auto GetResourceManager()
		{
			return (SDxResourceManager*)(pGraphicsResourceManager.get());
		}
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
		inline auto GetScreenEfxSrvCPU(int offset)
		{
			auto srvCPU = CD3DX12_CPU_DESCRIPTOR_HANDLE(GetResourceManager()->GetOrAllocDescriptorHeap(SRVs::ScreenEfxSrvName)->GetCPUtDescriptorHandle(0));
			srvCPU.Offset(offset, CbvSrvUavDescriptorSize());
			return srvCPU;
		}
		inline auto GetScreenEfxSrvGPU(int offset)
		{
			auto srvGPU = CD3DX12_GPU_DESCRIPTOR_HANDLE(GetResourceManager()->GetOrAllocDescriptorHeap(SRVs::ScreenEfxSrvName)->GetGPUtDescriptorHandle(0));
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

		void AnimateMaterials();
		void UpdateObjectCBs();
		void UpdateMaterialCBs();
		void UpdateMainPassCB();
		void UpdateSsaoPassCB();

		void LoadTextures();
		void BuildGBufferPassRootSignature();
		void BuildDeferredPassRootSignature();

		// Create Descriptor Heaps for RenderTargetView & Depth/Stencil View
		virtual void CreateRtvAndDsvDescriptorHeaps() override;

		void BuildDescriptorHeaps();
		void BuildGBufferPassDescriptorHeaps();
		void BuildDeferredShadingPassDescriptorHeaps();
		void BindPassResources();

		void BuildShaderAndInputLayout();
		void BuildGBufferPassShaderAndInputLayout();
		void BuildGBufferDebugShaderAndInputLayout();

		void BuildDeferredShadingPassShaderAndInputLayout();
		void BuildGeneratedMeshes();
		void BuildGeometry();

		void BuildPSOs();
		void BuildFrameResources();
		void BuildMaterials();

		void BuildRenderItems();

		void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<SDxRenderItem*>& ritems);

	private:
		std::vector<std::unique_ptr<SFrameResource>> mFrameResources;
		SFrameResource* mCurrFrameResource = nullptr;
		int mCurrFrameResourceIndex = 0;
		UINT mCbvSrvDescriptorSize = 0;
		std::unordered_map<std::string, ComPtr<ID3D12RootSignature>> mRootSignatures;

	public:
		int CBIndex = 0;
		std::unordered_map<std::string, std::unique_ptr<Dx12MeshGeometry>> mGeometries;
		std::unordered_map<std::string, OpaqueMaterial*> mMaterials;
		std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
		std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
		std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayouts[SPasses::E_Count];

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
		int GBufferMaterials = 0;
		inline static const int GBufferRTNum = 4;// 1 : SSAO
		inline static const int LUTNum = 1;
		inline static const int SkyCubeMips = 8;
		inline static const int SkyCubePrefilters = 5;
		inline static const int SkyCubeConvNum = 1;
		inline static const int SkyCubeConvFilterNum = SkyCubePrefilters + SkyCubeConvNum;
		inline static const int GBufferSrvStartAt = SkyCubeMips + SkyCubePrefilters + SkyCubeConvNum + LUTNum;
		// Screen Efx phase Rtvs
		#define MotionVectorRtvStart 0
		inline static const int MotionVectorRtvNum = 1;
		#define TAARtvsStart  MotionVectorRtvNum + MotionVectorRtvStart
		inline static const int TAARtvsNum = 3;
		#define ScreenEfxRtvsCount TAARtvsStart + TAARtvsNum

	protected:
		SDx12RenderTarget2D* mMotionVectorRT;
		std::shared_ptr<SDx12RenderTarget2D>* GBufferRTs;
		std::shared_ptr<SDx12RenderTarget2D> mBrdfLutRT2D;
		std::shared_ptr<SDx12RenderTarget2D>* mTaaRTs;

		std::shared_ptr<SGBufferPass> mGbufferPass = nullptr;
		std::shared_ptr<SsaoPass> mSsaoPass = nullptr;
		std::shared_ptr<SkySpherePass> mDrawSkyPass = nullptr;
		std::shared_ptr<STaaPass> mTaaPass = nullptr;
		std::shared_ptr<SMotionVectorPass> mMotionVectorPass = nullptr;

		struct SRVs
		{
			inline static const std::string GBufferSrvName = "GBufferSrv";
			inline static const std::string CaptureSrvName = "CaptureSrv";
			inline static const std::string DeferredSrvName = "DeferredSrv";
			inline static const std::string ScreenEfxSrvName = "ScreenEfxSrv";
		};
		struct RTVs
		{
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
				L"Textures/Urn_ALB.dds",
				L"Textures/Urn_RMA.dds",
				L"Textures/Urn_RMA.dds",
				L"Textures/Urn_NRM.dds",
				L"Textures/grasscube1024.dds",
				L"Textures/020.hdr"
			};
		};
		struct RT2Ds
		{
			inline static const std::string MotionVectorRTName = "MotionVectorRT";
		};
		struct RT3Ds
		{

		};

		// Helper Containers
		std::vector<ID3D12Resource*> mGBufferSrvResources;
		std::vector<ID3D12Resource*> mSsaoSrvResources;
		std::vector<ID3D12Resource*> mTaaResources;
		std::vector<ID3D12Resource*> mConvAndPrefilterSkyCubeResource[SkyCubeConvFilterNum];
		std::vector<ID3D12Resource*> mSkyCubeResource;

		// SRenderTargetCubeMultiLevels<5>
		std::shared_ptr<SDx12RenderTargetCube> mConvAndPrefilterCubeRTs[SkyCubeConvFilterNum];
		std::shared_ptr<SDx12RenderTargetCube> mSkyCubeRT[SkyCubeMips];

		//TAA
		inline static const int TAA_SAMPLE_COUNT = 8;
		inline static const float TAA_JITTER_DISTANCE = 1.f;
	};

}
	