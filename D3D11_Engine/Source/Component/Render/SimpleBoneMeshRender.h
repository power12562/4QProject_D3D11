#pragma once
#include <Component/Render/MeshRender.h>
#include <format>
#include <stack>

struct MatrixPallete
{
	Matrix MatrixPalleteArray[128]; 
	Matrix BoneWIT[128];
};

struct OffsetMatrices
{
	std::vector<Matrix> data;
};

class BoneObject;
class SimpleMaterial;
class SimpleBoneMeshRender : public MeshRender
{
	friend class GameObjectFactory;
public:
	struct Vertex
	{
		Vector4 position{ 0,0,0,1 };
		Vector3 normal;
		Vector3 Tangent;
		Vector2 Tex;
		int BlendIndecses[4] = {0,};
		float BlendWeights[4] = {0,};
	};
public:
	SimpleBoneMeshRender();
	virtual ~SimpleBoneMeshRender() override;
	virtual void Serialized(std::ofstream& ofs);
	virtual void Deserialized(std::ifstream& ifs);
private:
	/*Deserialized된 항목들.*/
	inline static std::vector<SimpleBoneMeshRender*> DeserializedListVec;
	/*Deserialized 종료 후 호출해야하는 함수(GameObjectFactory에서 호출)*/
	static void EndDeserialized();

public:
	virtual void Start() 		 override;
protected:
	virtual void FixedUpdate()	 override;
	virtual void Update() 		 override;
	virtual void LateUpdate()	 override;
	virtual void Render() 		 override;

public:
	virtual void CreateMesh() override;

public:
	std::vector<Vertex> vertices;
	std::vector<UINT>   indices;
	std::shared_ptr<OffsetMatrices> offsetMatrices = nullptr;

	/*최상위 부모부터 순회하면서 Bone을 찾아 등록합니다.*/
	void AddBonesFromRoot();

private:
	inline static size_t MatrixPalletCounter = 0;
	inline static std::queue<size_t> RealseCounterQueue;
	inline std::string MakeMatrixPalleteKey();
	std::shared_ptr<MatrixPallete> matrixPallete = {};
	std::string matrixPalleteKey;
	int matrixPalleteRegisterIndex = -1;
	size_t MyMatrixPalletCounter = 0;
	std::vector<BoneObject*> boneList;
};

inline std::string SimpleBoneMeshRender::MakeMatrixPalleteKey()
{
	if (!RealseCounterQueue.empty())
	{
		MyMatrixPalletCounter = RealseCounterQueue.front();
		RealseCounterQueue.pop();
	}
	else
	{
		MyMatrixPalletCounter = MatrixPalletCounter++;
	}
	return std::format("MatrixPallete ({})", MyMatrixPalletCounter);
}

