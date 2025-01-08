#pragma once

#include <sal.h>
#include <wrl.h>
#include <memory>

using namespace Microsoft::WRL;

template<typename T>
using SharedPtr = std::shared_ptr<T>;


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

private:
	static ComPtr<ID3D11Device> device;
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
	};
}