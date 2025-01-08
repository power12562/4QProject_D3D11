#include "AssimpUtility.h"
#include <Light/SimpleDirectionalLight.h>
#include <Manager/SceneManager.h>
#include <GameObject\Base\GameObject.h>
#include <Component\Render\SimpleMeshRender.h>
#include <Component/Render/SimpleBoneMeshRender.h>
#include <GameObject/Bone/BoneObject.h>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <Math/AssimpMath.h>
#include <Utility\utfConvert.h>
#include <Material\BlingPhongMaterial.h>
#include <Component/Animation/TransformAnimation.h>
#include <Manager\ResourceManager.h>
#include <Manager/HLSLManager.h>
#include <Manager/TextureManager.h>
#include <Utility/ImguiHelper.h>
#include <Core/WinGameApp.h>

#include <Component/Render/PBRMeshRender.h>
#include <Component/Render/PBRBoneMeshRender.h>
#include <Component/Render/BlingPhongMeshRender.h>
#include <Component/Render/BlingPhongBoneMeshRender.h>

#include <Math/Mathf.h>
#include <iostream>

namespace Utility
{
	static GameObject* NewMeshObject(
		SURFACE_TYPE surface,
		const wchar_t* name)
	{
		switch (surface)
		{
		case SURFACE_TYPE::BlingPhong:
			return NewGameObject<BlingPhongMeshObject>(name);
		case SURFACE_TYPE::PBR:
			return NewGameObject<PBRMeshObject>(name);
		default:
			return NewGameObject(name);
		}
	}

	static SimpleMeshRender& AddMeshComponent(GameObject* obj, SURFACE_TYPE surface)
	{
		switch (surface)
		{
		case SURFACE_TYPE::BlingPhong:
			return obj->AddComponent<BlingPhongMeshRender>();
		case SURFACE_TYPE::PBR:
			return obj->AddComponent<PBRMeshRender>();
		default:
			return obj->AddComponent<SimpleMeshRender>();
		}

	}

	static SimpleBoneMeshRender& AddBoneMeshComponent(GameObject* obj, SURFACE_TYPE surface)
	{
		switch (surface)
		{
		case SURFACE_TYPE::BlingPhong:
			return obj->AddComponent<BlingPhongBoneMeshRender>();
		case SURFACE_TYPE::PBR:
			return obj->AddComponent<PBRBoneMeshRender>();
		default:
			return obj->AddComponent<SimpleBoneMeshRender>();
		}
	}

	static void SetBaseColor(GameObject& obj, Color& baseColor, SURFACE_TYPE surface)
	{
		switch (surface)
		{
		case SURFACE_TYPE::BlingPhong:
			static_cast<BlingPhongMeshObject&>(obj).Material.MaterialDiffuse = baseColor;
			return;
		case SURFACE_TYPE::PBR:
			static_cast<PBRMeshObject&>(obj).Material.Albedo = baseColor;
			return;
		default:
			return;
		}
	}

