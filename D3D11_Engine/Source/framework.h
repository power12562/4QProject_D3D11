#pragma once
//frameworks headers

//Core
#include <Core/TimeSystem.h>

//D3DCore
#include <D3DCore/D3D11_GameApp.h>

//Managers
#include <Manager/HLSLManager.h>
#include <Manager/ResourceManager.h>
#include <Manager/SceneManager.h>
#include <Manager/TextureManager.h>
#include <Manager/GameObjectFactory.h>
#include <Manager/InputManager.h>

//PhysX
#include <Physics/PhysXFramework.h>
#include <Physics/enum/ForceMode.h>
#include <Physics/enum/CollisionDetectionMode.h>
#include <Physics/Struct/CollisionInfo.h>
#include <Physics/Struct/RaycastResult.h>
#include <Physics/PhysicsManager.h>
#include <Physics/PhysicsLayerManager/PhysicsLayerManager.h>
#include <Physics/PhysicsMaterial/PhysicsMaterial.h>
#include <Physics/PhysicsActor/PhysicsActor.h>
#include <Physics/PhysicsScene/PhysicsScene.h>

// Sound
#include <Sound/FMODFramework.h>
#include <Sound/SoundSystem.h>


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

#include <Component/Rigidbody/Rigidbody.h>
#include <Component/Collider/BoxCollider.h>
#include <Component/Collider/CapsuleCollider.h>
#include <Component/Collider/SphereCollider.h>

#include <Component/SoundManager/SoundManager.h>


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
#include <Utility/WinUtility.h>

//Math
#include <Math/Mathf.h>