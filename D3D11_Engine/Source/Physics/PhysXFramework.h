#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <physx/PxPhysics.h>
#include <physx/PxPhysicsAPI.h>

#pragma comment(lib, "PhysX_64.lib")
#pragma comment(lib, "PhysXCommon_64.lib")
#pragma comment(lib, "PhysXCooking_64.lib")
#pragma comment(lib, "PhysXExtensions_static_64.lib")
#pragma comment(lib, "PhysXFoundation_64.lib")
#pragma comment(lib, "PhysXPvdSDK_static_64.lib")
//#pragma comment(lib, "PhysXTask_static_64.lib")
//#pragma comment(lib, "PhysXVehicle_static_64.lib")
//#pragma comment(lib, "PhysXVehicle2_static_64.lib")
//#pragma comment(lib, "PVDRuntime_64.lib")
//#pragma comment(lib, "SceneQuery_static_64.lib")
//#pragma comment(lib, "SimulationController_static_64.lib")

namespace px = physx;


template<class Interface>
inline void SafeRelease(Interface** ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != nullptr)
	{
		(*ppInterfaceToRelease)->Release();
		(*ppInterfaceToRelease) = nullptr;
	}
}

template <typename T>
void SafeDelete(T** target)
{
	if (*target != nullptr)
	{
		delete (*target);
		(*target) = nullptr;
	}
}