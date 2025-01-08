#include "TransformAnimation.h"
#include <Core/TimeSystem.h>
#include <Utility\AssimpUtility.h>
#include <D3DCore/D3DRenderer.h>
#include <stack>
#include <format>
#include <Manager/ResourceManager.h>
#include <Utility/SerializedUtility.h>

std::shared_ptr<std::vector<TransformAnimation::Clip::NodeAnimation::PositionKey>>
TransformAnimation::get_position_key(const wchar_t* clipName, unsigned int nodeIndex)
{
	return GetResourceManager<std::vector<TransformAnimation::Clip::NodeAnimation::PositionKey>>()
		.GetResource(std::format(L"{}_{}_position_{}", gameObject.Name, clipName, nodeIndex).c_str());
}

std::shared_ptr<std::vector<TransformAnimation::Clip::NodeAnimation::RotationKey>>
TransformAnimation::get_rotation_key(const wchar_t* clipName, unsigned int nodeIndex)
{
	return GetResourceManager<std::vector<TransformAnimation::Clip::NodeAnimation::RotationKey>>()
		.GetResource(std::format(L"{}_{}_rotation_{}", gameObject.Name, clipName, nodeIndex).c_str());
}

std::shared_ptr<std::vector<TransformAnimation::Clip::NodeAnimation::ScaleKey>>
TransformAnimation::get_scale_key(const wchar_t* clipName, unsigned int nodeIndex)
{
	return GetResourceManager<std::vector<TransformAnimation::Clip::NodeAnimation::ScaleKey>>()
		.GetResource(std::format(L"{}_{}_scale_{}", gameObject.Name, clipName, nodeIndex).c_str());
}

void TransformAnimation::CopyClips(TransformAnimation* dest, TransformAnimation* source)
{
	dest->clips = source->clips;
}

TransformAnimation::TransformAnimation()
{
}

TransformAnimation::~TransformAnimation()
{
}

void TransformAnimation::SerializedAnimation(std::ofstream& ofs)
{
	using Clip = TransformAnimation::Clip;
	using NodeAnimation = TransformAnimation::Clip::NodeAnimation;
	using PositionKey = TransformAnimation::Clip::NodeAnimation::PositionKey;
	using RotationKey = TransformAnimation::Clip::NodeAnimation::RotationKey;
	using ScaleKey = TransformAnimation::Clip::NodeAnimation::ScaleKey;

	using namespace Binary;
	Write::data(ofs, clips.size());
	for (auto& [clipName, clip] : clips)
	{
		Write::wstring(ofs, clipName);
		Write::data(ofs, clip.Duration);
		Write::data(ofs, clip.TickTime);

		Write::data(ofs, clip.nodeAnimations.size());
		for (auto& nodeAnimation : clip.nodeAnimations)
		{
			Write::wstring(ofs, nodeAnimation.targetName);
			Write::data(ofs, nodeAnimation.positionKeys->size());
			if (nodeAnimation.positionKeys->size() > 0)
			{
				Write::data(ofs, nodeAnimation.positionKeys->data(), sizeof(PositionKey) * nodeAnimation.positionKeys->size());
			}
			Write::data(ofs, nodeAnimation.rotationKeys->size());
			if (nodeAnimation.rotationKeys->size() > 0)
			{
				Write::data(ofs, nodeAnimation.rotationKeys->data(), sizeof(RotationKey) * nodeAnimation.rotationKeys->size());
			}
			Write::data(ofs, nodeAnimation.scaleKeys->size());
			if (nodeAnimation.scaleKeys->size() > 0)
			{
				Write::data(ofs, nodeAnimation.scaleKeys->data(), sizeof(ScaleKey) * nodeAnimation.scaleKeys->size());
			}
		}
	}
}

