#include "MeshRender.h"
#include <Manager/ResourceManager.h>
#include <Manager/HLSLManager.h>
#include <Utility/MemoryUtility.h>
#include <ranges>
#include <Material/PBRMaterial.h>

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
	std::string MaterialKey = gameObject.GetNameToString();
	auto tempData = constBuffer.GetData<cb_PBRMaterial>(MaterialKey.c_str());

	pbrMaterial.Set(*tempData);

	MeshData meshData{};
	MaterialData materialData{};

	meshData.vertexBuffer.Load(meshResource->pVertexBuffer);
	meshData.indexBuffer.Load(meshResource->pIndexBuffer);

	meshData.indexCounts = meshResource->indicesCount;
	meshData.vertexStride = meshResource->vertexBufferStride;
	BoundingOrientedBox::CreateFromBoundingBox(meshData.boundingBox, gameObject.Bounds);
	meshData.boundingBox.Transform(meshData.boundingBox, gameObject.transform.GetWM());

	meshData.shaderResources.emplace_back(
		Binadble
		{
			.shaderType = EShaderType::Vertex,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)transformBuffer
		});

	meshData.vertexShader.LoadShader(pVertexShader, pInputLayout);
	materialData.pixelShader.LoadShader(pPixelShader);
	materialData.pixelShader.isForward = isForward;
	materialData.shaderResources.clear();

	///
	// 으음...... 마음에 들지는 않음.... 호환성을 위해....
	// Begin Temp
	auto texturesView2 =
		std::views::iota(0)
		| std::views::take(textures.size())
		| std::views::transform([&](uint32_t i) -> Binadble
								{
									return Binadble
									{
										.shaderType = EShaderType::Pixel,
										.bindableType = EShaderBindable::ShaderResource,
										.slot = i,
										.bind = textures[i]
									};
								});

	std::ranges::copy(texturesView2, std::back_inserter(materialData.shaderResources));

	auto samplersView =
		std::views::iota(0)
		| std::views::take(samplerState.size())
		| std::views::transform([&](uint32_t i) -> Binadble
								{
									return Binadble
									{
										.shaderType = EShaderType::Pixel,
										.bindableType = EShaderBindable::Sampler,
										.slot = i,
										.bind = samplerState[i]
									};
								});
	std::ranges::copy(samplersView, std::back_inserter(materialData.shaderResources));
	materialData.shaderResources.emplace_back(Binadble
											  {
												  .shaderType = EShaderType::Pixel,
												  .bindableType = EShaderBindable::ConstantBuffer,
												  .slot = 4,
												  .bind = (ID3D11Buffer*)pbrMaterial
											  });


	// End Temp

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

	meshDrawCommand.meshData = std::move(meshData);
	meshDrawCommand.materialData = std::move(materialData);
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

