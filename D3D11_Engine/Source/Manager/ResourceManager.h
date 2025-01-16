#pragma once
#include <map>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <D3DCore/D3DRenderer.h>
#include <Utility/utfConvert.h>
#include <Utility/Console.h>

#include <Sound/FMODFramework.h>
#include <Sound/SoundSystem.h>

class Resource
{
public:
    static void ClearResourceManagers();
protected:
    inline static std::vector<std::function<void()>> clearList;
};

template <typename T>
class ResourceManager : public Resource
{
public:
    /*GetInstance*/
    static ResourceManager<T>& instance()
    {
        static ResourceManager<T> instance;
        return instance;
    }
private:
    ResourceManager()
    {
        auto clearFunc = [this]() { this->Clear(); };
        clearList.push_back(clearFunc);
    }
    ~ResourceManager()
    {
        Clear();
    }

private:
	std::map<std::wstring, std::weak_ptr<T>> resourceMap;
public:
	void Clear()
    {
        resourceMap.clear();
    }
	std::shared_ptr<T> GetResource(const wchar_t* key)
    {
        auto findIter = resourceMap.find(key);

        if (findIter != resourceMap.end())
        {
            if (std::shared_ptr<T> resource = findIter->second.lock())
            {
                return  resource;
            }
        }
        std::shared_ptr<T> resource = std::make_shared<T>();
        resourceMap[key] = resource;
        return resource;
    }
	std::shared_ptr<T> operator[](const wchar_t* key) { return GetResource(key); }
};


template <>
class ResourceManager<DRAW_INDEX_DATA> : public Resource
{
public:
    /*GetInstance*/
    static ResourceManager<DRAW_INDEX_DATA>& instance()
    {
        static ResourceManager<DRAW_INDEX_DATA> instance;
        return instance;
    }
private:
    ResourceManager()
    {
        auto clearFunc = [this]() { this->Clear(); };
        clearList.push_back(clearFunc);
    }
    ~ResourceManager()
    {
        Clear();
    }

public:
    using DRAW_INDEX_DATA_LIST = std::vector<std::weak_ptr<DRAW_INDEX_DATA>>;
private:
    std::map<std::wstring, DRAW_INDEX_DATA_LIST> resourceMap;
public:
    void Clear()
    {
        resourceMap.clear();
    }
    bool isResource(const wchar_t* key, int index)
    {
        auto findIter = resourceMap.find(key);
        if (findIter != resourceMap.end())
        {
            if (findIter->second.size() > index && !findIter->second[index].expired())
                    return true;
        }
        return false;
    }
    std::shared_ptr<DRAW_INDEX_DATA> GetResource(const wchar_t* key, int index)
    {
        if (resourceMap[key].size() <= index)
            resourceMap[key].resize(index + 1);

        auto findIter = resourceMap.find(key);
        if (findIter != resourceMap.end())
        {
            if (std::shared_ptr<DRAW_INDEX_DATA> resource = findIter->second[index].lock())
            {
                return  resource;
            }
        }
        std::shared_ptr<DRAW_INDEX_DATA> resource = std::make_shared<DRAW_INDEX_DATA>();
        resourceMap[key][index] = resource;
        return resource;
    }
};

template <>
class ResourceManager<FMOD::Sound> : public Resource
{
public:
    /*GetInstance*/
    static ResourceManager<FMOD::Sound>& instance()
    {
        static ResourceManager<FMOD::Sound> instance;
        return instance;
    }
private:
    ResourceManager()
    {
        auto clearFunc = [this]() { this->Clear(); };
        clearList.push_back(clearFunc);
    }
    ~ResourceManager()
    {
        Clear();
    }

private:
    std::map<std::wstring, std::weak_ptr<FMOD::Sound>> resourceMap;
public:
    void Clear()
    {
        resourceMap.clear();
    }
    bool isResource(const wchar_t* key)
    {
        return resourceMap.find(key) != resourceMap.end() ? true : false;
    }
    std::shared_ptr<FMOD::Sound> AddResource(std::wstring_view key, std::wstring_view path)
    {
        if (resourceMap.find(key.data()) == resourceMap.end())
        {
            FMOD::Sound* _sound = SoundSystem::GetInstance().CreateSound(path);
            std::shared_ptr<FMOD::Sound> sound(_sound, [](FMOD::Sound* sound) { if (sound) sound->release(); });
            resourceMap[key.data()] = sound;
            return sound;
        }
        else
        {
            printf("Resource already exists : FMOD::Sound / %s\n", key.data());
            return resourceMap[key.data()].lock();
        }
    }
    std::shared_ptr<FMOD::Sound> GetResource(std::wstring_view key)
    {
        if (resourceMap.find(key.data()) == resourceMap.end())
        {
            printf("Resource not found : FMOD::Sound / %s\n", key.data());
            return nullptr;
        }
        else
        {
            return resourceMap[key.data()].lock();
        }
    }
};

template<typename T>
ResourceManager<T>& GetResourceManager()
{
    return ResourceManager<T>::instance();
}
