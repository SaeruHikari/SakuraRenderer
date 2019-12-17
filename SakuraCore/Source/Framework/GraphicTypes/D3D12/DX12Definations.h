#pragma once
#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include "Common/Microsoft/d3dx12.h"
#include "Common/HikaUtils/HikaCommonUtils/MathHelper.h"
#include "../GraphicsInterface/ISTexture.h"

///<summary>
/// Extern const int gNumFrameResources
/// The num of frame resources
///</summary>
inline static const int gNumFrameResources = 3;


class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	std::wstring ToString() const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};

//Defines a subrange of geometry in a Dx12MeshGeometry. This is for when multiple
//geometries are stored in one vertex and index buffer. It provides the offsets
//and data needed to draw a subset if geometry stores in the vertex and index 
//buffers so that we can implement the technique described by Figure 6.3.
struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	//Bounding box of the geometry defined by this submesh
	//This is used in later chapters of the book
	DirectX::BoundingBox Bounds;
};

struct Dx12MeshGeometry
{
	//Give it a name so we can look it up by name
	std::string Name;

	//System memory copies. Use Blobs because the vertex/index format can be generic
	//It is up to the client to cast appropriately
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	//Data about the buffers
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	//A Dx12MeshGeometry may store multiple geometries in one vertex/index buffer.
	//Use this container to define the Submesh geometries so we can draw
	//the Submeshes individually
	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const {
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}

	//We can free this memory after we finish upload to the GPU
	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}
};

struct Light
{
	DirectX::XMFLOAT3 Strength = { .5f, .5f, .5f };
	float FalloffStart = 1.f;							// point/spot light only
	DirectX::XMFLOAT3 Direction = { 0.f, -1.f, 0.f };	// direction/spot light only
	float FalloffEnd = 10.f;							// point/spot light only
	DirectX::XMFLOAT3 Position = { 0.f, 0.f, 0.f };		// point/spot light only
	float SpotPower = 64.f;								// spot light only
};

#define MaxLights 16

struct MaterialConstants
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.f, 1.f, 1.f, 1.f };
	DirectX::XMFLOAT3 FresnelR0 = { .01f, .01f, .01f };
	float Roughness = .25f;

	// Used in texture mapping
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

// Simple struct to represent a material for our demos. A production 3D engine
// would likely create a class hierarchy of Materials.
struct Material
{
	// Unique material name for lookup.
	std::string Name;

	// Index into constant buffer corresponding to this material.
	int MatCBIndex = -1;

	// Index into SRV heap for diffuse texture.
	int DiffuseSrvHeapIndex = -1;

	// Index into SRV heap for normal texture.
	int NormalSrvHeapIndex = -1;

	// Dirty flag indicating the material has changed and we need to update the constant buffer.
	// Because we have a material constant buffer for each FrameResource, we have to apply the 
	// update to each FrameResource. Thus, when we modify a material we should set
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Material constant buffer data used for shading.
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.f, 1.f, 1.f, 1.f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, .01f, .01f };
	float Roughness = .25f;
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

struct PBRMaterialConstants
{
	DirectX::XMFLOAT3 BaseColor = { 1.f, 1.f, 1.f };
	float Opaque = 1.f;

	float	Metallic = 1.f;
	float	Roughness = 1.f;
	float   SpecularTint = .4f;
	float   SpecularStrength = 1.f;

	DirectX::XMFLOAT3 SpecularColor = { 1.f, 1.f, 1.f };
	float 	Anisotropic = 0.f;

	float 	Subsurface = 1.f;
	DirectX::XMFLOAT3  SubsurfaceColor = BaseColor;

	float   Clearcoat = 0.f;
	float	ClearcoarGloss = 1.f;
	float   Sheen = 0.f;
	float 	SheenTint = 0.f;

	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();


	// Index into SRV heap for diffuse texture.
	int DiffuseSrvHeapIndex = -1;
	// Index into SRV heap for roughness texture.
	int RMOSrvHeapIndex = -1;
	// Index into SRV heap for metallic texture.
	int SpecularSrvHeapIndex = -1;
	// Index into SRV heap for normal texture.
	int NormalSrvHeapIndex = -1;
};

struct OpaqueMaterial
{
	// Unique material name for lookup.
	std::string Name;

	// Dirty flag indicating the material has changed and we need to update the constant buffer.
	// Because we have a material constant buffer for each FrameResource, we have to apply the 
	// update to each FrameResource. Thus, when we modify a material we should set
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;
	// Index into constant buffer corresponding to this material.
	int MatCBIndex = -1;
	// Material constant buffer data used for shading.
	PBRMaterialConstants MatConstants;
};

struct SD3DTexture : public SGraphics::ISTexture
{
	SD3DTexture()
	{

	}
	SD3DTexture(const std::string& name, ID3D12Resource* resource)
	{
		Name = name;
		Resource.Attach(resource);
	}
	// Unique texture name for lookup
	std::string Name;
	std::wstring Filename;
	virtual SGraphics::SResourceHandle* GetResourceHandle()
	{
		return nullptr;
	}
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

// Lightweight structure stores parameters to draw a shape. This will
// vary from app-to-app
struct SDxRenderItem
{
	SDxRenderItem() = default;

	// World matrix of the shape that describes the object's local space
	// relative to the world space, which defines the position, orientation,
	// and scale of the object in the world.
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 PrevWorld = MathHelper::Identity4x4();

	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the 
	// update to each FrameResource. Thus, when we modify object data we should set
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	OpaqueMaterial* Mat = nullptr;
	Dx12MeshGeometry* Geo = nullptr;

	//Primitive topology
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced parameters
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};