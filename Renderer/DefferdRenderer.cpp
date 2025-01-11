#include "DefferdRenderer.h"
#include <format>
#include <functional>


#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")



#ifdef _DEBUG
#include <dxgidebug.h>
//https://develop-dream.tistory.com/141
void list_remaining_d3d_objects()
{
	HMODULE dxgidebugdll = GetModuleHandleW(L"dxgidebug.dll");
	decltype(&DXGIGetDebugInterface) GetDebugInterface = reinterpret_cast<decltype(&DXGIGetDebugInterface)>(GetProcAddress(dxgidebugdll, "DXGIGetDebugInterface"));

	IDXGIDebug* debug;

	GetDebugInterface(IID_PPV_ARGS(&debug));

	OutputDebugStringW(L"Starting Live Direct3D Object Dump:\r\n");
	debug->ReportLiveObjects(DXGI_DEBUG_D3D11, DXGI_DEBUG_RLO_DETAIL);
	OutputDebugStringW(L"Completed Live Direct3D Object Dump.\r\n");

	debug->Release();
}
#endif

#pragma comment(lib, "dxguid.lib")


DefferdRenderer::DefferdRenderer()
{
	HRESULT result;
	device = RendererUtility::GetDevice();
	if (!device)
	{
		UINT creationFlags = 0;
#ifdef _DEBUG
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		result = D3D11CreateDevice(nullptr,
								   D3D_DRIVER_TYPE_HARDWARE,
								   nullptr,
								   creationFlags,
								   nullptr,
								   0,
								   D3D11_SDK_VERSION,
								   &device,
								   nullptr,
								   &immediateContext);
		Check(result);
	}
	RendererUtility::SetDevice(device);

	device->GetImmediateContext(&immediateContext);

	struct Init
	{
		ComPtr<ID3D11Device> device;

		void CreateNoWriteDepthStencilState(ID3D11DepthStencilState** depthState)
		{
			CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

			HRESULT result;
			result = device->CreateDepthStencilState(&desc, depthState);
			Check(result);
		}
		void CreateNoRenderState(ID3D11BlendState** depthState)
		{
			CD3D11_BLEND_DESC desc(D3D11_DEFAULT);
			desc.RenderTarget[0].RenderTargetWriteMask = 0;

			HRESULT result;
			result = device->CreateBlendState(&desc, depthState);
			Check(result);
		}
		void CreateAlphaRenderState(ID3D11BlendState** depthState)
		{
			CD3D11_BLEND_DESC desc(D3D11_DEFAULT);

			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

			desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL ^ D3D11_COLOR_WRITE_ENABLE_ALPHA;

			HRESULT result;
			result = device->CreateBlendState(&desc, depthState);
			Check(result);
		}
	};

	Init init{ device };
	init.CreateNoWriteDepthStencilState(&noWriteDSS);
	init.CreateNoRenderState(&noRenderState);
	init.CreateAlphaRenderState(&alphaRenderState);


	
	CameraBufferData cameraData{};
	cameraBuffer.Init(cameraData);
	cameraBinadbleVS.shaderType = EShaderType::Vertex;
	cameraBinadbleVS.bindableType = EShaderBindable::ConstantBuffer;
	cameraBinadbleVS.slot = 1;
	cameraBinadbleVS.bind = cameraBuffer;

	cameraBinadblePS.shaderType = EShaderType::Pixel;
	cameraBinadblePS.bindableType = EShaderBindable::ConstantBuffer;
	cameraBinadblePS.slot = 1;
	cameraBinadblePS.bind = cameraBuffer;
}

DefferdRenderer::~DefferdRenderer()
{
	RendererUtility::SetDevice(nullptr);
#ifdef _DEBUG
	atexit(list_remaining_d3d_objects);
#endif
}

void DefferdRenderer::AddDrawCommand(_In_ const MeshDrawCommand& command)
{
	allDrawCommandsOrigin.emplace_back(command);
}

void DefferdRenderer::AddBinadble(_In_ const Binadble& bindable)
{
	bindables.emplace_back(bindable);
}

