#pragma once
#include <D3DCore/D3DRenderer.h> 
#include <memory>
#include <unordered_set>
#include <format>

/* b0 레지스터는 Transform 버퍼*/
struct cb_Transform
{
	DirectX::SimpleMath::Matrix World;
	DirectX::SimpleMath::Matrix WorldInverseTranspose;
	DirectX::SimpleMath::Matrix WVP;
};

/* b1 레지스터는 Camera 버퍼*/
struct cb_Camera
{
	DirectX::SimpleMath::Vector3 MainCamPos;
	float camPad{};

	DirectX::SimpleMath::Matrix View;
	DirectX::SimpleMath::Matrix Projection;
	DirectX::SimpleMath::Matrix IVM;
	DirectX::SimpleMath::Matrix IPM;
};

/* b2 레지스터는 ShadowMap 버퍼*/
struct cb_ShadowMap
{
	DirectX::SimpleMath::Matrix ShadowProjections[cb_PBRDirectionalLight::MAX_LIGHT_COUNT];
	DirectX::SimpleMath::Matrix ShadowViews[cb_PBRDirectionalLight::MAX_LIGHT_COUNT];
};

/*copy to fxh file

cbuffer cb_Transform : register(b0)
{
	Matrix World;
	Matrix WorldInverseTranspose;
	Matrix WVP;
};

cbuffer cb_Camera : register(b1)
{
	float3 MainCamPos;
	Matrix View;
	Matrix Projection;
	Matrix IVM;
	Matrix IPM;
};

cbuffer cb_ShadowMap : register(b2)
{
	Matrix ShadowProjections[MAX_LIGHT_COUNT];
	Matrix ShadowViews[MAX_LIGHT_COUNT];
};

*/

/*상수 버퍼용 4바이트 bool*/
struct alignas(4) CBbool
{
	int value;

	CBbool() { value = false; }
	CBbool(bool value) { this->value = (int)value; }

	bool& operator=(bool value) { return (bool&)this->value = (int)value; }
	bool* operator&() { return (bool*)&value; }

	operator bool() const { return value; }
	operator bool*() { return (bool*)&value; }
};

class D3DConstBuffer
{
	friend D3DRenderer;
public:
	static constexpr bool UPDATE_MODE_MAP = true;
	static void CreateStaticCbuffer();
	static void ReleaseStaticCbuffer();
	static void SetStaticCbuffer();

	/*상수버퍼 초기화 여부 리셋. Scene 소멸자에서 호출됩니다.*/
	inline static void ClearInitFlag() { initFlagSet.clear(); }

	template<typename T>
	static void UpdateStaticCbuffer(const T& data);

	template<typename T>
	inline static std::shared_ptr<T> GetData(const char* key);
private:
	static std::shared_ptr<char[]> GetData(size_t size_of, const char* data_key);
	inline static std::string make_key_data(size_t size_of, const char* key);
	static bool IsSerializedData(const char* data_key);

private:
	inline static constexpr size_t StaticCbufferCount = 3;
	inline static ID3D11Buffer* cBufferTransform = nullptr;
	inline static ID3D11Buffer* cBufferCamera = nullptr;
	inline static ID3D11Buffer* cBufferShadowMap = nullptr;

private:
	inline static std::unordered_map<std::string, ID3D11Buffer*> cbufferMap;
	inline static std::unordered_map<std::string, std::weak_ptr<char[]>> dataMap;
	inline static std::unordered_set<std::string> initFlagSet;

public:
	D3DConstBuffer();
	~D3DConstBuffer();
	void clear();
	void clear_vs();
	void clear_ps();

	void Serialized(std::ofstream& ofs);
	void Deserialized(std::ifstream& ifs);

public:
	D3DConstBuffer(const D3DConstBuffer& rhs) = delete;
	D3DConstBuffer& operator=(const D3DConstBuffer& rhs) = delete;

private:
	std::unordered_map<size_t, int> sizeCounterMap;
	inline std::string make_key_cbuffer(size_t size_of);

public:
	void SetConstBuffer(); //상수 버퍼 바인딩

	/*
	VS 상수 버퍼 생성.
	return : register index
	*/
	template<typename T>
	int CreateVSConstantBuffers(const char* key);
	/*
	PS 상수 버퍼 생성.
	return : register index
	*/
	template<typename T>
	int CreatePSConstantBuffers(const char* key);

	/*바인딩된 데이터 가져오기*/
	template<typename T>
	inline std::shared_ptr<T> GetVSData(int index);

	/*바인딩된 데이터 가져오기*/
	template<typename T>
	inline std::shared_ptr<T> GetPSData(int index);