	static void LoadTexture(aiMaterial* ai_material, const wchar_t* directory, MeshRender* meshRender, SURFACE_TYPE surface)
	{
		auto compressPopup = [=](const wchar_t* path, E_TEXTURE::TYPE type)
			{
				//return false;  //테스트
				return ImGui::ShowCompressPopup(path, &meshRender->textures, type);
			};

		aiString path;
		std::wstring basePath;
		{
			if (AI_SUCCESS == ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &path) ||
				AI_SUCCESS == ai_material->GetTexture(aiTextureType_BASE_COLOR, 0, &path))
			{
				if (Utility::ParseFileName(path))
				{
					basePath = directory;
					basePath += L"\\";
					basePath += utfConvert::utf8_to_wstring(path.C_Str());
					if (!compressPopup(basePath.c_str(), E_TEXTURE::Albedo))
						meshRender->textures.SetTexture2D(E_TEXTURE::Albedo, basePath.c_str());
				}
			}
			else if (aiColor4D baseColor; ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS)
			{
				SimpleMeshRender* simpleMeshRender = reinterpret_cast<SimpleMeshRender*>(meshRender);

				simpleMeshRender->baseColor.x = baseColor.r;
				simpleMeshRender->baseColor.y = baseColor.g;
				simpleMeshRender->baseColor.z = baseColor.b;
				simpleMeshRender->baseColor.w = baseColor.a;

				SetBaseColor(simpleMeshRender->gameObject, simpleMeshRender->baseColor, surface);
			}
			if (AI_SUCCESS == ai_material->GetTexture(aiTextureType_NORMALS, 0, &path))
			{
				if (Utility::ParseFileName(path))
				{
					basePath = directory;
					basePath += L"\\";
					basePath += utfConvert::utf8_to_wstring(path.C_Str());
					if (!compressPopup(basePath.c_str(), E_TEXTURE::Normal))
						meshRender->textures.SetTexture2D(E_TEXTURE::Normal, basePath.c_str());
				}
			}
			if (AI_SUCCESS == ai_material->GetTexture(aiTextureType_SPECULAR, 0, &path))
			{
				if (Utility::ParseFileName(path))
				{
					basePath = directory;
					basePath += L"\\";
					basePath += utfConvert::utf8_to_wstring(path.C_Str());
					if (!compressPopup(basePath.c_str(), E_TEXTURE::Specular))
						meshRender->textures.SetTexture2D(E_TEXTURE::Specular, basePath.c_str());
				}
			}
			if (AI_SUCCESS == ai_material->GetTexture(aiTextureType_EMISSIVE, 0, &path))
			{
				if (Utility::ParseFileName(path))
				{
					basePath = directory;
					basePath += L"\\";
					basePath += utfConvert::utf8_to_wstring(path.C_Str());
					if (!compressPopup(basePath.c_str(), E_TEXTURE::Emissive))
						meshRender->textures.SetTexture2D(E_TEXTURE::Emissive, basePath.c_str());
				}
			}
			if (AI_SUCCESS == ai_material->GetTexture(aiTextureType_OPACITY, 0, &path))
			{
				if (Utility::ParseFileName(path))
				{
					basePath = directory;
					basePath += L"\\";
					basePath += utfConvert::utf8_to_wstring(path.C_Str());
					if (!compressPopup(basePath.c_str(), E_TEXTURE::Opacity))
						meshRender->textures.SetTexture2D(E_TEXTURE::Opacity, basePath.c_str());

					meshRender->RenderFlags |= RENDER_ALPHA;
					if (meshRender->IsPSShader())
					{
						std::wstring path = HLSLManager::EngineShaderPath;
						switch (surface)
						{
						case SURFACE_TYPE::BlingPhong:
							path += L"BlingPhongOpacitiyPS.hlsl";
							meshRender->SetPixelShader(path.c_str());
							break;
						case SURFACE_TYPE::PBR:
							path += L"PBROpacitiyPS.hlsl";
							meshRender->SetPixelShader(path.c_str());
							break;
						default:
							break;
						}
					}
				}
			}

			//블랜더 기준 메탈릭 맵
			if (AI_SUCCESS == ai_material->GetTexture(aiTextureType_METALNESS, 0, &path))
			{
				if (Utility::ParseFileName(path))
				{
					basePath = directory;
					basePath += L"\\";
					basePath += utfConvert::utf8_to_wstring(path.C_Str());
					if (!compressPopup(basePath.c_str(), E_TEXTURE::Metalness))
						meshRender->textures.SetTexture2D(E_TEXTURE::Metalness, basePath.c_str());

					if (surface == SURFACE_TYPE::PBR)
					{
						static_cast<PBRMeshObject&>(meshRender->gameObject).Material.UseMetalnessMap = true;
					}
				}
			}
			else if (float scala; ai_material->Get(AI_MATKEY_METALLIC_FACTOR, scala) == AI_SUCCESS)
			{
				if (surface == SURFACE_TYPE::PBR)
				{
					static_cast<PBRMeshObject&>(meshRender->gameObject).Material.Roughness = scala;
				}
			}

			//블랜더 기준 러프니스 맵
			if (AI_SUCCESS == ai_material->GetTexture(aiTextureType_SHININESS, 0, &path))
			{
				if (Utility::ParseFileName(path))
				{
					basePath = directory;
					basePath += L"\\";
					basePath += utfConvert::utf8_to_wstring(path.C_Str());
					if (!compressPopup(basePath.c_str(), E_TEXTURE::Roughness))
						meshRender->textures.SetTexture2D(E_TEXTURE::Roughness, basePath.c_str());

					if (surface == SURFACE_TYPE::PBR)
					{
						static_cast<PBRMeshObject&>(meshRender->gameObject).Material.UseRoughnessMap = true;
					}
				}
			}
			else if (float scala; ai_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, scala) == AI_SUCCESS)
			{
				if (surface == SURFACE_TYPE::PBR)
				{
					static_cast<PBRMeshObject&>(meshRender->gameObject).Material.Roughness = scala;
				}
			}
		}
	}

	static void SetNodeTransform(const aiNode* node, GameObject* objcet)
	{
		Vector3 pos;
		Quaternion rot;
		Vector3 scale;
		AssimpMath::DecomposeTransform(node, pos, rot, scale);
		if (objcet->transform.Parent)
		{
			objcet->transform.localPosition = pos;
			objcet->transform.localRotation = rot;
			objcet->transform.localScale = scale;
		}
	}

	static TransformAnimation* SetTransformAnimation(const aiScene* pScene, GameObject& _gameObject, std::unordered_map<std::wstring, GameObject*>& addObjMap)
	{
		using Clip = TransformAnimation::Clip;
		using PositionKey = TransformAnimation::Clip::NodeAnimation::PositionKey;
		using RotationKey = TransformAnimation::Clip::NodeAnimation::RotationKey;
		using ScaleKey = TransformAnimation::Clip::NodeAnimation::ScaleKey;
		TransformAnimation& anime = _gameObject.AddComponent<TransformAnimation>();
		for (unsigned int clipNum = 0; clipNum < pScene->mNumAnimations; clipNum++)
		{
			aiAnimation* currAnimation = pScene->mAnimations[clipNum];
			Clip clip;
			clip.Duration = (float)currAnimation->mDuration;
			clip.TickTime = (float)currAnimation->mTicksPerSecond;
			std::wstring clipName = utfConvert::utf8_to_wstring(pScene->mAnimations[clipNum]->mName.C_Str());
			for (unsigned int nodeIndex = 0; nodeIndex < currAnimation->mNumChannels; nodeIndex++)
			{
				aiNodeAnim* currNodeAnim = currAnimation->mChannels[nodeIndex];
				using NodeAnime = Clip::NodeAnimation;
				NodeAnime nodeAnime;
				std::wstring currNodeName = utfConvert::utf8_to_wstring(currNodeAnim->mNodeName.C_Str());
				nodeAnime.targetName = currNodeName;

				//make keyList
				if (currNodeAnim->mNumPositionKeys > 0 && !nodeAnime.positionKeys)
				{
					nodeAnime.positionKeys = anime.get_position_key(clipName.c_str(), nodeIndex);
				}
				if (currNodeAnim->mNumRotationKeys > 0 && !nodeAnime.rotationKeys)
				{
					nodeAnime.rotationKeys = anime.get_rotation_key(clipName.c_str(), nodeIndex);
				}
				if (currNodeAnim->mNumScalingKeys > 0 && !nodeAnime.scaleKeys)
				{
					nodeAnime.scaleKeys = anime.get_scale_key(clipName.c_str(), nodeIndex);
				}

				for (unsigned int k = 0; k < currNodeAnim->mNumPositionKeys; k++)
				{
					NodeAnime::PositionKey key;
					key.position.x = currNodeAnim->mPositionKeys[k].mValue.x;
					key.position.y = currNodeAnim->mPositionKeys[k].mValue.y;
					key.position.z = currNodeAnim->mPositionKeys[k].mValue.z;
					key.Time = (float)currNodeAnim->mPositionKeys[k].mTime;
					nodeAnime.positionKeys->push_back(key);
				}
				for (unsigned int k = 0; k < currNodeAnim->mNumRotationKeys; k++)
				{
					NodeAnime::RotationKey key;
					key.rotation.x = currNodeAnim->mRotationKeys[k].mValue.x;
					key.rotation.y = currNodeAnim->mRotationKeys[k].mValue.y;
					key.rotation.z = currNodeAnim->mRotationKeys[k].mValue.z;
					key.rotation.w = currNodeAnim->mRotationKeys[k].mValue.w;

					key.Time = (float)currNodeAnim->mRotationKeys[k].mTime;
					nodeAnime.rotationKeys->push_back(key);
				}
				for (unsigned int k = 0; k < currNodeAnim->mNumScalingKeys; k++)
				{
					NodeAnime::ScaleKey key;
					key.scale.x = currNodeAnim->mScalingKeys[k].mValue.x;
					key.scale.y = currNodeAnim->mScalingKeys[k].mValue.y;
					key.scale.z = currNodeAnim->mScalingKeys[k].mValue.z;
					key.Time = (float)currNodeAnim->mScalingKeys[k].mTime;
					nodeAnime.scaleKeys->push_back(key);
				}
				clip.nodeAnimations.push_back(nodeAnime);
			}
			anime.AddClip(clipName.c_str(), clip);
		}
		return &anime;
	}

	static bool IsBone(GameObject* obj)
	{
		std::queue<GameObject*> objQue;
		objQue.push(obj);
		GameObject* currObj = nullptr;
		while (!objQue.empty())
		{
			currObj = objQue.front();
			objQue.pop();
			if (typeid(BoneObject) == typeid(*currObj))
			{
				return true;
			}
			for (unsigned int i = 0; i < currObj->transform.GetChildCount(); i++)
			{
				objQue.push(&currObj->transform.GetChild(i)->gameObject);
			}
		}
		return false;
	}	
}

