#include "BlingPhongMeshObject.h"
#include <D3DCore/D3DConstBuffer.h>
#include <Utility/SerializedUtility.h>
#include <Component/Render/BlingPhongBoneMeshRender.h>
#include <Component/Render/BlingPhongMeshRender.h>

void BlingPhongMeshObject::Start()
{
	SetMaterial(GetNameToString().c_str());
}

void BlingPhongMeshObject::SetMaterial(const char* name)
{
	materialName = name;
	material = D3DConstBuffer::GetData<cb_BlingPhongMaterial>(name);
}

void BlingPhongMeshObject::Serialized(std::ofstream& ofs)
{
	using namespace Binary;
	Write::string(ofs, materialName);
	Write::data(ofs, material.get(), sizeof(cb_BlingPhongMaterial));
	size_t MeshCount = 0;
	for (size_t i = 0; i < GetComponentCount(); i++)
	{
		if (GetComponentAtIndex<BlingPhongMeshRender>(i))
		{
			MeshCount++;
		}
	}
	if (MeshCount > 0)
	{
		Write::data(ofs, MeshCount);
		Write::string(ofs, typeid(BlingPhongMeshRender).name());
	}
	else
	{
		size_t boneMeshCount = 0;
		for (size_t i = 0; i < GetComponentCount(); i++)
		{
			if (GetComponentAtIndex<BlingPhongBoneMeshRender>(i))
			{
				boneMeshCount++;
			}
		}
		if (boneMeshCount > 0)
		{
			Write::data(ofs, boneMeshCount);
			Write::string(ofs, typeid(BlingPhongBoneMeshRender).name());
		}
		else
		{
			Write::data(ofs, size_t{ 0 });
		}
	}
}

void BlingPhongMeshObject::Deserialized(std::ifstream& ifs)
{
	using namespace Binary;
	SetMaterial(Read::string(ifs).c_str());
	Read::data(ifs, material.get(), sizeof(cb_BlingPhongMaterial));
	size_t meshCount = Read::data<size_t>(ifs);
	if (meshCount > 0)
	{
		std::string type = Read::string(ifs);
		if (type == typeid(BlingPhongMeshRender).name())
		{
			for (size_t i = 0; i < meshCount; i++)
			{
				AddComponent<BlingPhongMeshRender>();
			}
		}
		else
		{
			for (size_t i = 0; i < meshCount; i++)
			{
				AddComponent<BlingPhongBoneMeshRender>();
			}
		}
	}
}