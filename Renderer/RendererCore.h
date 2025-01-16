#pragma once

#include <sal.h>
#include <wrl.h>
#include <memory>
#include <directxtk\SimpleMath.h>

using namespace Microsoft::WRL;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

using Matrix = DirectX::SimpleMath::Matrix;
using Vector3 = DirectX::SimpleMath::Vector3;
using Vector4 = DirectX::SimpleMath::Vector4;
using XMFLOAT3 = DirectX::XMFLOAT3;
using XMMATRIX = DirectX::XMMATRIX;
using BoundingBox = DirectX::BoundingBox;
using BoundingOrientedBox = DirectX::BoundingOrientedBox;

struct ID3D11DeviceContext;
struct ID3D11Device;
struct ID3D11DeviceChild;
struct ID3D11Buffer;
struct ID3D10Blob;
typedef ID3D10Blob ID3DBlob;
struct ID3D11ShaderResourceView;
struct D3D11_BUFFER_DESC;


struct RendererUtility
{
public:
	static void SetDevice(ComPtr<ID3D11Device> device);
	static ID3D11Device* GetDevice();

	static void SetSwapChain(ComPtr<struct IDXGISwapChain1> swapChain);
	static ComPtr<struct IDXGISwapChain1> GetSwapChain();

private:
	static ComPtr<ID3D11Device> device;
	static ComPtr<struct IDXGISwapChain1> swapChain;
	static std::atomic_int deviceCount;
	static std::atomic_int swapChainCount;
};

static struct Check
{
	void operator()(HRESULT result);
} Check;

namespace EShaderType
{
	enum Type
	{
		Vertex,
		Pixel,
		Geometry,
		Compute,
		Hull,
		Domain,
		MAX
	};
}

namespace EShaderBindable
{
	enum Type
	{
		ConstantBuffer,
		ShaderResource,
		UnorderedAccess,
		Sampler
	};
}

struct Binadble
{
	EShaderType::Type shaderType;
	EShaderBindable::Type bindableType;
	uint32_t slot;

	ComPtr<IUnknown> bind;
};


struct CameraBufferData
{
	alignas(16) Vector3 MainCamPos;
	alignas(16) Matrix View;
	alignas(16) Matrix Projection;
	alignas(16) Matrix IVM;
	alignas(16) Matrix IPM;
};