bool Utility::ParseFileName(aiString& str)
{
	const char* findBackSlash = strrchr(str.C_Str(), '\\');
	const char* findSlash = strrchr(str.C_Str(), '/');
	const char* findDot = strrchr(str.C_Str(), '.');
	if (findBackSlash)
	{
		str.Set(findBackSlash + 1);
		return true;
	}
	else if (findSlash)
	{
		str.Set(findSlash + 1);
		return true;
	}
	else if (findDot)
	{
		return true;
	}
	else
		return false;
}

GameObject* Utility::LoadFBX(const wchar_t* path,
	std::function<void(MeshRender*)> initMesh,
	bool isStatic,
	SURFACE_TYPE surface)
{
	using namespace utfConvert;

	Assimp::Importer importer;
	unsigned int importFlags =
		aiProcess_Triangulate |         // vertex 삼각형 으로 출력
		aiProcess_GenNormals |          // Normal 정보 생성  
		aiProcess_GenUVCoords |         // 텍스처 좌표 생성
		aiProcess_CalcTangentSpace |    // 탄젠트 벡터 생성
		aiProcess_PopulateArmatureData |// 본 데이터 생성
		aiProcess_LimitBoneWeights |    // 본 영향 정점 개수 제한
		aiProcess_ConvertToLeftHanded;  // DX용 왼손좌표계 변환

	if (isStatic)
		importFlags |= aiProcess_PreTransformVertices;   // 노드의 변환행렬을 적용한 버텍스 생성한다.  *사용하면 모든 메쉬가 합쳐져서 애니메이션 사용이 불가능하다.

	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);    // $assimp_fbx$ 노드 생성안함

	std::wstring wstr_path = std::filesystem::path(path).filename().c_str();
	std::string str_path = wstring_to_utf8(path);
	std::filesystem::path resourcePath(GetTempResourcePath(wstr_path.c_str()));

	//첫 로드시 반드시 리소스 생성
	static std::unordered_set<std::wstring> resourceSetFlag;
	auto find = resourceSetFlag.find(wstr_path);
	if (find != resourceSetFlag.end())
	{
		if (std::filesystem::exists(resourcePath)) //리소스 존재 확인
		{
			GameObject* obj = gameObjectFactory.DeserializedObject(resourcePath.c_str());
			return obj;
		}
	}
	else
	{
		if (!std::filesystem::exists(resourcePath.parent_path()))
		{
			std::filesystem::create_directories(resourcePath.parent_path());
		}
	}
	resourceSetFlag.insert(wstr_path);

	const aiScene* pScene = importer.ReadFile(str_path, importFlags);
	if (pScene == nullptr)
	{
		__debugbreak(); //파일이 존재하지 않음.
		return nullptr;
	}
		
	std::wstring directory = utfConvert::utf8_to_wstring(str_path.substr(0, str_path.find_last_of("/\\")));

	std::queue<aiNode*> nodeQue;
	nodeQue.push(pScene->mRootNode);
	const aiNode* currNode = nullptr;

	std::queue<GameObject*> objQue;
	GameObject* rootObject = NewGameObject(wstr_path.c_str());
	objQue.push(rootObject);
	GameObject* currObj = nullptr;

	std::unordered_map<std::wstring, GameObject*> addObjMap;
	std::unordered_map<std::wstring, int> boneIndexMap(128);
	int boneIndexManager = 0;
	auto getBoneIndex = [&boneIndexMap, &boneIndexManager](const std::wstring& name)->int
		{
			auto findIter = boneIndexMap.find(name);
			if (findIter != boneIndexMap.end())
				return findIter->second;
			else
			{
				boneIndexMap[name] = boneIndexManager;
				return boneIndexManager++;
			}
		};

	//count bone
	int boneCount = 0;
	for (unsigned int i = 0; i < pScene->mNumMeshes; ++i)
	{
		aiMesh* mesh = pScene->mMeshes[i];
		for (unsigned int j = 0; j < mesh->mNumBones; ++j)
		{
			getBoneIndex(utf8_to_wstring(mesh->mBones[j]->mName.C_Str()));
		}
	}
	boneCount = boneIndexMap.size();

	std::vector<SimpleBoneMeshRender*> meshList;
	TransformAnimation* transformAnimation = nullptr;
	//set animation
	if (pScene->mAnimations)
	{
		transformAnimation = SetTransformAnimation(pScene, *rootObject, addObjMap);
	}

	while (!nodeQue.empty())
	{
		currNode = nodeQue.front();
		nodeQue.pop();

		currObj = objQue.front();
		objQue.pop();

		addObjMap[currObj->Name] = currObj;

		if (currNode)
		{			
			std::wstring currNodeName = utf8_to_wstring(currNode->mName.C_Str());
			if (boneCount > 0)
			{			
				SetNodeTransform(currNode, currObj);
				
				if (currNode->mNumMeshes > 0) 
				{			
					for (unsigned int i = 0; i < currNode->mNumMeshes; i++)
					{
						SimpleBoneMeshRender& meshComponent = AddBoneMeshComponent(currObj, surface);
						meshList.push_back(&meshComponent);

						unsigned int meshIndex = currNode->mMeshes[i];
						std::wstring OffsetMatricesKey = currNodeName + std::to_wstring(meshIndex);

						aiMesh* pMesh = pScene->mMeshes[meshIndex];

						//Load Texture
						aiMaterial* ai_material = pScene->mMaterials[pMesh->mMaterialIndex];
						LoadTexture(ai_material, directory.c_str(), &meshComponent, surface);

						//offsetMatrices
						meshComponent.offsetMatrices = GetResourceManager<OffsetMatrices>().GetResource(OffsetMatricesKey.c_str());
						meshComponent.offsetMatrices->data.resize(boneCount);

						for (unsigned int i = 0; i < pMesh->mNumBones; i++)
						{
							aiMatrix4x4 ai_matrix = pMesh->mBones[i]->mOffsetMatrix;
							std::wstring name = utf8_to_wstring(pMesh->mBones[i]->mName.C_Str());
							meshComponent.offsetMatrices->data[getBoneIndex(name)] = Matrix(&ai_matrix.a1).Transpose();
						}

						//cash vertex bone info
						std::unordered_map<int, std::vector<float>>		 weightsMap;
						std::unordered_map<int, std::list<std::wstring>> nameMap;
						if (pMesh->mNumVertices > 0)
						{
							weightsMap.reserve(pMesh->mNumVertices);
							nameMap.reserve(pMesh->mNumVertices);
						}
						for (unsigned int j = 0; j < pMesh->mNumBones; j++)
						{
							aiBone* pBone = pMesh->mBones[j];
							for (unsigned int k = 0; k < pBone->mNumWeights; k++)
							{
								aiVertexWeight weight = pBone->mWeights[k];
								weightsMap[weight.mVertexId].push_back(weight.mWeight);
								nameMap[weight.mVertexId].emplace_back(utfConvert::utf8_to_wstring(pBone->mName.C_Str()));				
							}
						}

						for (unsigned int vertexIndex = 0; vertexIndex < pMesh->mNumVertices; vertexIndex++)
						{
							SimpleBoneMeshRender::Vertex vertex;

							aiVector3D position = pMesh->mVertices[vertexIndex];
							vertex.position.x = position.x;
							vertex.position.y = position.y;
							vertex.position.z = position.z;
							vertex.position.w = 1.0f;

							aiVector3D normal = pMesh->mNormals[vertexIndex];
							vertex.normal.x = normal.x;
							vertex.normal.y = normal.y;
							vertex.normal.z = normal.z;

							aiVector3D Tangents = pMesh->mTangents[vertexIndex];
							vertex.Tangent.x = Tangents.x;
							vertex.Tangent.y = Tangents.y;
							vertex.Tangent.z = Tangents.z;

							aiVector3D texCoord = pMesh->mTextureCoords[0][vertexIndex];
							vertex.Tex.x = texCoord.x;
							vertex.Tex.y = texCoord.y;

							auto n = nameMap[vertexIndex].begin();
							for (int j = 0; j < weightsMap[vertexIndex].size(); j++)
							{
								if(vertex.BlendWeights[j] != 0 )
									__debugbreak();

								vertex.BlendWeights[j] = weightsMap[vertexIndex][j];
								vertex.BlendIndecses[j] = getBoneIndex(*n);
								n++;
							}

							meshComponent.vertices.push_back(vertex);	 							
						}

						//Create Index
						for (unsigned int faceIndex = 0; faceIndex < pMesh->mNumFaces; faceIndex++)
						{
							const aiFace& face = pMesh->mFaces[faceIndex];
							for (unsigned int numIndex = 0; numIndex < face.mNumIndices; numIndex++)
							{
								meshComponent.indices.push_back(face.mIndices[numIndex]);
							}
						}
			
						meshComponent.MeshID = meshIndex;
						meshComponent.SetMeshResource(currObj->Name.c_str());
						meshComponent.CreateMesh();

						initMesh(&meshComponent);
					}
				}
			}
			else
			{
				SetNodeTransform(currNode, currObj);

				if (currNode->mNumMeshes > 0)
				{
					//Create Vertex
					for (unsigned int i = 0; i < currNode->mNumMeshes; i++)
					{
						SimpleMeshRender& meshComponent = AddMeshComponent(currObj, surface);

						unsigned int meshIndex = currNode->mMeshes[i];
						aiMesh* pMesh = pScene->mMeshes[meshIndex];

						//Load Texture
						aiMaterial* ai_material = pScene->mMaterials[pMesh->mMaterialIndex];
						LoadTexture(ai_material, directory.c_str(), &meshComponent, surface);
			
						for (unsigned int vertexIndex = 0; vertexIndex < pMesh->mNumVertices; vertexIndex++)
						{
							SimpleMeshRender::Vertex vertex;
							aiVector3D position = pMesh->mVertices[vertexIndex];
							vertex.position.x = position.x;
							vertex.position.y = position.y;
							vertex.position.z = position.z;
							vertex.position.w = 1.0f;

							aiVector3D normal = pMesh->mNormals[vertexIndex];
							vertex.normal.x = normal.x;
							vertex.normal.y = normal.y;
							vertex.normal.z = normal.z;

							aiVector3D Tangents = pMesh->mTangents[vertexIndex];
							vertex.Tangent.x = Tangents.x;
							vertex.Tangent.y = Tangents.y;
							vertex.Tangent.z = Tangents.z;

							aiVector3D texCoord = pMesh->mTextureCoords[0][vertexIndex];
							vertex.Tex.x = texCoord.x;
							vertex.Tex.y = texCoord.y;

							meshComponent.vertices.push_back(vertex);
						}

						//Create Index
						for (unsigned int faceIndex = 0; faceIndex < pMesh->mNumFaces; faceIndex++)
						{
							const aiFace& face = pMesh->mFaces[faceIndex];
							for (unsigned int numIndex = 0; numIndex < face.mNumIndices; numIndex++)
							{
								meshComponent.indices.push_back(face.mIndices[numIndex]);
							}
						}

						meshComponent.MeshID = meshIndex;
						meshComponent.SetMeshResource(currObj->Name.c_str());
						meshComponent.CreateMesh();

						initMesh(&meshComponent);
					}
				}
			}
			
			//create childs
			for (unsigned int i = 0; i < currNode->mNumChildren; i++)
			{
				nodeQue.push(currNode->mChildren[i]);
				std::wstring childName = utf8_to_wstring(currNode->mChildren[i]->mName.C_Str());
				GameObject* childObj = nullptr;
				auto findIndex = boneIndexMap.find(childName);
				if (findIndex != boneIndexMap.end())
				{
					BoneObject* chidBone = NewGameObject<BoneObject>(childName.c_str());
					int index = findIndex->second;
					chidBone->myIndex = index;
					childObj = chidBone;
				}
				else
				{
					childObj = NewMeshObject(surface, childName.c_str());
				}			
				childObj->transform.SetParent(currObj->transform, false);
				objQue.push(childObj);
			}
		}
	}

	//set bone info
	for (auto& mesh : meshList)
	{
		mesh->AddBonesFromRoot();
	}

	//set Target
	if (transformAnimation)
	{
		transformAnimation->AddChildrenToTargets();
	}

	//리소스 등록
	gameObjectFactory.SerializedObject(rootObject, resourcePath.c_str(), true); //리소스 직렬화.

	return rootObject;
}

