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
		Diffuse_IBL,
		Specular_IBL,
		BRDF_LUT,
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
	virtual void Render() override;

public:
	void SetSkyBox(TEXTURE_TYPE type, const wchar_t* path);
	void ResetSkyBox(TEXTURE_TYPE type);

private:
	virtual void CreateMesh() override;

private:
	std::vector<Vector4> vertices;
	std::vector<UINT> indices;
};