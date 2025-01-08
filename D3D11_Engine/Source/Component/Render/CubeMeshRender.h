#pragma once
#include <Component/Render/PBRMeshRender.h>
class CubeMeshRender : public PBRMeshRender
{
public:
	CubeMeshRender() = default;
	virtual	~CubeMeshRender() override = default;

	virtual void Start() override;
};