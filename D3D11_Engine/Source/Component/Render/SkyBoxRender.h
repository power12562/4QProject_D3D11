#pragma once
#include <Component/Render/MeshRender.h>

class SkyBoxRender : public MeshRender
{
	inline static SkyBoxRender* mainSkyBox = nullptr;
public:
	static SkyBoxRender* GetMainSkyBox();
	enum TEXTURE_TYPE
	{
		ENV,
		BRDF_LUT = 30,
		Diffuse_IBL = 31,
		Specular_IBL = 32,
		Count,
	};

public:
	SkyBoxRender() = default;
	virtual ~SkyBoxRender() override;

	virtual void Start() override;
protected:
	virtual void FixedUpdate() override;
	virtual void Update() override;
	virtual void LateUpdate() override;
	virtual void UpdateMeshDrawCommand() override;

public:
	void SetSkyBox(TEXTURE_TYPE type, const wchar_t* path);
	void ResetSkyBox(TEXTURE_TYPE type);

private:
	virtual void CreateMesh() override;

private:
	std::vector<Vector4> vertices;
	std::vector<UINT> indices;
};