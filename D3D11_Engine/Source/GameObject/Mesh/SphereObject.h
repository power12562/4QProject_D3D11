#pragma once
#include <GameObject/Mesh/PBRMeshObject.h>
#include <Material/PBRMaterial.h>
class PBRMeshRender;
class SphereObject : public PBRMeshObject
{
	SERIALIZED_OBJECT(SphereObject)
public:
	SphereObject() = default;
	virtual void Start() override;
	virtual ~SphereObject() override = default;

	PBRMeshRender* SphereMeshRender = nullptr;
};
