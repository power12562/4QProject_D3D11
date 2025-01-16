#pragma once
#include <GameObject/Mesh/BlingPhongMeshObject.h>
#include <Component/Render/SimpleMeshRender.h>

class BlingPhongMeshRender : public SimpleMeshRender
{
public:
	BlingPhongMeshRender() = default;
	virtual ~BlingPhongMeshRender() override = default;

	virtual void Start() override;
	virtual void UpdateMeshDrawCommand() override;
	BlingPhongMeshObject& GetMeshObject() { return *meshObj; }
private:
	BlingPhongMeshObject* meshObj = nullptr;
};