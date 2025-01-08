#include "SimpleBoneMeshRender.h"
#include <D3DCore/D3DRenderer.h>
#include <Material\BlingPhongMaterial.h>
#include <Light\SimpleDirectionalLight.h>
#include <Manager/ResourceManager.h>
#include <Math/Mathf.h>
#include <Utility/SerializedUtility.h>
#include <GameObject/Bone/BoneObject.h>
#include <Utility/ExceptionUtility.h>

SimpleBoneMeshRender::SimpleBoneMeshRender()
{
	RenderFlags |= RENDER_SKINNING;
	matrixPalleteKey = MakeMatrixPalleteKey();
	matrixPallete = D3DConstBuffer::GetData<MatrixPallete>(matrixPalleteKey.c_str());
	matrixPalleteRegisterIndex = constBuffer.CreateVSConstantBuffers<MatrixPallete>(matrixPalleteKey.c_str());
}

SimpleBoneMeshRender::~SimpleBoneMeshRender()
{
	SimpleBoneMeshRender::RealseCounterQueue.push(this->MyMatrixPalletCounter);
}

void SimpleBoneMeshRender::Serialized(std::ofstream& ofs)
{
	if (typeid(gameObject) == typeid(GameObject))
		return;

	using namespace Binary;
	Write::data(ofs, Enable);
	Write::Color(ofs, baseColor);
	Write::data(ofs, RenderFlags);
	Write::data(ofs, MeshID);
	Write::wstring(ofs, GetVertexShaderPath());
	Write::wstring(ofs, GetPixelShaderPath());

	//offsetMatrix
	std::wstring offsetMatrixKey = gameObject.Name + std::to_wstring(MeshID);
	Write::wstring(ofs, offsetMatrixKey);
	size_t offsetMatrixCount = offsetMatrices->data.size();
	Write::data(ofs, offsetMatrixCount);
	Write::data(ofs, offsetMatrices->data.data(), sizeof(Matrix) * offsetMatrixCount);

	D3D11_BUFFER_DESC bd{};
	meshResource->pIndexBuffer->GetDesc(&bd);
	indices.resize(bd.ByteWidth / sizeof(UINT));
	Utility::RetrieveIndexBufferData(d3dRenderer.GetDeviceContext(), d3dRenderer.GetDevice(), meshResource->pIndexBuffer, indices.data(), bd.ByteWidth);
	Write::data(ofs, indices.size());
	ofs.write(reinterpret_cast<const char*>(indices.data()), bd.ByteWidth);
	indices.clear();
	Write::data(ofs, meshResource->indicesCount);

	meshResource->pVertexBuffer->GetDesc(&bd);
	vertices.resize(bd.ByteWidth / sizeof(Vertex));
	Utility::RetrieveVertexBufferData(d3dRenderer.GetDeviceContext(), d3dRenderer.GetDevice(), meshResource->pVertexBuffer, vertices.data(), bd.ByteWidth);
	Write::data(ofs, vertices.size());
	ofs.write(reinterpret_cast<const char*>(vertices.data()), bd.ByteWidth);
	vertices.clear();
	Write::data(ofs, meshResource->vertexBufferOffset);
	Write::data(ofs, meshResource->vertexBufferStride);

	textures.Serialized(ofs);
	constBuffer.Serialized(ofs);

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
	RenderFlags = Read::data<int>(ifs);
	MeshID = Read::data<int>(ifs);   SetMeshResource(gameObject.Name.c_str());
	SetVertexShader(Read::wstring(ifs).c_str());
	SetPixelShader(Read::wstring(ifs).c_str());

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
	if (meshResource->pIndexBuffer == nullptr)
	{
		indices.resize(indicesSize);
		ifs.read(reinterpret_cast<char*>(indices.data()), sizeof(decltype(indices[0])) * indicesSize);
	}
	else
		ifs.seekg(sizeof(decltype(indices[0])) * indicesSize, std::ios::cur);
	meshResource->indicesCount = Read::data<int>(ifs);

	size_t verticesSize = Read::data<size_t>(ifs);
	if (meshResource->pVertexBuffer == nullptr)
	{
		vertices.resize(verticesSize);
		ifs.read(reinterpret_cast<char*>(vertices.data()), sizeof(Vertex) * verticesSize);
	}
	else
		ifs.seekg(sizeof(Vertex) * verticesSize, std::ios::cur);
	meshResource->vertexBufferOffset = Read::data<decltype(meshResource->vertexBufferOffset)>(ifs);
	meshResource->vertexBufferStride = Read::data<decltype(meshResource->vertexBufferStride)>(ifs);

	if (!indices.empty() && !vertices.empty())
		CreateMesh();

	textures.Deserialized(ifs);
	constBuffer.Deserialized(ifs);

	vertices.shrink_to_fit();
	indices.shrink_to_fit();

	DeserializedListVec.push_back(this);
}

