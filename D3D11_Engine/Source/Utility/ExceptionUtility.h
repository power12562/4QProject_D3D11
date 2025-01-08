#pragma once
#include <exception>
#include <string>
class GameObject;
class ObjectException : public std::exception
{
public:
	ObjectException(
		const char* message, 
		GameObject* object, 
		const char* file, 
		const char* func, 
		int line) 
		: std::exception(message)
	{
		obj = object;
		fileName = file;
		funcName = func;
		lineNum  = line;
	}
	GameObject*  GetGameObject()    const { return obj; }
	const char*  GetFileName()		const { return fileName.c_str(); }
	const char*  GetFuncName()		const { return funcName.c_str(); }
	int			 GetLineNum()		const { return lineNum; }

private:
	GameObject* obj;
	std::string fileName;
	std::string funcName;
	int lineNum;
};

#define throw_GameObject(Message, gameObject) \
    throw ObjectException(                   \
        Message,                             \
        gameObject,                          \
        __FILE__,							 \
        __func__,							 \
        __LINE__)