GameObject* Utility::LoadFBX(const wchar_t* path, bool isStatic, SURFACE_TYPE surface)
{
	return LoadFBX(path, [](MeshRender* mesh)->void { return; }, isStatic, surface);
}

void Utility::LoadFBXResource(const wchar_t* path,
	std::function<void(MeshRender*)> initMesh,
	bool isStatic,
	SURFACE_TYPE surface)
{
	ImGui::DestroyObjTextureCompressEnd(LoadFBX(path, initMesh, isStatic, surface));
}

void Utility::LoadFBXResource(const wchar_t* path, bool isStatic, SURFACE_TYPE surface)
{
	LoadFBXResource(path, [](MeshRender* mesh)->void { return; }, isStatic, surface);
}

std::filesystem::path Utility::GetTempResourcePath(const wchar_t* flieName)
{
	//임시 리소스 경로
	wchar_t tempPath[MAX_PATH];
	GetTempPath(MAX_PATH, tempPath);
	std::filesystem::path resourcePath = tempPath;

	//파일 이름 설정
	GetModuleFileName(nullptr, tempPath, MAX_PATH);
	resourcePath /= L"power12562";
	resourcePath /= WinGameApp::GetAppName();
	resourcePath /= std::filesystem::path(flieName).filename();
	resourcePath.replace_extension(L".GameObject");

	return resourcePath;
}
