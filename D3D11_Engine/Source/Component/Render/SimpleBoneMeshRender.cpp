#include "SimpleBoneMeshRender.h"
#include <Material\BlingPhongMaterial.h>
#include <Light\SimpleDirectionalLight.h>
#include <Manager/ResourceManager.h>
#include <Math/Mathf.h>
#include <Utility/SerializedUtility.h>
#include <GameObject/Bone/BoneObject.h>
#include <Utility/ExceptionUtility.h>

SimpleBoneMeshRender::SimpleBoneMeshRender()
{

}

SimpleBoneMeshRender::~SimpleBoneMeshRender()
{
	
}

void SimpleBoneMeshRender::Serialized(std::ofstream& ofs)
{
	if (typeid(gameObject) == typeid(GameObject))
		return;

	ID3D11Device* pDevice = RendererUtility::GetDevice();
	ComPtr<ID3D11DeviceContext> pDeviceContext;
	pDevice->GetImmediateContext(&pDeviceContext);

	using namespace Binary;
	Write::data(ofs, Enable);
	Write::Color(ofs, baseColor);
	Write::data(ofs, isForward);
	Write::data(ofs, GetMeshID);
	Write::wstring(ofs, GetVSpath());
	Write::wstring(ofs, GetPSpath());

	//offsetMatrix
	std::wstring offsetMatrixKey = gameObject.Name + std::to_wstring(GetMeshID());
	Write::wstring(ofs, offsetMatrixKey);
	size_t offsetMatrixCount = offsetMatrices->data.size();
	Write::data(ofs, offsetMatrixCount);
	Write::data(ofs, offsetMatrices->data.data(), sizeof(Matrix) * offsetMatrixCount);

	D3D11_BUFFER_DESC bd{};
	ID3D11Buffer* indexBuffer = (ID3D11Buffer*)meshDrawCommand.meshData.indexBuffer;
	indexBuffer->GetDesc(&bd);
	indices.resize(bd.ByteWidth / sizeof(UINT));
	Utility::RetrieveIndexBufferData(pDeviceContext.Get(), pDevice, indexBuffer, indices.data(), bd.ByteWidth);
	Write::data(ofs, indices.size());
	ofs.write(reinterpret_cast<const char*>(indices.data()), bd.ByteWidth);
	indices.clear();
	Write::data(ofs, meshDrawCommand.meshData.indexCounts);

	ID3D11Buffer* vertexBuffer = (ID3D11Buffer*)meshDrawCommand.meshData.vertexBuffer;
	vertexBuffer->GetDesc(&bd);
	vertices.resize(bd.ByteWidth / sizeof(Vertex));
	Utility::RetrieveVertexBufferData(pDeviceContext.Get(), pDevice, vertexBuffer, vertices.data(), bd.ByteWidth);
	Write::data(ofs, vertices.size());
	ofs.write(reinterpret_cast<const char*>(vertices.data()), bd.ByteWidth);
	vertices.clear();
	Write::data(ofs, meshDrawCommand.meshData.vertexStride);

	Write::wstring(ofs, materialAsset.GetAssetPath());
	materialAsset.SaveAsset();

	vertices.shrink_to_fit();
	indices.shrink_to_fit();
}

void SimpleBoneMeshRender::Deserialized(std::ifstream& ifs)
{
	if (typeid(gameObject) == typeid(GameObject))
		return;

	using namespace Binary;
	Enable = Read::data<bool>(ifs);
	baseColor = Read::Color(ifs);
	isForward = Read::data<bool>(ifs);
	SetMeshResource(Read::data<int>(ifs)); //meshID
	SetVS(Read::wstring(ifs).c_str());
	SetPS(Read::wstring(ifs).c_str());

	//offsetMatrix
	std::wstring offsetMatrixKey = Read::wstring(ifs);
	offsetMatrices = GetResourceManager<OffsetMatrices>().GetResource(offsetMatrixKey.c_str());
	size_t offsetMatrixCount = Read::data<size_t>(ifs);
	if (offsetMatrices.use_count() == 1)
	{
		offsetMatrices->data.resize(offsetMatrixCount);
		Read::data(ifs, offsetMatrices->data.data(), sizeof(Matrix) * offsetMatrixCount);
	}
	else
	{
		ifs.seekg(sizeof(Matrix) * offsetMatrixCount, std::ios::cur);
	}
		
	size_t indicesSize = Read::data<size_t>(ifs);
	if ((ID3D11Buffer*)meshDrawCommand.meshData.indexBuffer == nullptr)
	{
		indices.resize(indicesSize);
		ifs.read(reinterpret_cast<char*>(indices.data()), sizeof(decltype(indices[0])) * indicesSize);
	}
	else
		ifs.seekg(sizeof(decltype(indices[0])) * indicesSize, std::ios::cur);
	meshDrawCommand.meshData.indexCounts = Read::data<int>(ifs);

	size_t verticesSize = Read::data<size_t>(ifs);
	if ((ID3D11Buffer*)meshDrawCommand.meshData.vertexBuffer == nullptr)
	{
		vertices.resize(verticesSize);
		ifs.read(reinterpret_cast<char*>(vertices.data()), sizeof(Vertex) * verticesSize);
	}
	else
		ifs.seekg(sizeof(Vertex) * verticesSize, std::ios::cur);
	meshDrawCommand.meshData.vertexStride = Read::data<decltype(meshDrawCommand.meshData.vertexStride)>(ifs);

	if (!indices.empty() && !vertices.empty())
		CreateMesh();

	materialAsset.OpenAsset(Read::wstring(ifs).c_str());

	vertices.shrink_to_fit();
	indices.shrink_to_fit();
}

