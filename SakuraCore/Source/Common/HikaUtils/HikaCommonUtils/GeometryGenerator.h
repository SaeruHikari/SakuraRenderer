#pragma once
#include "Common/EngineDefinations/MeshData.hpp"

using namespace SCommon;

class GeometryGenerator
{
public:
	///<summary>
	/// Creates a box centered at the origin with the given dimensions, where each 
	/// face has m rows and n columns of vertices.
	///</summary>
	StaticMeshData CreateBox(float width, float height, float depth, uint32 numSubdivisions);

	///<summary>
	/// Creates a spheres centered at the origin with the given radius. The
	/// slices and stacks parameters controls the degree of tessellation.
	///</summary>
	StaticMeshData CreateSphere(float radius, uint32 sliceCount, uint32 stackCount);

	///<summary>
	/// Creates a geosphere centered at the origin with the given radius. The
	/// depth controls the level of tessellation.
	///</summary>
	StaticMeshData CreateGeosphere(float radius, uint32 numSubdivisions);

	///<summary>
	/// Creates a cylinder parallel to the y-axis, and centered about the origin.
	/// The bottom and top radius can vary to form various cone shapes rather than true
	/// cylinders. The slices and stacks parameters control the degree of tessellation.
	///</summary>
	StaticMeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);

	///<summary>
	/// Creates an mxn grid in the xz-plane with m rows and n columns, centered 
	/// at the origin with the specified width and depth.
	///</summary>
	StaticMeshData CreateGrid(float width, float depth, uint32 m, uint32 n);

	///<summary>
	/// Creates a quad aligned with the screen. This is useful for post processing and screen effects.
	///</summary>
	StaticMeshData CreateQuad(float x, float y, float w, float h, float depth);

private:
	void Subdivide(StaticMeshData& meshData);
	Vertex MidPoint(const Vertex& v0, const Vertex& v1);
	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, StaticMeshData& meshData);
	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, StaticMeshData& meshData);
};

class Waves
{
public:
	Waves(int m, int n, float dx, float dt, float speed, float damping);
	Waves(const Waves& rhs) = delete;
	Waves& operator=(const Waves& rhs) = delete;
	~Waves();

	int RowCount()const;
	int ColumnCount()const;
	int VertexCount()const;
	int TriangleCount()const;
	float Width()const;
	float Depth()const;

	// Returns the solution at the ith grid point.
	const DirectX::XMFLOAT3& Position(int i)const { return mCurrSolution[i]; }

	// Returns the solution normal at the ith grid point.
	const DirectX::XMFLOAT3& Normal(int i)const { return mNormals[i]; }

	// Returns the unit tangent vector at the ith grid point in the local x-axis direction.
	const DirectX::XMFLOAT3& TangentX(int i)const { return mTangentX[i]; }

	void Update(float dt);
	void Disturb(int i, int j, float magnitude);

private:
	int mNumRows = 0;
	int mNumCols = 0;

	int mVertexCount = 0;
	int mTriangleCount = 0;

	// Simulation constants we can precompute.
	float mK1 = 0.0f;
	float mK2 = 0.0f;
	float mK3 = 0.0f;

	float mTimeStep = 0.0f;
	float mSpatialStep = 0.0f;

	std::vector<DirectX::XMFLOAT3> mPrevSolution;
	std::vector<DirectX::XMFLOAT3> mCurrSolution;
	std::vector<DirectX::XMFLOAT3> mNormals;
	std::vector<DirectX::XMFLOAT3> mTangentX;
};