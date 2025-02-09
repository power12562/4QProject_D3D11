#include "RendererBuffer.h"
#include <d3d11.h>

RendererBuffer::RendererBuffer() = default;
RendererBuffer::~RendererBuffer() = default;

void RendererBuffer::Load(ComPtr<ID3D11Buffer> buffer)
{
	this->buffer = std::move(buffer);
	RendererUtility::GetDevice()->GetImmediateContext(&immediateContext);
}

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

	result = RendererUtility::GetDevice()->CreateBuffer(&bufferDesc, initDataPointer, &buffer);
	Check(result);

	RendererUtility::GetDevice()->GetImmediateContext(&immediateContext);
}

void RendererBuffer::Update(const void* data)
{
	immediateContext->UpdateSubresource(buffer.Get(), 0, nullptr, data, 0, 0);
}