void TransformAnimation::DeserializedAnimation(std::ifstream& ifs)
{
	using Clip = TransformAnimation::Clip;
	using NodeAnimation = TransformAnimation::Clip::NodeAnimation;
	using PositionKey = TransformAnimation::Clip::NodeAnimation::PositionKey;
	using RotationKey = TransformAnimation::Clip::NodeAnimation::RotationKey;
	using ScaleKey = TransformAnimation::Clip::NodeAnimation::ScaleKey;

	using namespace Binary;
	size_t clipCount = Read::data<size_t>(ifs);
	for (size_t clipIndex = 0; clipIndex < clipCount; clipIndex++)
	{
		Clip clip{};
		std::wstring clipName = Read::wstring(ifs);
		clip.Duration = Read::data<float>(ifs);
		clip.TickTime = Read::data<float>(ifs);

		size_t nodeAnimationCount = Read::data<size_t>(ifs);
		for (size_t nodeIndex = 0; nodeIndex < nodeAnimationCount; nodeIndex++)
		{
			NodeAnimation nodeAnimation{};
			nodeAnimation.targetName = Read::wstring(ifs);
			nodeAnimation.positionKeys = get_position_key(clipName.c_str(), nodeIndex);
			size_t positionKeyCount = Read::data<size_t>(ifs);
			if (positionKeyCount > 0 && nodeAnimation.positionKeys.use_count() == 1)
			{
				nodeAnimation.positionKeys->resize(positionKeyCount);
				Read::data(ifs, nodeAnimation.positionKeys->data(), sizeof(PositionKey) * positionKeyCount);
			}
			else
			{
				ifs.seekg(sizeof(PositionKey) * positionKeyCount, std::ios::cur);
			}
			nodeAnimation.rotationKeys = get_rotation_key(clipName.c_str(), nodeIndex);
			size_t rotationKeyCount = Read::data<size_t>(ifs);
			if (rotationKeyCount > 0 && nodeAnimation.rotationKeys.use_count() == 1)
			{
				nodeAnimation.rotationKeys->resize(rotationKeyCount);
				Read::data(ifs, nodeAnimation.rotationKeys->data(), sizeof(RotationKey) * rotationKeyCount);
			}
			else
			{
				ifs.seekg(sizeof(RotationKey) * rotationKeyCount, std::ios::cur);
			}
			nodeAnimation.scaleKeys = get_scale_key(clipName.c_str(), nodeIndex);
			size_t scaleKeyCount = Read::data<size_t>(ifs);
			if (scaleKeyCount > 0 && nodeAnimation.scaleKeys.use_count() == 1)
			{
				nodeAnimation.scaleKeys->resize(scaleKeyCount);
				Read::data(ifs, nodeAnimation.scaleKeys->data(), sizeof(ScaleKey) * scaleKeyCount);
			}	
			else
			{
				ifs.seekg(sizeof(ScaleKey) * scaleKeyCount, std::ios::cur);
			}
			clip.nodeAnimations.push_back(nodeAnimation);
		}
		AddClip(clipName.c_str(), clip);
	}
}

bool TransformAnimation::PlayClip(const wchar_t* clipName, bool _isLoop)
{
	auto findIter = clips.find(clipName);
	if (findIter != clips.end())
	{
		currClip = &findIter->second;
		isPause = false;
		isLoop = _isLoop;
		
		//대상을 찾는다
		for (auto& animation : currClip->nodeAnimations)
		{
			auto findTarget = targets.find(animation.targetName);
			if (findTarget == targets.end())
			{
				Debug_printf("warning: TransformAnimation::PlayClip target not found\n");
				currClip = nullptr;
				return false;
			}
			auto& [name, target] = *findTarget;
			animation.objTarget = target;
		}

		//첫프레임은 Matrix 바로 갱신
		elapsedTime = currClip->TickTime * TimeSystem::Time.DeltaTime;
		currClip->ResetCashIndex();
		for (auto& nodeAni : currClip->nodeAnimations)
		{
			nodeAni.Evaluate(elapsedTime);
		}
		transform.PushUpdateList();
		return true;
	}
	else
	{
		__debugbreak();
		Debug_printf("존재하지 않는 클립.\n");
		return false;
	}
}

void TransformAnimation::StopClip()
{
	isPause = true;
	elapsedTime = currClip->TickTime * TimeSystem::Time.DeltaTime;
	currClip->ResetCashIndex();
	for (auto& nodeAni : currClip->nodeAnimations)
	{
		nodeAni.Evaluate(elapsedTime);
	}
}

void TransformAnimation::AddClip(const wchar_t* name, Clip& clip)
{
	clips[name] = clip;
}

void TransformAnimation::CopyClips(TransformAnimation* source)
{
	this->clips = source->clips;
}

