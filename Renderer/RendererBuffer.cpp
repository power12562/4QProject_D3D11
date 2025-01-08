#include "RendererBuffer.h"
#include <d3d11.h>

RendererBuffer::RendererBuffer() = default;
RendererBuffer::~RendererBuffer() = default;

void RendererBuffer::Init(D3D11_BUFFER_DESC bufferDesc, size_t bufferSize, _In_opt_ const void* data)
{
	HRESULT result;

	bufferDesc.ByteWidth = bufferSize;


	D3D11_SUBRESOURCE_DATA initData = {};
	D3D11_SUBRESOURCE_DATA* initDataPointer = nullptr;
	if (data)
	{
		initData.pSysMem = data;
		initDataPointer = &initData;
	}
	ComPtr<ID3D11Buffer> newBuffer;
	result = RendererUtility::GetDevice()->CreateBuffer(&bufferDesc, initDataPointer, &newBuffer);
	Check(result);

	buffer = newBuffer;
	RendererUtility::GetDevice()->GetImmediateContext(&immediateContext);
}

void RendererBuffer::Update(void* data)
{
	immediateContext->UpdateSubresource(buffer.Get(), 0, nullptr, data, 0, 0);
}



