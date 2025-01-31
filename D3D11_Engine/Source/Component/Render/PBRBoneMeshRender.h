#pragma once
#include <Component/Render/SimpleBoneMeshRender.h>
#include <GameObject/Mesh/PBRMeshObject.h>

class PBRBoneMeshRender : public SimpleBoneMeshRender
{
public:
	PBRBoneMeshRender();
	virtual ~PBRBoneMeshRender() override = default;

	virtual void Start() override;
	virtual void UpdateMeshDrawCommand() override;

	PBRMeshObject& GetPBRMeshObject() { return *meshObject; }
private:
	PBRMeshObject* meshObject = nullptr;
};