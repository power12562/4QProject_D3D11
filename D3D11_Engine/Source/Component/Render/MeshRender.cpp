#include "MeshRender.h"
#include <Manager/ResourceManager.h>
#include <Manager/HLSLManager.h>
#include <Utility/MemoryUtility.h>
#include <ranges>

using namespace Utility;

void MeshRender::ReloadShaderAll()
{
	hlslManager.ClearSharingShader();
	for (auto& item : instanceList)
	{
		if (!item->vertexShaderPath.empty())
			item->SetVertexShader(item->vertexShaderPath.c_str());

		if (!item->pixelShaderPath.empty())
			item->SetPixelShader(item->pixelShaderPath.c_str());
	}
}

MeshRender::MeshRender()
{
	RenderFlags = 0;
	instanceList.push_back(this);
	myIter = --(instanceList.end());

	//레스터화 기본 규칙
	ZeroMemory(&currRRdesc, sizeof(currRRdesc));
	currRRdesc.FillMode = D3D11_FILL_SOLID;
	currRRdesc.CullMode = D3D11_CULL_NONE;  //컬링 없음
	currRRdesc.FrontCounterClockwise = false; //기본값
}

MeshRender::~MeshRender()
{
	if (meshResource.use_count() == 1)
	{
		SafeRelease(meshResource->pIndexBuffer);
		SafeRelease(meshResource->pVertexBuffer);
	}
	SafeRelease(pRRState);
	SafeRelease(pInputLayout);
	SafeRelease(pVertexShader);
	SafeRelease(pPixelShader);

	instanceList.erase(myIter);
}

void MeshRender::UpdateMeshDrawCommand()
{
	TransformBufferData transformData
	{
		.World = XMMatrixTranspose(gameObject.transform.GetWM()),
		.WorldInverseTranspose = gameObject.transform.GetWM().Invert()
	};
	transformBuffer.Set(transformData);

	MeshData meshData{};
	MaterialData materialData{};

	meshData.vertexBuffer.Load(meshResource->pVertexBuffer);
	meshData.indexBuffer.Load(meshResource->pIndexBuffer);

	meshData.indexCounts = meshResource->indicesCount;
	meshData.vertexStride = meshResource->vertexBufferStride;
	meshData.boundingBox = gameObject.Bounds;
	meshData.transformBuffer =
		Binadble
		{
			.shaderType = EShaderType::Vertex,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)transformBuffer
		};

	meshData.vertexShader.LoadShader(pVertexShader, pInputLayout);
	materialData.pixelShader.LoadShader(pPixelShader);
	materialData.shaderResources.clear();

	auto texturesView =
		std::views::iota(0)
		| std::views::take(std::min(texturesSlot.size(), texturesV2.size()))
		| std::views::transform([&](int i) -> Binadble
								{
									return Binadble
									{
										.shaderType = EShaderType::Pixel,
										.bindableType = EShaderBindable::ShaderResource,
										.slot = texturesSlot[i],
										.bind = (ID3D11ShaderResourceView*)texturesV2[i]
									};
								});

	std::ranges::copy(texturesView, std::back_inserter(materialData.shaderResources));

	meshDrawCommand << meshData;
	meshDrawCommand << materialData;
}

void MeshRender::SetMeshResource(const wchar_t* path)
{
	if (MeshID < 0)
	{
		__debugbreak();
		return;
	}

	using namespace utfConvert;
	if (meshResource == nullptr)
	{
		//고유의 경로 + index
		meshResource = GetResourceManager<DRAW_INDEX_DATA>().GetResource(path, MeshID);
	}
}

void MeshRender::CopyShader(MeshRender& rhs)
{
	//생성된 shader는 씬 파괴 시점까지 존재함. 따라서 두개의 수명은 같다.
	pInputLayout = rhs.pInputLayout;
	pVertexShader = rhs.pVertexShader;
	pPixelShader = rhs.pPixelShader;
}

void MeshRender::SetVertexShader(const wchar_t* path)
{
	SafeRelease(pVertexShader);
	SafeRelease(pInputLayout);
	hlslManager.CreateSharingShader(path, &pVertexShader, &pInputLayout);
	vertexShaderPath = path;
}

void MeshRender::SetPixelShader(const wchar_t* path)
{
	SafeRelease(pPixelShader);
	hlslManager.CreateSharingShader(path, &pPixelShader);
	pixelShaderPath = path;
	MaterialData materialData{};
	materialData.pixelShader.LoadShader(pPixelShader);

	meshDrawCommand << materialData;
}

void MeshRender::ResetVertexShader()
{
	if (pVertexShader)
	{
		pInputLayout = nullptr;
		pVertexShader = nullptr;

		vertexShaderPath.clear();
	}
}

void MeshRender::ResetPixelShader()
{
	if (pPixelShader)
	{
		pPixelShader = nullptr;

		pixelShaderPath.clear();
	}
}

void MeshRender::SetFillMode(D3D11_FILL_MODE mode)
{
	currRRdesc.FillMode = mode;
	SetRRState(currRRdesc);
}

void MeshRender::SetCullMode(D3D11_CULL_MODE mode)
{
	currRRdesc.CullMode = mode;
	SetRRState(currRRdesc);
}

void MeshRender::SetRRState(D3D11_RASTERIZER_DESC& desc)
{
	ResetRRState();
	d3dRenderer.CreateRRState(desc, &pRRState);
	currRRdesc = desc;
}

void MeshRender::ResetRRState()
{
	Utility::SafeRelease(pRRState);
}

RENDERER_DRAW_DESC MeshRender::GetRendererDesc()
{
	RENDERER_DRAW_DESC desc{};
	desc.pConstBuffer = &constBuffer;
	desc.pD3DTexture2D = &textures;
	desc.pSamperState = &samplerState;
	desc.pInputLayout = pInputLayout;
	desc.pVertexShader = pVertexShader;
	desc.pPixelShader = pPixelShader;
	desc.pTransform = &gameObject.transform;
	desc.pVertexIndex = meshResource.get();
	desc.pRRState = pRRState;
	desc.flags = this->RenderFlags;
	return desc;
}

