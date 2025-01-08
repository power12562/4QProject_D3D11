#pragma once

#include "RendererCore.h"
#include "ShaderResource.h"
#include "RendererBuffer.h"
#include <vector>
#include <typeinfo>
#include <d3d11.h>

struct ConstantBuffer : public ShaderResource
{
public:
	ConstantBuffer();
	~ConstantBuffer();

public:
	template<typename T>
	void Init(uint32_t registrerSlot, _In_opt_ const T* data = nullptr);
	template<typename T>
	void Update(_In_opt_ T* data = nullptr);

	void ChangeSlot(uint32_t newSlot) { registrerSlot = newSlot; }
	operator ID3D11Buffer* () { return buffer; }

private:
	RendererBuffer buffer;
	const std::type_info* typeInfo;
	uint32_t registrerSlot;
};

template<typename T>
inline void ConstantBuffer::Init(uint32_t registrerSlot, _In_opt_ const T* data)
{
	buffer.Init(D3D11_BUFFER_DESC{ .Usage = D3D11_USAGE_DEFAULT, .BindFlags = D3D11_BIND_CONSTANT_BUFFER }, sizeof(T), data);
	typeInfo = &typeid(T);
	this->registrerSlot = registrerSlot;
}

template<typename T>
inline void ConstantBuffer::Update(T* data)
{
	if (*typeInfo == typeid(T))
	{
		buffer.Update(data);
	}
}
