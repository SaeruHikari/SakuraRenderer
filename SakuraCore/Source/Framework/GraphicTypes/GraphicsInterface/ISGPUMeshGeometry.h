#pragma once

namespace SGraphics
{
	struct ISMeshGeometry
	{
	public:
		//Give it a name so we can look it up by name
		std::string Name;

		//Data about the buffers
		UINT VertexByteStride = 0;
		UINT VertexBufferByteSize = 0;
		DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
		UINT IndexBufferByteSize = 0;
	};
}