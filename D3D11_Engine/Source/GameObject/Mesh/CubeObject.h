#pragma once
#include <GameObject/Mesh/PBRMeshObject.h>

class CubeObject : public PBRMeshObject
{
	SERIALIZED_OBJECT(CubeObject)
public:
	CubeObject() = default;
	virtual ~CubeObject() override = default;
	virtual void Start();
	virtual void Serialized(std::ofstream& ofs);
	virtual void Deserialized(std::ifstream& ifs);
};