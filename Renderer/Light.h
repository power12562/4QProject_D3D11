#pragma once

#include "RendererCore.h"
#include "StructuredBuffer.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include <map>
#include <string>

struct DirectionLightData
{
	Vector4 Color;
	Vector4 Directoin;
	Matrix VP;
};

struct DirectionLightBuffer
{
public:
	DirectionLightBuffer(int capacity = 0);

public:
	void PushLight(std::string_view name, const DirectionLightData& light);
	void PopLight(std::string_view name);
	DirectionLightData& GetLight(std::string_view name);
	DirectionLightData& GetLight(size_t index);


	ConstantBuffer& GetLightCamera(size_t index) { return lightCameras[index]; }
	StructuredBuffer GetDirectLightBuffer() const { return directLightBuffer; }
	ID3D11ShaderResourceView* GetShadowMapArray() const { return (ID3D11ShaderResourceView*)shadowMap; }
	ID3D11DepthStencilView* GetShadowMapDS(int index) const { return shadowMap_DSV[index].Get(); }

	uint32_t size() const { return _size; }
	void UpdateBuffer();

	static void ComputeLightMatrix(const BoundingBox& boundBox,							// 바운딩 박스
								   const Vector3& lightDirection,							// 빛의 방향 (정규화된 벡터)
								   _Out_ XMMATRIX& lightViewMatrix,						// 빛 뷰 행렬
								   _Out_ XMMATRIX& lightProjMatrix,						// 빛 직교 투영 행렬)
								   _Out_opt_ BoundingOrientedBox* outProjectBox = nullptr	// 디버깅용 투영박스
	);
private:
	// 하드웨어 친화적으로 설계됨, like ECS
	std::map<std::string, int> names;
	std::vector<DirectionLightData> lights;
	std::vector<ConstantBuffer> lightCameras;


	StructuredBuffer directLightBuffer;
	Texture shadowMap;
	std::vector<ComPtr<struct ID3D11DepthStencilView>> shadowMap_DSV;

	uint32_t capacity;
	uint32_t _size;

private:
	void CreateBuffer(UINT newCapacity);
	void CreateView(uint32_t newSize);
};