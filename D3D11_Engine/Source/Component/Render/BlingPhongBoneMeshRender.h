#pragma once
#include <GameObject/Mesh/BlingPhongMeshObject.h>
#include <Component/Render/SimpleBoneMeshRender.h>

class BlingPhongBoneMeshRender : public SimpleBoneMeshRender
{
public:
	BlingPhongBoneMeshRender() = default;
	virtual ~BlingPhongBoneMeshRender() override = default;

	virtual void Start() override;
	virtual void UpdateMeshDrawCommand() override;
	BlingPhongMeshObject& GetMeshObject() { return *meshObj; }
private:
	BlingPhongMeshObject* meshObj = nullptr;
};