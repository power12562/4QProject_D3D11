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
	void Set(const T& data);

	template<typename T>
	void Init();

	template<typename T, std::enable_if_t<!std::is_pointer_v<T>, int> = 0>
	void Init(const T& data = nullptr);
	
	template<typename T>
	void Update(const T& data = nullptr);

	operator ID3D11Buffer* () { return buffer; }


private:
	RendererBuffer buffer{};
	const std::type_info* typeInfo{ nullptr };
	uint8_t isInit : 1 { false };


private:

};

template<typename T>
inline void ConstantBuffer::Set(const T& data)
{
	if (!isInit || *typeInfo != typeid(T))
	{
		Init(data);
		isInit = true;
	}
	else
	{
		Update(data);
	}
}

template<typename T>
inline void ConstantBuffer::Init()
{
	buffer.Init(D3D11_BUFFER_DESC{ .Usage = D3D11_USAGE_DEFAULT, .BindFlags = D3D11_BIND_CONSTANT_BUFFER }, sizeof(T));
	typeInfo = &typeid(T);
}

template<typename T, std::enable_if_t<!std::is_pointer_v<T>, int>>
inline void ConstantBuffer::Init(_In_opt_ const T& data)
{
	buffer.Init(D3D11_BUFFER_DESC{ .Usage = D3D11_USAGE_DEFAULT, .BindFlags = D3D11_BIND_CONSTANT_BUFFER }, sizeof(T), &data);
	typeInfo = &typeid(T);
}

template<typename T>
inline void ConstantBuffer::Update(const T& data)
{
	if (*typeInfo == typeid(T))
	{
		buffer.Update(&data);
	}
}
