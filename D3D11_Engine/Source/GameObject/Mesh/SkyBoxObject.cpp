#include "SkyBoxObject.h"
#include <Component/Render/SkyBoxRender.h>

SkyBoxObject::SkyBoxObject() : 
	skyBoxRender(AddComponent<SkyBoxRender>())
{

}
