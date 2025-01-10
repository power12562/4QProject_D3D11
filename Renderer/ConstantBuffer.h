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
	void Init(_In_opt_ const T* data = nullptr);
	template<typename T>
	void Update(_In_opt_ T* data = nullptr);


	operator ID3D11Buffer* () { return buffer; }


private:
	RendererBuffer buffer;
	const std::type_info* typeInfo;
};

template<typename T>
inline void ConstantBuffer::Init(_In_opt_ const T* data)
{
	buffer.Init(D3D11_BUFFER_DESC{ .Usage = D3D11_USAGE_DEFAULT, .BindFlags = D3D11_BIND_CONSTANT_BUFFER }, sizeof(T), data);
	typeInfo = &typeid(T);
}

template<typename T>
inline void ConstantBuffer::Update(T* data)
{
	if (*typeInfo == typeid(T))
	{
		buffer.Update(data);
	}
}