void DefferdRenderer::SetRenderTarget(_In_ Texture& target)
{
	renderTarget = &target;

	{
		D3D11_TEXTURE2D_DESC desc{};
		((ID3D11Texture2D*)target)->GetDesc(&desc);
		width = desc.Width;
		height = desc.Height;
	}


	for (size_t i = 0; i < std::size(renderBuffers); i++)
	{
		HRESULT result;
		ComPtr<ID3D11Texture2D> texture;
		CD3D11_TEXTURE2D_DESC desc = CD3D11_TEXTURE2D_DESC( 
			DXGI_FORMAT_R8G8B8A8_UNORM,
			width,
			height,
			1,
			0,
			D3D11_BIND_RENDER_TARGET);


		result = device->CreateTexture2D(&desc, nullptr, &texture);
		Check(result);
		renderBuffers[i].LoadTexture(texture.Get(), ETextureUsage::RTV);
	}
	{
		HRESULT result;
		CD3D11_TEXTURE2D_DESC desc = CD3D11_TEXTURE2D_DESC(
			DXGI_FORMAT_R24G8_TYPELESS,
			width,
			height,
			1,
			0,
			D3D11_BIND_DEPTH_STENCIL);
		ComPtr<ID3D11Texture2D> texture;
		result = device->CreateTexture2D(&desc, nullptr, &texture);
		Check(result);
		depthStencilTexture.LoadTexture(texture.Get(), ETextureUsage::DSV);
	}
	
}

void DefferdRenderer::Render()
{
	for (auto& drawCommand : allDrawCommandsOrigin)
	{
		if (drawCommand.GetMaterialData().pixelShader.isForward)
		{
			forwardDrawCommands.emplace_back(&drawCommand);
		}
		else
		{
			deferredDrawCommands.emplace_back(&drawCommand);
		}
		allDrawCommands.emplace_back(&drawCommand);
	}


	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)width, (float)height);

	ID3D11RenderTargetView* renderBuffersRTV[4];
	for (size_t i = 0; i < std::size(renderBuffersRTV); i++)
	{
		renderBuffersRTV[i] = renderBuffers[i];
	}
	immediateContext->OMSetRenderTargets(4, renderBuffersRTV, depthStencilTexture);
	immediateContext->OMSetDepthStencilState(noWriteDSS.Get(), 0);
	immediateContext->RSSetViewports(1, &viewport);


	for (size_t i = 0; i < std::size(renderBuffers); i++)
	{
		immediateContext->ClearRenderTargetView(renderBuffers[i], DirectX::SimpleMath::Color{ 0.0f,0.0f ,0.0f ,0.0f });
	}
	immediateContext->ClearDepthStencilView(depthStencilTexture, D3D11_CLEAR_DEPTH, 1.0f, 0);


	
	//ProcessDrawCommands(allDrawCommands, false);


	CameraBufferData cameraData{};
	cameraData.MainCamPos = cameraWorld.Translation();
	cameraData.IPM = DirectX::XMMatrixTranspose(cameraProjection.Invert());
	cameraData.IVM = DirectX::XMMatrixTranspose(cameraWorld);
	cameraData.Projection = DirectX::XMMatrixTranspose(cameraProjection);
	cameraData.View = DirectX::XMMatrixTranspose(cameraWorld.Invert());

	cameraBuffer.Update(cameraData);
	BindBinadble(cameraBinadbleVS);
	BindBinadble(cameraBinadblePS);

	ID3D11RenderTargetView* backBuffersRTV[1] = { *renderTarget };
	immediateContext->OMSetRenderTargets(std::size(backBuffersRTV), backBuffersRTV, nullptr);
	immediateContext->ClearRenderTargetView(*renderTarget, DirectX::SimpleMath::Color{ 0.0f,1.0f ,0.0f ,0.0f });

	for (auto& item : bindables)
	{
		BindBinadble(item);
	}

	ProcessDrawCommands(deferredDrawCommands);
	ProcessDrawCommands(forwardDrawCommands);




	allDrawCommands.clear();
	deferredDrawCommands.clear();
	forwardDrawCommands.clear();
	allDrawCommandsOrigin.clear();

	bindables.clear();
}

void DefferdRenderer::SetCameraMatrix(const Matrix& world)
{
	cameraWorld = world;
}

void DefferdRenderer::SetProjection(float fov, float nearZ, float farZ)
{
	this->fov = fov;
	this->nearZ = nearZ;
	this->farZ = farZ;

	cameraProjection = DirectX::XMMatrixPerspectiveFovLH(fov, (float)width / (float)height, nearZ, farZ);
}

