#include "BoneObject.h"
#include <Utility/SerializedUtility.h>
#include <framework.h>

void BoneObject::Serialized(std::ofstream& ofs)
{
	using namespace Binary;
	Write::data(ofs, myIndex);
}

void BoneObject::Deserialized(std::ifstream& ifs)
{
	using namespace Binary;
	myIndex = Read::data<int>(ifs);
}

const Matrix& BoneObject::GetBoneMatrix()
{
	return transform.GetWM();
}
