#include "BlingPhongBoneMeshRender.h"
#include <Light/PBRDirectionalLight.h>
#include <Manager/HLSLManager.h>

void BlingPhongBoneMeshRender::Start()
{
	SimpleBoneMeshRender::Start();
	RenderFlags |= RENDER_FORWARD;

	BlingPhongMeshObject* meshObj = dynamic_cast<BlingPhongMeshObject*>(&gameObject);
	if (meshObj)
	{
		int index = constBuffer.CreatePSConstantBuffers<cb_PBRDirectionalLight>(DirectionalLight::DirectionalLights_key);

		std::string materialKey = meshObj->GetNameToString();
		index = constBuffer.CreatePSConstantBuffers<cb_BlingPhongMaterial>(materialKey.c_str());
		{
			using namespace std::string_literals;
			std::wstring vertexPath(HLSLManager::EngineShaderPath + L"VertexSkinningShader.hlsl"s);
			SetVertexShader(vertexPath.c_str());

			std::wstring pixelPath(HLSLManager::EngineShaderPath + L"BlingPhongPixelShader.hlsl"s);
			SetPixelShader(pixelPath.c_str());
		}
	}
}
