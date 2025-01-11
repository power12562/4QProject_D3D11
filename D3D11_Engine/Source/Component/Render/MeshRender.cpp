#include "MeshRender.h"
#include <Manager/ResourceManager.h>
#include <Manager/HLSLManager.h>
#include <Utility/MemoryUtility.h>

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

	//������ȭ �⺻ ��Ģ
	ZeroMemory(&currRRdesc, sizeof(currRRdesc));
	currRRdesc.FillMode = D3D11_FILL_SOLID;
	currRRdesc.CullMode = D3D11_CULL_NONE;  //�ø� ����
	currRRdesc.FrontCounterClockwise = false; //�⺻��
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
		.WorldInverseTranspose = XMMatrixInverse(nullptr, transformData.World)
	};
	transformBuffer.Set(transformData);



	FixedMaterialData fixedMaterialData
	{
		.Metallic = metallic,
		.Specular = specular,
		.Roughness = roughness,
		.AmbientOcclusion = ao,
		.Albedo = albedo,
		.Emissive = emissive,
	};
	fixedMaterial.Set(fixedMaterialData);


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
	materialData.shaderResources.emplace_back(Binadble
											  {
												  .shaderType = EShaderType::Pixel,
												  .bindableType = EShaderBindable::ConstantBuffer,
												  .slot = 4,
												  .bind = (ID3D11Buffer*)fixedMaterial
											  });
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
		//������ ��� + index
		meshResource = GetResourceManager<DRAW_INDEX_DATA>().GetResource(path, MeshID);
	}
}

void MeshRender::CopyShader(MeshRender& rhs)
{
	//������ shader�� �� �ı� �������� ������. ���� �ΰ��� ������ ����.
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

