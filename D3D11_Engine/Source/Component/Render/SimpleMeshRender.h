#pragma once
#include <Component\Render\MeshRender.h>
#include <functional>
#include <ConstantBuffer.h>

class SimpleMaterial;
class SimpleMeshRender : public MeshRender
{
public:
	struct Vertex
	{
		Vector4 position{0,0,0,1};
		Vector3 normal;
		Vector3 Tangent;
		Vector2 Tex;
	};
public:
	SimpleMeshRender();
	virtual ~SimpleMeshRender() override = default;
	virtual void Serialized(std::ofstream& ofs);
	virtual void Deserialized(std::ifstream& ifs);

public:
	virtual void Start() 		 override;
protected:						 
	virtual void FixedUpdate()	 override {};
	virtual void Update() 		 override {};
	virtual void LateUpdate()	 override {};
	virtual void UpdateMeshDrawCommand() override {};

public:
	void CreateMesh();

public:
	std::vector<Vertex> vertices;	
	std::vector<UINT> indices;	
};
