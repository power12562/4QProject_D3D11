#include "CubeObject.h"
#include <framework.h>

void CubeObject::Start()
{
	PBRMeshObject::Start();
	AddComponent<CubeMeshRender>();
}
