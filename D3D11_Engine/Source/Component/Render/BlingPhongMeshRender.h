#pragma once
#include <GameObject/Mesh/BlingPhongMeshObject.h>
#include <Component/Render/SimpleMeshRender.h>

class BlingPhongMeshRender : public SimpleMeshRender
{
public:
	BlingPhongMeshRender() = default;
	virtual ~BlingPhongMeshRender() override = default;

	virtual void Start() override;
};