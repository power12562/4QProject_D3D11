#include "CubeObject.h"
#include <framework.h>
#include <Utility/SerializedUtility.h>

void CubeObject::Start()
{
	PBRMeshObject::Start();
	AddComponent<CubeMeshRender>();
}

void CubeObject::Serialized(std::ofstream& ofs)
{
	using namespace Binary;
	Write::wstring(ofs, materialName);
	Write::PBRMaterial(ofs, *material);
}

void CubeObject::Deserialized(std::ifstream& ifs)
{
	using namespace Binary;
	SetMaterial(Read::wstring(ifs).c_str());
	*material = Read::PBRMaterial(ifs);
}
