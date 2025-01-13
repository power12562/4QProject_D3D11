#pragma once

#include "RendererCore.h"
#include "ShaderResource.h"
#include "RendererBuffer.h"
#include <vector>
#include <typeinfo>
#include <d3d11.h>

struct StructuredBuffer : public ShaderResource
{
public:
	StructuredBuffer();
	~StructuredBuffer();

public:
	template<typename Container>
	void Set(const Container& contaioner);

	template<typename T, size_t size>
	void Init();

	template<typename Container, std::enable_if_t<!std::is_pointer_v<Container>, int> = 0>
	void Init(const Container& contaioner);

	template<typename Container, std::enable_if_t<!std::is_pointer_v<Container>, int> = 0>
	void Update(const Container& contaioner);

	operator ID3D11ShaderResourceView* () const { return srv.Get(); }

	void CreateView(int newSize);


private:
	RendererBuffer buffer{};
	ComPtr<ID3D11ShaderResourceView> srv{ nullptr };
	const std::type_info* typeInfo{ nullptr };
	uint32_t capicity{ 0 };
	uint32_t size{ 0 };
	uint8_t isInit : 1 { false };
};

template<typename Container>
inline void StructuredBuffer::Set(const Container& contaioner)
{
	using ValueType = Container::value_type;
	if (!isInit || *typeInfo != typeid(ValueType))
	{
		Init(contaioner);
		isInit = true;
	}
	else
	{
		Update(contaioner);
	}
}

template<typename T, size_t size>
inline void StructuredBuffer::Init()
{
	uint32_t bufferSize = sizeof(T) * size;
	D3D11_BUFFER_DESC bufferDesc
	{
		.ByteWidth = bufferSize,
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_SHADER_RESOURCE,
		.CPUAccessFlags = 0,
		.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
		.StructureByteStride = sizeof(T)
	};

	buffer.Init(bufferDesc, bufferSize);

	typeInfo = &typeid(T);
	capicity = size;
	isInit = true;

	CreateView(capicity);
}

template<typename Container, std::enable_if_t<!std::is_pointer_v<Container>, int>>
inline void StructuredBuffer::Init(const Container& contaioner)
{
	using ValueType = Container::value_type;
	uint32_t bufferSize = sizeof(ValueType) * contaioner.size();

	D3D11_BUFFER_DESC bufferDesc
	{
		.ByteWidth = bufferSize,
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_SHADER_RESOURCE,
		.CPUAccessFlags = 0,
		.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
		.StructureByteStride = sizeof(ValueType)
	};

	buffer.Init(bufferDesc, bufferSize);

	typeInfo = &typeid(ValueType);
	capicity = contaioner.size();
	isInit = true;

	CreateView(capicity);
}

template<typename Container, std::enable_if_t<!std::is_pointer_v<Container>, int>>
inline void StructuredBuffer::Update(const Container& contaioner)
{
	if (contaioner.size() > capicity)
	{
		Init(contaioner);
	}
	if (contaioner.size() != size)
	{
		CreateView(contaioner.size());
	}

	buffer.Update(contaioner.data());
}