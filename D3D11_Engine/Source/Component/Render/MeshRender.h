#pragma once
#include <Component/Base/RenderComponent.h>
#include <string>

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

	bool isForward = false;
protected:
	std::shared_ptr<MeshData> sharedMeshData;

	MeshDrawCommand meshDrawCommand;
	ConstantBuffer transformBuffer;
	ConstantBuffer material;
	std::vector<uint32_t> texturesSlot;
	std::vector<Texture> texturesV2;
protected:
	virtual void FixedUpdate() = 0;
	virtual void Update() = 0;
	virtual void LateUpdate() = 0;
	virtual void Render();

public:
	virtual void CreateMesh() = 0;

public:
	void SetMeshResource(const wchar_t* path);
	int MeshID = -1;

public:
	/*�޽� ���͸����� BaseColor ����. CopyFBX���� �ش� �÷��� �����ذ��ϴ�.*/
	DirectX::SimpleMath::Color baseColor{1,1,1,1};
};