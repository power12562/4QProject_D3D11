#pragma once
#include <Component/Base/RenderComponent.h>
#include <string>
#include <Asset/MaterialAsset.h>

struct TransformBufferData
{
	alignas(16) Matrix World;
	alignas(16) Matrix WorldInverseTranspose;
};

class MeshRender : public RenderComponent
{
	inline static std::list<MeshRender*> instanceList;
	std::list<MeshRender*>::iterator myIter;
public:
	static void ReloadShaderAll();

public:
	MeshRender();
	virtual ~MeshRender() override;
public:
	virtual void Start() = 0;

	/// ���ο� �������� �Է��� ���� ������� �׽�Ʈ������ ���Ɗx���ϴ�
	/// Begin IRenderer Data
	virtual struct MeshDrawCommand GetMeshDrawCommand() override { return meshDrawCommand; }

protected:
	MeshDrawCommand				meshDrawCommand;
	ConstantBuffer				transformBuffer;
	ConstantBuffer				material;

public:
	MaterialAsset materialAsset;

protected:
	virtual void FixedUpdate() = 0;
	virtual void Update() = 0;
	virtual void LateUpdate() = 0;
	virtual void Render() final;
public:
	virtual void CreateMesh() = 0;
	virtual void UpdateMeshDrawCommand() = 0;

public:
	void SetVS(const wchar_t* path);
	inline const std::wstring& GetVSpath() { return currVSpath; }

	void SetPS(const wchar_t* path);
	inline const std::wstring& GetPSpath() { return currPSpath; }

private:
	std::wstring currVSpath;
	std::wstring currPSpath;

public:
	void SetMeshResource(int meshID);
	inline int GetMeshID() const { return MeshID; }
private:
	int MeshID = -1;

public:
	/*�޽� ���͸����� BaseColor ����. CopyFBX���� �ش� �÷��� �����ذ��ϴ�.*/
	DirectX::SimpleMath::Color baseColor{1,1,1,1};
};