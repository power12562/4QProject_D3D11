#pragma once
#include <GameObject/Base/GameObject.h>
#include <Material/BlingPhongMaterial.h>

class BlingPhongMeshObject : public GameObject
{
	SERIALIZED_OBJECT(BlingPhongMeshObject)
public:
	BlingPhongMeshObject() = default;
	virtual ~BlingPhongMeshObject() override = default;
	virtual void Start() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;
public:
	void SetMaterial(const char* name);
	inline cb_BlingPhongMaterial& GetMaterial() { return *material; }
	_declspec (property(get = GetMaterial)) cb_BlingPhongMaterial& Material;
private:
	std::shared_ptr<cb_BlingPhongMaterial> material;
	std::string materialName;
};