#include <Manager/ResourceManager.h>
#include <GameObject/Base/GameObject.h>

void Resource::ClearResourceManagers()
{
	for (auto& clear : clearList)
	{
		clear();
	}
}

