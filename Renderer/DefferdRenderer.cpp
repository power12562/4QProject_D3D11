#include "DefferdRenderer.h"
#include <format>


#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")




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

	struct CameraBufferData
	{
		Matrix cameraWorld;
		Matrix cameraProjection;
	} cameraData;

	cameraBuffer.Init(0, &cameraData);

}

void DefferdRenderer::AddDrawCommand(_In_ DrawCommand& command)
{
	drawCommands.emplace_back(command);
}

void DefferdRenderer::SetRenderTarget(_In_ Texture& target)
{
	renderTarget = &target;

	//width = target.width;
	//height = target.width;
}

void DefferdRenderer::Render()
{

	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)width, (float)height);


	immediateContext->OMSetRenderTargets(4, renderRTV[0].GetAddressOf(), depthStencilDSV.Get());
	immediateContext->OMSetDepthStencilState(noWriteDSS.Get(), 0);
	immediateContext->RSSetViewports(1, &viewport);


	for (size_t i = 0; i < std::size(renderRTV); i++)
	{
		immediateContext->ClearRenderTargetView(renderRTV[i].Get(), DirectX::SimpleMath::Color{ 0.0f,0.0f ,0.0f ,0.0f });
	}
	immediateContext->ClearDepthStencilView(depthStencilDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);



	for (auto& command : drawCommands)
	{
		auto mesh = command.GetMeshData();
		auto material = command.GetMaterialData();
	}

	drawCommands.clear();
}

void DefferdRenderer::SetCameraMatricx(const Matrix& world)
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
