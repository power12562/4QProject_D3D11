#pragma once
#include <GameObject/Base/GameObject.h>

class BoneObject : public GameObject
{
	SERIALIZED_OBJECT(BoneObject)
public:
	BoneObject() = default;
	/*이름, 인스턴스 아이디 부여 후 호출되는 함수.*/
	virtual ~BoneObject() = default;

	virtual void Serialized(std::ofstream& ofs);
	virtual void Deserialized(std::ifstream& ifs);

public:
	/*Get world matrix랑 동일*/
	const Matrix& GetBoneMatrix();
	int myIndex = -1;
};