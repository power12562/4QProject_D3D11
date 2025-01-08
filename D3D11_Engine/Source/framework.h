#pragma once
//frameworks headers

//Core
#include <Core/DXTKInputSystem.h>
#include <Core/TimeSystem.h>

//D3DCore
#include <D3DCore/D3D11_GameApp.h>

//Managers
#include <Manager/HLSLManager.h>
#include <Manager/ResourceManager.h>
#include <Manager/SceneManager.h>
#include <Manager/TextureManager.h>
#include <Manager/GameObjectFactory.h>

//gameObjects
#include <GameObject/Base/CameraObject.h>
#include <GameObject/Bone/BoneObject.h>
#include <GameObject/Mesh/BlingPhongMeshObject.h>
#include <GameObject/Mesh/PBRMeshObject.h>
#include <GameObject/Mesh/SphereObject.h>
#include <GameObject/Mesh/SkyBoxObject.h>
#include <GameObject/Mesh/CubeObject.h>

//Components
#include <Component/Camera/Camera.h>
#include <Component/Camera/CameraMoveHelper.h>
#include <Component/Animation/TransformAnimation.h>

#include <Component/Render/BlingPhongBoneMeshRender.h>
#include <Component/Render/BlingPhongMeshRender.h>
#include <Component/Render/PBRBoneMeshRender.h>
#include <Component/Render/PBRMeshRender.h>
#include <Component/Render/SkyBoxRender.h>
#include <Component/Render/CubeMeshRender.h>

//Lights
#include <Light/PBRDirectionalLight.h>
#include <Light/SimpleDirectionalLight.h>

//Utility
#include <Utility/AssimpUtility.h>
#include <Utility/Console.h>
#include <Utility/D3D11Utility.h>
#include <Utility/SpinLock.h>
#include <Utility/utfConvert.h>
#include <Utility/ImguiHelper.h>
#include <Utility/FileSystemUtility.h>

//Math
#include <Math/Mathf.h>