	/*바인딩 시킬 키 변경하기*/
	template<typename T>
	void ChangeVSkey(int index, const char* key);
	/*바인딩 시킬 키 변경하기*/
	template<typename T>
	void ChangePSkey(int index, const char* key);
private:
	int CreateVSConstantBuffers(size_t size_of, const char* data_key);
	int CreatePSConstantBuffers(size_t size_of, const char* data_key);
	std::shared_ptr<char[]> GetVSData(size_t size_of, int index);
	std::shared_ptr<char[]> GetPSData(size_t size_of, int index);
	void ChangeVSkey(size_t size_of, int index, const char* key);
	void ChangePSkey(size_t size_of, int index, const char* key);
private:
	std::vector<std::pair<std::string, std::string>> vs_keyList{};
	std::vector<std::pair<size_t, std::shared_ptr<char[]>>> vs_dataList{};
	std::vector<std::pair<std::string, std::string>> ps_keyList{};
	std::vector<std::pair<size_t, std::shared_ptr<char[]>>> ps_dataList{};

	inline void UpdateVSconstBuffer(int index);
	inline void UpdatePSconstBuffer(int index);
public:
	int GetVSCbufferCount() { return (int)vs_keyList.size() + StaticCbufferCount; }
	int GetPSCbufferCount() { return (int)ps_keyList.size() + StaticCbufferCount; }

};

template<typename T>
inline void D3DConstBuffer::UpdateStaticCbuffer(const T& data)
{
	static_assert(false, "T is not static buffer");
}

template<typename T>
inline std::shared_ptr<T> D3DConstBuffer::GetData(const char* key)
{
	static_assert((sizeof(T) % 16) == 0, "Constant Buffer size must be 16-byte aligned. Ensure that the size of the structure is a multiple of 16 bytes.");
	static_assert(!std::is_polymorphic_v<T>, "Constant Buffer cannot have virtual functions. The type must be a plain old data (POD) structure.");
	static_assert(std::is_trivially_destructible_v<T>, "Constant Buffer cannot have a destructor. The type must not be destructible.");

	std::string data_key = make_key_data(sizeof(T), key);
	std::shared_ptr<char[]> data = GetData(sizeof(T), data_key.c_str());

	// If this is the first reference, create a T object using placement new
	auto [iter, result] = initFlagSet.insert(data_key);
	if (result)
		new(data.get()) T;

	std::shared_ptr<T> ptr = std::reinterpret_pointer_cast<T>(data);
	
	return ptr;
}

inline std::string D3DConstBuffer::make_key_data(size_t size_of, const char* key)
{
	return std::format("{}_{}", size_of, key);
}

