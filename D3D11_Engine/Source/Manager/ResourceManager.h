#pragma once
#include <map>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <D3DCore/D3DRenderer.h>
#include <Utility/utfConvert.h>
#include <Utility/Console.h>

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

template<typename T>
ResourceManager<T>& GetResourceManager()
{
    return ResourceManager<T>::instance();
}