void SimpleBoneMeshRender::EndDeserialized()
{
	for (auto& item : SimpleBoneMeshRender::DeserializedListVec)
	{
		item->AddBonesFromRoot();
		item->constBuffer.ChangeVSkey<MatrixPallete>(item->matrixPalleteRegisterIndex, item->matrixPalleteKey.c_str());
	}
	SimpleBoneMeshRender::DeserializedListVec.clear();
	SimpleBoneMeshRender::DeserializedListVec.shrink_to_fit();
}

void SimpleBoneMeshRender::Start()
{
	using namespace Utility;

	// Create Liner Sampler
	samplerState.resize(2);
	D3D11_SAMPLER_DESC SamplerDesc = {};
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerState.SetSamplerState(0, SamplerDesc);

	//Shadow Sampler
	SamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerState.SetSamplerState(1, SamplerDesc);
}

void SimpleBoneMeshRender::FixedUpdate()
{

}

void SimpleBoneMeshRender::Update()
{

}

void SimpleBoneMeshRender::LateUpdate()
{
}

void SimpleBoneMeshRender::Render()
{
	if (meshResource->pVertexBuffer && meshResource->pIndexBuffer && matrixPallete)
	{
		if (IsVSShader() && IsPSShader())
		{
			RENDERER_DRAW_DESC desc = GetRendererDesc();
			d3dRenderer.DrawIndex(desc);
		}

		if (gameObject.IsCameraCulling())
		{
			Matrix temp{};
			for (int i = 0; i < boneList.size(); i++)
			{
				temp = offsetMatrices->data[i] * boneList[i]->GetBoneMatrix();
				matrixPallete->MatrixPalleteArray[i] = XMMatrixTranspose(temp);

				temp = XMMatrixInverse(nullptr, temp);
				temp = Utility::XMMatrixIsNaN(temp) ? Matrix() : temp;
				matrixPallete->BoneWIT[i] = temp;
			}
		}
	}
	else
	{
		__debugbreak(); //데이터 없음.
	}
}

void SimpleBoneMeshRender::CreateMesh()
{
	using namespace Utility;
	if (vertices.empty() || indices.empty())
		return;

	if (meshResource->pIndexBuffer)
		SafeRelease(meshResource->pIndexBuffer);

	if (meshResource->pVertexBuffer)
		SafeRelease(meshResource->pVertexBuffer);

	meshResource->vertexBufferOffset = 0;
	meshResource->vertexBufferStride = sizeof(Vertex);

	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = sizeof(Vertex) * vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices.data();
	CheckHRESULT(d3dRenderer.GetDevice()->CreateBuffer(&bd, &vbData, &meshResource->pVertexBuffer));

	meshResource->indicesCount = indices.size();
	bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(UINT) * indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices.data();
	CheckHRESULT(d3dRenderer.GetDevice()->CreateBuffer(&bd, &ibData, &meshResource->pIndexBuffer));
	
	//Create bounding box
	BoundingBox box;
	box.CreateFromPoints(box, vertices.size(), reinterpret_cast<XMFLOAT3*>(vertices.data()), sizeof(Vertex));
	if (gameObject.Bounds.Center.x < Mathf::FLOAT_MAX)
	{
		BoundingBox::CreateMerged(gameObject.Bounds, gameObject.Bounds, box);
	}
	else
		gameObject.Bounds = box;


	if (Transform* root = transform.RootParent)
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
