#include "D3DTexture2D.h"
#include <Utility/AssimpUtility.h>
#include <Manager/TextureManager.h>
#include <Utility/ImguiHelper.h>
#include <Utility/SerializedUtility.h>

D3DTexture2D::D3DTexture2D()
{
	resize(E_TEXTURE::PBRTextureCount);
	SetDefaultTexture(E_TEXTURE::Albedo, E_TEXTURE_DEFAULT::ONE);
	SetDefaultTexture(E_TEXTURE::Normal, E_TEXTURE_DEFAULT::ZERO);
	SetDefaultTexture(E_TEXTURE::Specular, E_TEXTURE_DEFAULT::ONE);
	SetDefaultTexture(E_TEXTURE::Emissive, E_TEXTURE_DEFAULT::ZERO);
	SetDefaultTexture(E_TEXTURE::Opacity, E_TEXTURE_DEFAULT::ONE);
	SetDefaultTexture(E_TEXTURE::Metalness, E_TEXTURE_DEFAULT::ZERO);
	SetDefaultTexture(E_TEXTURE::Roughness, E_TEXTURE_DEFAULT::ZERO);
	SetDefaultTexture(E_TEXTURE::AmbientOcculusion, E_TEXTURE_DEFAULT::ONE);
}

D3DTexture2D::~D3DTexture2D()
{
	for (std::wstring& path : pathList)
	{
		if (path != L"")
			textureManager.ReleaseSharingTexture(path.c_str());
	}
}

void D3DTexture2D::resize(size_t newsize)
{
	if (newsize < D3DTexture2D::MAX_SIZE)
	{
		if (newsize < SRVList.size())
		{
			for (int i = (int)newsize; i < (int)SRVList.size(); i++)
			{
				ResetTexture2D(i);
			}
		}
		SRVList.resize(newsize);
		pathList.resize(newsize);
	}
	else
	{
		__debugbreak();
		//사이즈 제한 초과
	}
}

void D3DTexture2D::SetCubeMapTexture(int index, const wchar_t* path)
{
	ResetTexture2D(index);
	textureManager.CreateSharingCubeMap(path, &SRVList[index]);
	pathList[index] = path;
}

void D3DTexture2D::SetTexture2D(int index, const wchar_t* path)
{
	ResetTexture2D(index);
	textureManager.CreateSharingTexture(path, &SRVList[index]);
	pathList[index] = path;
}


void D3DTexture2D::SetDefaultTexture(int index, E_TEXTURE_DEFAULT::DEFAULT_TEXTURE type)
{
	ResetTexture2D(index);
	SRVList[index] = textureManager.GetDefaultTexture(type);
}

void D3DTexture2D::ResetTexture2D(int index)
{
	if (SRVList[index])
	{
		if(pathList[index] != L"")
			textureManager.ReleaseSharingTexture(pathList[index].c_str());

		SRVList[index] = nullptr;
		pathList[index].clear();
	}
}

void D3DTexture2D::Serialized(std::ofstream& ofs)
{
	using namespace Binary;
	Write::data(ofs, pathList.size());
	for (size_t i = 0; i < pathList.size(); i++)
	{
		Write::wstring(ofs, pathList[i]);
	}
}

void D3DTexture2D::Deserialized(std::ifstream& ifs)
{
	using namespace Binary;
	size_t size = Read::data<size_t>(ifs);
	pathList.resize(size);
	SRVList.resize(size);
	for (size_t i = 0; i < size; i++)
	{
		pathList[i] = Read::wstring(ifs);
	}
	ReloadTexture();
}

D3DTexture2D::D3DTexture2D(const D3DTexture2D& rhs)
{
	if (this == &rhs)
		return;

	SRVList = rhs.SRVList;
	pathList = rhs.pathList;
	for (int i = 0; i < rhs.pathList.size(); ++i)
	{
		if(pathList[i] != L"")
			textureManager.CreateSharingTexture(pathList[i].c_str(), &SRVList[i]);
	}
}

ID3D11ShaderResourceView* D3DTexture2D::operator[](int index)
{
	if (0 <= index && index < SRVList.size())
	{
		return SRVList[index];
	}
	return nullptr;
}

D3DTexture2D& D3DTexture2D::operator=(const D3DTexture2D& rhs)
{
	if (this == &rhs)
		return *this;
	
	for (int i = 0; i < rhs.pathList.size(); ++i)
	{
		ResetTexture2D(i);
	}
	SRVList = rhs.SRVList;
	pathList = rhs.pathList;
	for (int i = 0; i < rhs.pathList.size(); ++i)
	{
		if (pathList[i] != L"")
		{
			if(!ImGui::ReloadTextureCompressEnd(pathList[i].c_str(), this, i)) //압축중인지 확인.
				textureManager.CreateSharingTexture(pathList[i].c_str(), &SRVList[i]); //바로 로드
		}	
	}
	return *this;
}

void D3DTexture2D::ReloadTexture()
{
	for (int i = 0; i < pathList.size(); ++i)
	{
		if (pathList[i] != L"")
		{
			textureManager.CreateSharingTexture(pathList[i].c_str(), &SRVList[i]);
		}	
	}
}

