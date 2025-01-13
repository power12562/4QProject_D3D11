#include "StructuredBuffer.h"



StructuredBuffer::StructuredBuffer() = default;
StructuredBuffer::~StructuredBuffer() = default;












void StructuredBuffer::CreateView(int newSize)
{
	HRESULT result;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = newSize;

	result = RendererUtility::GetDevice()->CreateShaderResourceView((ID3D11Buffer*)buffer, &srvDesc, &srv);
	Check(result);
	
	this->size = newSize;
}