template<>
inline void D3DConstBuffer::UpdateStaticCbuffer(const cb_Transform& data)
{
	ID3D11DeviceContext* pDeviceContext = d3dRenderer.GetDeviceContext();
	if constexpr (UPDATE_MODE_MAP)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		Utility::CheckHRESULT(pDeviceContext->Map(cBufferTransform, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		// 상수 버퍼에 데이터를 쓴다.
		cb_Transform* dataPtr = (cb_Transform*)mappedResource.pData;
		*dataPtr = data;
		// 맵핑 해제
		pDeviceContext->Unmap(cBufferTransform, 0);
	}
	else
	{
		pDeviceContext->UpdateSubresource(cBufferTransform, 0, NULL, &data, 0, 0);
	}
}

template<>
inline void D3DConstBuffer::UpdateStaticCbuffer(const cb_Camera& data)
{
	ID3D11DeviceContext* pDeviceContext = d3dRenderer.GetDeviceContext();
	if constexpr (UPDATE_MODE_MAP)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		Utility::CheckHRESULT(pDeviceContext->Map(cBufferCamera, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		// 상수 버퍼에 데이터를 쓴다.
		cb_Camera* dataPtr = (cb_Camera*)mappedResource.pData;
		*dataPtr = data;
		// 맵핑 해제
		pDeviceContext->Unmap(cBufferCamera, 0);
	}
	else
	{
		pDeviceContext->UpdateSubresource(cBufferCamera, 0, NULL, &data, 0, 0);
	}
}

template<>
inline void D3DConstBuffer::UpdateStaticCbuffer(const cb_ShadowMap& data)
{
	ID3D11DeviceContext* pDeviceContext = d3dRenderer.GetDeviceContext();
	if constexpr (UPDATE_MODE_MAP)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		Utility::CheckHRESULT(d3dRenderer.GetDeviceContext()->Map(cBufferShadowMap, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		// 상수 버퍼에 데이터를 쓴다.
		cb_ShadowMap* dataPtr = (cb_ShadowMap*)mappedResource.pData;
		*dataPtr = data;
		// 맵핑 해제
		d3dRenderer.GetDeviceContext()->Unmap(cBufferShadowMap, 0);
	}
	else
	{
		pDeviceContext->UpdateSubresource(cBufferShadowMap, 0, NULL, &data, 0, 0);
	}
}

template<typename T>
inline int D3DConstBuffer::CreateVSConstantBuffers(const char* key)
{
	static_assert((sizeof(T) % 16) == 0, "Constant Buffer size must be 16-byte aligned. Ensure that the size of the structure is a multiple of 16 bytes.");
	static_assert(!std::is_polymorphic_v<T>, "Constant Buffer cannot have virtual functions. The type must be a plain old data (POD) structure.");
	static_assert(std::is_trivially_destructible_v<T>, "Constant Buffer cannot have a destructor. The type must not be destructible.");

	std::string data_key = make_key_data(sizeof(T), key);
	int regindex = CreateVSConstantBuffers(sizeof(T), data_key.c_str());

	// If this is the first reference, create a T object using placement new
	auto [iter, result] = initFlagSet.insert(data_key);
	if (result)
		new(vs_dataList[regindex - StaticCbufferCount].second.get())T;

	return regindex;
}

template<typename T>
inline int D3DConstBuffer::CreatePSConstantBuffers(const char* key)
{
	static_assert((sizeof(T) % 16) == 0, "Constant Buffer size must be 16-byte aligned. Ensure that the size of the structure is a multiple of 16 bytes.");
	static_assert(!std::is_polymorphic_v<T>, "Constant Buffer cannot have virtual functions. The type must be a plain old data (POD) structure.");
	static_assert(std::is_trivially_destructible_v<T>, "Constant Buffer cannot have a destructor. The type must not be destructible.");

	std::string data_key = make_key_data(sizeof(T), key);
	int regindex = CreatePSConstantBuffers(sizeof(T), data_key.c_str());

	// If this is the first reference, create a T object using placement new
	auto [iter, result] = initFlagSet.insert(data_key);
	if (result)
		new(ps_dataList[regindex - StaticCbufferCount].second.get())T;

	return regindex;
}

template<typename T>
inline std::shared_ptr<T> D3DConstBuffer::GetVSData(int index)
{
	std::shared_ptr<char[]> dataPtr = GetVSData(sizeof(T), index);
	std::shared_ptr<T> Tptr = std::reinterpret_pointer_cast<T>(dataPtr);
	return Tptr;
}

template<typename T>
inline std::shared_ptr<T> D3DConstBuffer::GetPSData(int index)
{
	std::shared_ptr<char[]> dataPtr = GetPSData(sizeof(T), index);
	std::shared_ptr<T> Tptr = std::reinterpret_pointer_cast<T>(dataPtr);
	return Tptr;
}

template<typename T>
inline void D3DConstBuffer::ChangeVSkey(int index, const char* key)
{
	ChangeVSkey(sizeof(T), index, key);
}

template<typename T>
inline void D3DConstBuffer::ChangePSkey(int index, const char* key)
{
	ChangePSkey(sizeof(T), index, key);
}

inline void D3DConstBuffer::UpdateVSconstBuffer(int index)
{
	if (ID3D11DeviceContext* pDeviceContext = d3dRenderer.GetDeviceContext())
	{
		auto& [size, data] = vs_dataList[index];
		auto& [data_key, cbuffer_key] = vs_keyList[index];
		auto findIter = cbufferMap.find(cbuffer_key);
		if (findIter != cbufferMap.end())
		{
			if constexpr (UPDATE_MODE_MAP)
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				Utility::CheckHRESULT(pDeviceContext->Map(findIter->second, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

				// 상수 버퍼에 데이터를 쓴다.
				memcpy(mappedResource.pData, data.get(), size);

				// 맵핑 해제
				pDeviceContext->Unmap(findIter->second, 0);
			}
			else
			{
				pDeviceContext->UpdateSubresource(cbufferMap[cbuffer_key], 0, NULL, data.get(), 0, 0);
			}
			//리소스 바인딩
			pDeviceContext->VSSetConstantBuffers(index + StaticCbufferCount, 1, &findIter->second);
		}
		else
		{
			__debugbreak(); //존재하지 않는 키.
			return;
		}
	}
}

inline void D3DConstBuffer::UpdatePSconstBuffer(int index)
{
	if (ID3D11DeviceContext* pDeviceContext = d3dRenderer.GetDeviceContext()) 
	{
		auto& [size, data] = ps_dataList[index];
		auto& [data_key, cbuffer_key] = ps_keyList[index];
		auto findIter = cbufferMap.find(cbuffer_key);
		if (findIter != cbufferMap.end())
		{
			if constexpr (UPDATE_MODE_MAP)
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				Utility::CheckHRESULT(pDeviceContext->Map(findIter->second, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

				// 상수 버퍼에 데이터를 쓴다.
				memcpy(mappedResource.pData, data.get(), size);

				// 맵핑 해제
				pDeviceContext->Unmap(findIter->second, 0);
			}
			else
			{
				pDeviceContext->UpdateSubresource(cbufferMap[cbuffer_key], 0, NULL, data.get(), 0, 0);
			}
			//리소스 바인딩
			pDeviceContext->PSSetConstantBuffers(index + StaticCbufferCount, 1, &findIter->second);
		}
		else
		{
			__debugbreak(); //존재하지 않는 키.
			return;
		}
	}
}

inline std::string D3DConstBuffer::make_key_cbuffer(size_t size_of)
{
	auto find = sizeCounterMap.find(size_of);
	if (find == sizeCounterMap.end())
	{
		sizeCounterMap[size_of] = 1;
		return std::format("{}_{}", size_of, 0);
	}
	else
	{
		int& counter = sizeCounterMap[size_of];
		return std::format("{}_{}", size_of, counter++);
	}
}