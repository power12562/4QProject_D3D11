#include "Light.h"
#include <d3d11.h>
#include <format>
#include <ranges>


DirectionLightBuffer::DirectionLightBuffer(int capacity) :
	capacity(capacity),
	_size(0)
{
	shadowMap_DSV.reserve(capacity);
}

void DirectionLightBuffer::CreateBuffer(UINT newCapacity)
{
	HRESULT result;

	capacity = newCapacity;

	D3D11_TEXTURE2D_DESC shadowMapDesc
	{
		.Width = 4096,
		.Height = 4096,
		.MipLevels = 1,
		.ArraySize = capacity,
		.Format = DXGI_FORMAT_R32_TYPELESS,
		.SampleDesc = { 1, 0 },
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE
	};

	ComPtr<ID3D11Texture2D> shadowMapArray;
	result = RendererUtility::GetDevice()->CreateTexture2D(&shadowMapDesc, NULL, &shadowMapArray);
	Check(result);


	D3D11_SHADER_RESOURCE_VIEW_DESC shadowMapSRVDesc
	{
		.Format = DXGI_FORMAT_R32_FLOAT,
		.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
		.Texture2DArray = { 0, 1 , 0, capacity }
	};
	
	shadowMap.LoadTexture(shadowMapArray.Get(), ETextureUsage::SRV, &shadowMapSRVDesc);
}

void DirectionLightBuffer::CreateView(uint32_t newSize)
{
	_size = newSize;
	shadowMap_DSV.resize(_size);

	HRESULT result;
	for (UINT i = 0; i < shadowMap_DSV.size(); i++)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC shadowMapDSVDesc
		{
			.Format = DXGI_FORMAT_D32_FLOAT,
			.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY,
			.Texture2DArray = { 0 , i, 1 }
		};
		result = RendererUtility::GetDevice()->CreateDepthStencilView(shadowMap, &shadowMapDSVDesc, &shadowMap_DSV[i]);
		Check(result);
	}
	lightCameras.resize(lights.size());
}

void DirectionLightBuffer::UpdateBuffer()
{
	directLightBuffer.Update(lights);
}


void DirectionLightBuffer::PushLight(std::string_view name, const DirectionLightData& light)
{
	lights.emplace_back(light);
	names.emplace(name.data(), lights.size() - 1);

	if (lights.capacity() > capacity)
	{
		CreateBuffer(lights.capacity());
	}

	CreateView(++_size);
}

void DirectionLightBuffer::PopLight(std::string_view name)
{
	auto popIndex = names[name.data()];
	auto lastIndex = lights.size() - 1;
	std::swap(lights[popIndex], lights.back());

	auto iter = std::find_if(names.begin(), names.end(), [lastIndex](const auto& pair) { return pair.second == lastIndex; });
	if (iter != names.end())
	{
		iter->second = popIndex;
	}

	CreateView(--_size);
}

DirectionLightData& DirectionLightBuffer::GetLight(std::string_view name)
{
	return lights[names[name.data()]];
}

DirectionLightData& DirectionLightBuffer::GetLight(size_t index)
{
	return lights[index];
}

void DirectionLightBuffer::ComputeLightMatrix(const BoundingBox& boundBox, 
											  const Vector3& lightDirection, 
											  _Out_ XMMATRIX& lightViewMatrix, 
											  _Out_ XMMATRIX& lightProjMatrix, 
											  _Out_opt_ BoundingOrientedBox* outProjectBox)
{
	// 1. 카메라 절두체의 꼭짓점 계산 (월드 좌표계로 가정)
	XMFLOAT3 frustumCorners[8];
	boundBox.GetCorners(frustumCorners);

	// 2. 빛 뷰 행렬 생성
	Vector3 lightUp(0.0f, 1.0f, 0.0f); // 기본 Up 벡터
	if (abs(lightDirection.y) > 0.99f) // 빛이 위나 아래로 향하는 경우
	{
		lightUp = Vector3(0.0f, 0.0f, -1.0f); // Up 벡터를 바꿈
	}
	lightViewMatrix = XMMatrixLookToLH(Vector3::Zero, lightDirection, lightUp);

	// 3. 빛 좌표계로 변환된 절두체의 꼭짓점 계산
	Vector4 lightMin(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
	Vector4 lightMax(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (int i = 0; i < 8; ++i)
	{
		Vector4 corner = DirectX::XMLoadFloat3(&frustumCorners[i]);
		corner = Vector4::Transform(corner, Matrix(lightViewMatrix)); // 빛 뷰 공간으로 변환
		lightMin = XMVectorMin(lightMin, corner);
		lightMax = XMVectorMax(lightMax, corner);
		XMStoreFloat3(&frustumCorners[i], corner);
	}

	// 4. 빛 좌표계에서 AABB 기반 직교 투영 행렬 생성
	XMFLOAT3 lightBoxMin, lightBoxMax;
	XMStoreFloat3(&lightBoxMin, lightMin);
	XMStoreFloat3(&lightBoxMax, lightMax);

	lightProjMatrix = DirectX::XMMatrixOrthographicOffCenterLH(
		lightBoxMin.x, lightBoxMax.x, // left, right
		lightBoxMin.y, lightBoxMax.y, // bottom, top
		lightBoxMin.z, lightBoxMax.z + 1  // near, far
	);

	if (outProjectBox)
	{
		BoundingOrientedBox::CreateFromPoints(*outProjectBox,
											  std::size(frustumCorners),
											  std::data(frustumCorners),
											  sizeof(XMFLOAT3));

	}
}
