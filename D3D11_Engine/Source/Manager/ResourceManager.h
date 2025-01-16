#pragma once
#include <map>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <Utility/utfConvert.h>
#include <Utility/Console.h>
#include <DrawCommand.h>

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

template<typename T>
ResourceManager<T>& GetResourceManager()
{
    return ResourceManager<T>::instance();
}