void SimpleBoneMeshRender::Start()
{
	using namespace Utility;

	// Create Liner Sampler
	D3D11_SAMPLER_DESC SamplerDesc = {};
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	materialAsset.SetSamplerState(SamplerDesc, 0);

	//Shadow Sampler
	SamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	materialAsset.SetSamplerState(SamplerDesc, 1);
}

void SimpleBoneMeshRender::UpdateMeshDrawCommand()
{
	Matrix temp{};
	//카메라 컬링 확인 필요할듯..?
	//본 행렬 업데이트
	for (int i = 0; i < boneList.size(); i++)
	{
		temp = offsetMatrices->data[i] * boneList[i]->GetBoneMatrix();
		matrixPallete.MatrixPalleteArray[i] = XMMatrixTranspose(temp);

		temp = XMMatrixInverse(nullptr, temp);
		temp = Utility::XMMatrixIsNaN(temp) ? Matrix() : temp;
		matrixPallete.BoneWIT[i] = temp;
	}
	matrixPalleteConstBuffer.Set(matrixPallete);
	meshDrawCommand.meshData.shaderResources.clear();
	meshDrawCommand.meshData.shaderResources.push_back(
		Binadble
		{
			.shaderType = EShaderType::Vertex,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 3,
			.bind = (ID3D11Buffer*)matrixPalleteConstBuffer
		}
	);
}


void SimpleBoneMeshRender::CreateMesh()
{
	using namespace Utility;
	if (vertices.empty() || indices.empty())
		return;

	ID3D11Device* pDevice = RendererUtility::GetDevice();

	//버텍스 버퍼 생성
	meshDrawCommand.meshData.vertexStride = sizeof(Vertex);

	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = sizeof(Vertex) * vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices.data();

	ComPtr<ID3D11Buffer> vertexBuffer;
	CheckHRESULT(pDevice->CreateBuffer(&bd, &vbData, &vertexBuffer));
	meshDrawCommand.meshData.vertexBuffer.Load(vertexBuffer);

	//인덱스 버퍼 생성
	meshDrawCommand.meshData.indexCounts = indices.size();
	bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(UINT) * indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices.data();

	ComPtr<ID3D11Buffer> indexBuffer;
	CheckHRESULT(pDevice->CreateBuffer(&bd, &ibData, &indexBuffer));
	meshDrawCommand.meshData.indexBuffer.Load(indexBuffer);

	//Create bounding box
	BoundingBox box;
	box.CreateFromPoints(box, vertices.size(), reinterpret_cast<XMFLOAT3*>(vertices.data()), sizeof(Vertex));
	if (gameObject.Bounds.Center.x < Mathf::FLOAT_MAX)
	{
		BoundingBox::CreateMerged(gameObject.Bounds, gameObject.Bounds, box);
	}
	else
		gameObject.Bounds = box;

	if (Transform* root = gameObject.transform.RootParent)
	{
		box.Transform(box, transform.scale.x, transform.rotation, transform.position);
		if (root->gameObject.Bounds.Center.x < Mathf::FLOAT_MAX)
		{
			BoundingBox::CreateMerged(root->gameObject.Bounds, root->gameObject.Bounds, box);
		}
		else
			root->gameObject.Bounds = box;
	}

	vertices.clear();
	indices.clear();

	vertices.shrink_to_fit();
	indices.shrink_to_fit();
}

void SimpleBoneMeshRender::AddBonesFromRoot()
{
	if (gameObject.transform.RootParent)
	{
		Transform* root = gameObject.transform.RootParent;
		boneList.clear();
		std::stack<Transform*> transformStack;
		for (unsigned int i = 0; i < root->GetChildCount(); i++)
		{
			transformStack.push(root->GetChild(i));
		}
		while (!transformStack.empty())
		{
			Transform* cur = transformStack.top();
			transformStack.pop();
			if(typeid(BoneObject) == typeid(cur->gameObject))
			{
				BoneObject& curBone = static_cast<BoneObject&>(cur->gameObject);
				if (curBone.myIndex < 0)
				{
					__debugbreak(); //인덱스가 잘못됨.
					throw_GameObject("Invalid bone index", &this->gameObject);
				}
				if (boneList.size() <= curBone.myIndex)
				{
					boneList.resize((size_t)curBone.myIndex + 1);			
				}
				boneList[curBone.myIndex] = &curBone;
			}			 

			for (unsigned int i = 0; i < cur->GetChildCount(); i++)
			{
				transformStack.push(cur->GetChild(i));
			}
		}
	}
}