void DefferdRenderer::ProcessDrawCommands(std::vector<MeshDrawCommand*>& drawCommands, bool isWithMaterial)
{
	for (auto& command : drawCommands)
	{
		auto mesh = command->GetMeshData();
		auto material = command->GetMaterialData();

		ID3D11Buffer* vertexBuffer[1] = { mesh.vertexBuffer };
		UINT stride = mesh.vertexStride;
		UINT offset = 0;

		immediateContext->IASetVertexBuffers(0, std::size(vertexBuffer), vertexBuffer, &mesh.vertexStride, &offset);
		immediateContext->IASetIndexBuffer(mesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		immediateContext->IASetInputLayout(mesh.vertexShader);
		immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		BindBinadble(mesh.transformBuffer);

		immediateContext->VSSetShader(mesh.vertexShader, nullptr, 0);
		if (isWithMaterial)
		{
			immediateContext->PSSetShader(material.pixelShader, nullptr, 0);

			for (auto& resource : material.shaderResources)
			{
				BindBinadble(resource);
			}
			for (auto& texture : material.texture)
			{
				ID3D11ShaderResourceView* textures[1] = { texture };
				immediateContext->PSSetShaderResources(0, std::size(textures), textures);
			}
		}


		immediateContext->DrawIndexed(mesh.indexCounts, 0, 0);
	}
}

struct BindHelper
{
	using BindConstantBufferFunction = std::function<void(ID3D11DeviceContext*, UINT, UINT, ID3D11Buffer* const*)>;
	using BindShaderResourceFunction = std::function<void(ID3D11DeviceContext*, UINT, UINT, ID3D11ShaderResourceView* const*)>;
	static BindConstantBufferFunction ConstantBufferBindFunction[EShaderType::MAX];
	static BindShaderResourceFunction ShaderResourceBindFunction[EShaderType::MAX];

	template<EShaderBindable::Type shaderBindable>
	static auto Get()
	{
		if constexpr (shaderBindable == EShaderBindable::ConstantBuffer)
		{
			return ConstantBufferBindFunction;
		}
		else if constexpr (shaderBindable == EShaderBindable::ShaderResource)
		{
			return ShaderResourceBindFunction;
		}
	}
};

void DefferdRenderer::BindBinadble(const Binadble& bindable)
{
	switch (bindable.bindableType)
	{
	case EShaderBindable::ConstantBuffer:
	{
		ComPtr<ID3D11Buffer> buffer;
		bindable.bind.As(&buffer);

		ID3D11Buffer* buffers[1] = { buffer.Get() };
		auto& function = BindHelper::Get<EShaderBindable::ConstantBuffer>()[bindable.shaderType];
		std::invoke(function, immediateContext.Get(), bindable.slot, std::size(buffers), buffers);
	}
	break;

	case EShaderBindable::ShaderResource:
	{
		ComPtr<ID3D11ShaderResourceView> srv;
		bindable.bind.As(&srv);

		ID3D11ShaderResourceView* resources[1] = { srv.Get() };
		auto& function = BindHelper::Get<EShaderBindable::ShaderResource>()[bindable.shaderType];
		std::invoke(function, immediateContext.Get(), bindable.slot, std::size(resources), resources);
	}
	break;

	default:
		
		break;
	}

}

BindHelper::BindConstantBufferFunction BindHelper::ConstantBufferBindFunction[EShaderType::MAX] =
{
	&ID3D11DeviceContext::VSSetConstantBuffers,
	&ID3D11DeviceContext::PSSetConstantBuffers,
	&ID3D11DeviceContext::GSSetConstantBuffers,
	&ID3D11DeviceContext::CSSetConstantBuffers,
	&ID3D11DeviceContext::HSSetConstantBuffers,
	&ID3D11DeviceContext::DSSetConstantBuffers
};

BindHelper::BindShaderResourceFunction BindHelper::ShaderResourceBindFunction[EShaderType::MAX] =
{
	&ID3D11DeviceContext::VSSetShaderResources,
	&ID3D11DeviceContext::PSSetShaderResources,
	&ID3D11DeviceContext::GSSetShaderResources,
	&ID3D11DeviceContext::CSSetShaderResources,
	&ID3D11DeviceContext::HSSetShaderResources,
	&ID3D11DeviceContext::DSSetShaderResources
};