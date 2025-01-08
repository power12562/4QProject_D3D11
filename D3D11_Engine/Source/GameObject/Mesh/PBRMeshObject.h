#pragma once
#include <GameObject/Base/GameObject.h>
#include <Material/PBRMaterial.h>

class PBRMeshObject : public GameObject
{
	SERIALIZED_OBJECT(PBRMeshObject)
public:
	PBRMeshObject() = default;
	virtual void Start() override;

	virtual ~PBRMeshObject() override = default;
	virtual void Serialized(std::ofstream& ofs);
	virtual void Deserialized(std::ifstream& ifs);

public:
	inline const char* GetMaterialName() { return materialName.c_str(); }
	void SetMaterial(const char* name);
	inline cb_PBRMaterial& GetMaterial() { return *material; }
	_declspec (property(get = GetMaterial)) cb_PBRMaterial& Material;
private:
	std::shared_ptr<cb_PBRMaterial> material;
	std::string materialName;
};