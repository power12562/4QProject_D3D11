#pragma once
#include <GameObject/Mesh/PBRMeshObject.h>

class CubeObject : public PBRMeshObject
{
	SERIALIZED_OBJECT(CubeObject)
public:
	CubeObject() = default;
	virtual ~CubeObject() override = default;
	virtual void Start();
};