void TransformAnimation::AddChildrenToTargets()
{
	targets.clear();
	std::stack<Transform*> transformStack;
	for (unsigned int i = 0; i < gameObject.transform.GetChildCount(); i++)
	{
		transformStack.push(gameObject.transform.GetChild(i));
	}
	while (!transformStack.empty())
	{
		Transform* currTransform = transformStack.top();
		transformStack.pop();
		targets[currTransform->gameObject.Name] = &currTransform->gameObject;
		for (unsigned int i = 0; i < currTransform->GetChildCount(); i++)
		{
			transformStack.push(currTransform->GetChild(i));
		}
	}
}

void TransformAnimation::Start()
{
}

void TransformAnimation::FixedUpdate()
{
}

void TransformAnimation::Update()
{

}

void TransformAnimation::LateUpdate()
{
	if (currClip && !isPause)
	{
		elapsedTime += currClip->TickTime * TimeSystem::Time.DeltaTime;

		if(d3dRenderer.CheckFrustumCulling(&gameObject))
			for (auto& nodeAni : currClip->nodeAnimations)
			{
				nodeAni.Evaluate(elapsedTime);
			}

		while (elapsedTime >= currClip->Duration)
		{
			if (isLoop)
			{
				elapsedTime -= currClip->Duration;
				currClip->ResetCashIndex();
			}
			else
			{ 
				isPause = true;
				break;
			}
		}
	}
}

void TransformAnimation::Clip::NodeAnimation::Evaluate(float elapsedTime)
{
	// 위치, 회전, 스케일에 대한 키를 찾기
	PositionKey* currPositionKey = nullptr;
	PositionKey* nextPositionKey = nullptr;
	for (int i = lastPosIndex; i < positionKeys->size(); i++)
	{
		if (positionKeys->data()[i].Time >= elapsedTime)
		{
			currPositionKey = &positionKeys->data()[i - 1];
			nextPositionKey = &positionKeys->data()[i];
			lastPosIndex = i; 
			break;
		}
	}

	RotationKey* currRotationKey = nullptr;
	RotationKey* nextRotationKey = nullptr;
	for (int i = lastRotIndex; i < rotationKeys->size(); i++)
	{
		if (rotationKeys->data()[i].Time >= elapsedTime)
		{
			currRotationKey = &rotationKeys->data()[i - 1];
			nextRotationKey = &rotationKeys->data()[i];
			lastRotIndex = i;
			break;
		}
	}

	ScaleKey* currScaleKey = nullptr;
	ScaleKey* nextScaleKey = nullptr;
	for (int i = lastScaleIndex; i < scaleKeys->size(); i++)
	{
		if (scaleKeys->data()[i].Time >= elapsedTime)
		{
			currScaleKey = &scaleKeys->data()[i - 1];
			nextScaleKey = &scaleKeys->data()[i];
			lastScaleIndex = i;
			break;
		}
	}

	float t = 0;
	// 위치 보간
	if (currPositionKey && nextPositionKey) 
	{
		t = (elapsedTime - currPositionKey->Time) / (nextPositionKey->Time - currPositionKey->Time);
		objTarget->transform.localPosition = Vector3::Lerp(currPositionKey->position, nextPositionKey->position, t);
	}
	else if (currPositionKey) 
	{
		objTarget->transform.localPosition = currPositionKey->position;
	}

	// 회전 보간
	if (currRotationKey && nextRotationKey) 
	{
		t = (elapsedTime - currRotationKey->Time) / (nextRotationKey->Time - currRotationKey->Time);
		objTarget->transform.localRotation = Quaternion::Slerp(currRotationKey->rotation, nextRotationKey->rotation, t);
	}
	else if (currRotationKey)
	{
		objTarget->transform.localRotation = currRotationKey->rotation;
	}

	// 스케일 보간
	if (currScaleKey && nextScaleKey) 
	{
		t = (elapsedTime - currScaleKey->Time) / (nextScaleKey->Time - currScaleKey->Time);
		objTarget->transform.localScale = Vector3::Lerp(currScaleKey->scale, nextScaleKey->scale, t);
	}
	else if (currScaleKey)
	{
		objTarget->transform.localScale = currScaleKey->scale;
	}
}