#include "D3D11Utility.h"
#include <comdef.h>
#include <d3dcompiler.h>
#include <Directxtk/DDSTextureLoader.h>
#include <Directxtk/WICTextureLoader.h>
#include <iostream>
#include <filesystem>

namespace Utility
{
	using namespace DirectX;

	LPCWSTR GetComErrorString(HRESULT hr)
	{
		_com_error err(hr);
		LPCWSTR errMsg = err.ErrorMessage();
		return errMsg;
	}

	HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
	{
		HRESULT hr = S_OK;

		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PARTIAL_PRECISION;
#ifdef _DEBUG
		// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
		// Setting this flag improves the shader debugging experience, but still allows 
		// the shaders to be optimized and to run exactly the way they will run in 
		// the release configuration of this program.
		dwShaderFlags |= D3DCOMPILE_DEBUG;

		// Disable optimizations to further improve shader debugging
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		dwShaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

		ID3DBlob* pErrorBlob = nullptr;
		hr = D3DCompileFromFile(szFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel,
			dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

		if (FAILED(hr))
		{
			if (pErrorBlob)
			{
				MessageBoxA(NULL, (char*)pErrorBlob->GetBufferPointer(), "CompileShaderFromFile", MB_OK);
				pErrorBlob->Release();
			}
			return hr;
		}
		if (pErrorBlob) pErrorBlob->Release();

		return S_OK;
	}

	HRESULT LoadShadeFormFile(const WCHAR* szFileName, ID3DBlob** ppBlobOut)
	{
		HRESULT hr{};
		hr = D3DReadFileToBlob(szFileName, ppBlobOut);
		if (FAILED(hr))
		{
			_com_error err(hr);
			MessageBox(NULL, err.ErrorMessage(), L"LoadShadeFormFile", MB_OK);
			return hr;
		}
		return S_OK;
	}

	HRESULT CreateTextureFromFile(ID3D11Device* d3dDevice, const wchar_t* szFileName, ID3D11Resource** texture, ID3D11ShaderResourceView** textureView)
	{
		HRESULT hr = S_OK;
		auto ShowErrorMessageBox = [&]()
			{
				MessageBoxW(NULL, GetComErrorString(hr), szFileName, MB_OK);
			};

		std::filesystem::path filePath = szFileName;
		DX_TEXTURE_EXTENSION extension = GetTexureExtension(filePath.extension().wstring());


		switch (extension)
		{
		case Utility::DX_TEXTURE_EXTENSION::tga:
		{
			DirectX::TexMetadata metaData;
			DirectX::ScratchImage scratchImage;
			hr = DirectX::LoadFromTGAFile(szFileName, &metaData, scratchImage);
			if (SUCCEEDED(hr))
			{
				if (texture)
				{
					hr = DirectX::CreateTexture(d3dDevice, scratchImage.GetImages(), scratchImage.GetImageCount(), metaData, texture);
					if (FAILED(hr))
					{
						ShowErrorMessageBox();
						return hr;
					}
					D3D_SET_OBJECT_NAME(*texture, L"D3DUtility");
				}
				if (textureView)
				{
					hr = DirectX::CreateShaderResourceView(d3dDevice, scratchImage.GetImages(), scratchImage.GetImageCount(), metaData, textureView);
					if (FAILED(hr))
					{
						ShowErrorMessageBox();
						return hr;
					}
					D3D_SET_OBJECT_NAME(*textureView, szFileName);
				}
			}
			else
			{
				ShowErrorMessageBox();
				return hr;
			}
		}
		break;

		case Utility::DX_TEXTURE_EXTENSION::dds:
			hr = DirectX::CreateDDSTextureFromFile(d3dDevice, szFileName, texture, textureView);
			if (FAILED(hr))
			{
				ShowErrorMessageBox();
				return hr;
			}
			break;

		default:
			hr = DirectX::CreateWICTextureFromFile(d3dDevice, szFileName, texture, textureView);
			if (FAILED(hr))
			{
				ShowErrorMessageBox();
				return hr;
			}
			break;
		}
		return S_OK;
	}

	HRESULT CreateCubeMapFromFile(ID3D11Device* d3dDevice, const wchar_t* szFileName, ID3D11Resource** texture, ID3D11ShaderResourceView** textureView)
	{
		using namespace DirectX;

		HRESULT hr = S_OK;
		auto ShowErrorMessageBox = [&]()
			{
				MessageBoxW(NULL, GetComErrorString(hr), szFileName, MB_OK);
			};

		std::filesystem::path filePath = szFileName;
		DX_TEXTURE_EXTENSION extension = GetTexureExtension(filePath.extension().wstring());

		switch (extension)
		{
		case Utility::DX_TEXTURE_EXTENSION::dds:
		{
			//DDS 로드
			TexMetadata metadata;
			ScratchImage scratchImage;
			ID3D11Texture2D* tempTexture = nullptr;
			hr = LoadFromDDSFile(szFileName, DDS_FLAGS_NONE, &metadata, scratchImage);
			if (FAILED(hr))
			{
				ShowErrorMessageBox();
				return hr;
			}

			// 큐브맵인지 확인
			if (!(metadata.miscFlags & TEX_MISC_TEXTURECUBE))
			{
				MessageBoxW(NULL, L"Cube Map 형식이 아닙니다.", szFileName, MB_OK);
				return E_FAIL; // DDS 파일이 큐브맵 형식이 아님
			}

			// 텍스처 생성    
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = (UINT)metadata.width;
			desc.Height = (UINT)metadata.height;
			desc.MipLevels = (UINT)metadata.mipLevels;
			desc.ArraySize = (UINT)metadata.arraySize;
			desc.Format = metadata.format;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

			//데이터 복사
			std::vector<D3D11_SUBRESOURCE_DATA> initData(scratchImage.GetImageCount());
			const Image* images = scratchImage.GetImages();
			for (UINT i = 0; i < initData.size(); ++i)
			{
				initData[i].pSysMem = images[i].pixels;
				initData[i].SysMemPitch = (UINT)images[i].rowPitch;
				initData[i].SysMemSlicePitch = (UINT)images[i].slicePitch;
			}

			hr = d3dDevice->CreateTexture2D(&desc, initData.data(), &tempTexture);
			if (FAILED(hr))
			{
				ShowErrorMessageBox();
				return hr; // 텍스처 생성 실패
			}
			D3D_SET_OBJECT_NAME(tempTexture, L"CreateCubeMapFromFile");

			// Shader Resource View 생성
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = desc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = desc.MipLevels;
			if (textureView)
			{
				hr = d3dDevice->CreateShaderResourceView(tempTexture, &srvDesc, textureView);
				if (FAILED(hr))
				{
					ShowErrorMessageBox();
					return hr; // 텍스처 생성 실패
				}
				D3D_SET_OBJECT_NAME(*textureView, szFileName);
			}
			if (texture)
			{
				*texture = tempTexture;
			}
			else
			{
				tempTexture->Release();
			}
			return hr;
		}
		default:
			MessageBoxW(NULL, L"Cube Map은 dds 파일만 사용 가능 합니다.", szFileName, MB_OK);
			return S_FALSE;
		}
	}

	std::shared_ptr<DirectX::ScratchImage> CreateCompressTexture(ID3D11Device* d3dDevice, const wchar_t* szFileName, ID3D11Resource** texture, ID3D11ShaderResourceView** textureView, E_COMPRESS::TYPE type)
	{
		HRESULT hr = S_OK;
		auto ShowErrorMessageBox = [&]()
			{
				MessageBoxW(NULL, GetComErrorString(hr), szFileName, MB_OK);
			};

		std::filesystem::path filePath = szFileName;
		DX_TEXTURE_EXTENSION extension = GetTexureExtension(filePath.extension().wstring());
		DirectX::TexMetadata metaData;
		std::unique_ptr<DirectX::ScratchImage> scratchImage = std::make_unique<DirectX::ScratchImage>();
		switch (extension)
		{
		case Utility::DX_TEXTURE_EXTENSION::tga:
		{
			hr = DirectX::LoadFromTGAFile(szFileName, &metaData, *scratchImage);
			if (FAILED(hr))
			{
				ShowErrorMessageBox();
				CheckHRESULT(hr);
			}
			break;
		}
		case Utility::DX_TEXTURE_EXTENSION::dds:
		{
			hr = DirectX::LoadFromDDSFile(szFileName, DDS_FLAGS_NONE, &metaData, *scratchImage);
			if (FAILED(hr))
			{
				ShowErrorMessageBox();
				CheckHRESULT(hr);
			}
			break;
		}
		default:
		{
			hr = DirectX::LoadFromWICFile(szFileName, WIC_FLAGS_NONE, &metaData, *scratchImage);
			if (FAILED(hr))
			{
				ShowErrorMessageBox();
				CheckHRESULT(hr);
			}
			break;
		}
		}

		if (scratchImage->GetMetadata().mipLevels <= 1 && scratchImage->GetMetadata().width > 1 && scratchImage->GetMetadata().height > 1)
		{
			std::unique_ptr<ScratchImage> tempimage(new (std::nothrow) ScratchImage);
			hr = GenerateMipMaps(scratchImage->GetImages(), scratchImage->GetImageCount(), scratchImage->GetMetadata(), TEX_FILTER_DEFAULT, 0, *tempimage);
			CheckHRESULT(hr);
			std::swap(scratchImage, tempimage);
		}

		if (!IsCompressed(scratchImage->GetMetadata().format))
		{
			std::unique_ptr<ScratchImage> compressedImage(new (std::nothrow) ScratchImage);
			DXGI_FORMAT compressFormat;
			TEX_COMPRESS_FLAGS compressFlag = TEX_COMPRESS_PARALLEL;
			switch (type)
			{
			case E_COMPRESS::BC1:
				compressFormat = DXGI_FORMAT_BC1_UNORM;
				compressFlag |= TEX_COMPRESS_DITHER | TEX_COMPRESS_UNIFORM | TEX_COMPRESS_SRGB;
				break;
			case E_COMPRESS::BC3:
				compressFormat = DXGI_FORMAT_BC3_UNORM;
				compressFlag |= TEX_COMPRESS_UNIFORM | TEX_COMPRESS_SRGB;
				break;
			case E_COMPRESS::BC4:
				compressFormat = DXGI_FORMAT_BC4_UNORM;
				break;
			case E_COMPRESS::BC5:
				compressFormat = DXGI_FORMAT_BC5_SNORM;
				break;
			case E_COMPRESS::BC6:
				compressFormat = DXGI_FORMAT_BC6H_SF16;
				break;
			case E_COMPRESS::BC7:
				compressFormat = DXGI_FORMAT_BC7_UNORM;
				compressFlag |= TEX_COMPRESS_BC7_QUICK;
				break;
			default:
				compressFormat = DXGI_FORMAT_UNKNOWN;
				break;
			}
			hr = Compress(scratchImage->GetImages(), scratchImage->GetImageCount(), scratchImage->GetMetadata(), compressFormat, compressFlag, TEX_ALPHA_WEIGHT_DEFAULT, *compressedImage);
			CheckHRESULT(hr);
			std::swap(scratchImage, compressedImage);
		}

		ComPtr<ID3D11Resource> tempTexture;
		hr = CreateTexture(d3dDevice, 
			scratchImage->GetImages(),
			scratchImage->GetImageCount(),
			scratchImage->GetMetadata(),
			&tempTexture);
		CheckHRESULT(hr);

		hr = d3dDevice->CreateShaderResourceView(tempTexture.Get(), nullptr, textureView);
		CheckHRESULT(hr);
		return std::move(scratchImage);
	}

	HRESULT SaveTextureForDDS(const wchar_t* szFileName, const std::shared_ptr<DirectX::ScratchImage>& image)
	{
		std::filesystem::path originPath = szFileName;
		originPath.replace_extension(L".dds");
		if (!std::filesystem::exists(originPath.parent_path())) 
		{
			std::filesystem::create_directories(originPath.parent_path());
		}
		return SaveToDDSFile(
			*image->GetImages(),      // 텍스처 이미지 데이터
			DDS_FLAGS_NONE,          // DDS 플래그
			originPath.c_str()
		);
	}

	UINT GetMipmapLevels(UINT width, UINT height)
	{
		UINT levels = 1;
		while ((width | height) >> levels) {
			++levels;
		}
		return levels;
	}

	DXGI_FORMAT GetDXGIFormat(D3D_REGISTER_COMPONENT_TYPE componentType, UINT mask)
	{
		switch (componentType)
		{
		case D3D_REGISTER_COMPONENT_FLOAT32:
		{
			switch (mask)
			{
			case 0b0001:  return DXGI_FORMAT_R32_FLOAT;            // 1 성분 (R)
			case 0b0011:  return DXGI_FORMAT_R32G32_FLOAT;         // 2 성분 (RG)
			case 0b0111:  return DXGI_FORMAT_R32G32B32_FLOAT;      // 3 성분 (RGB)
			case 0b1111:  return DXGI_FORMAT_R32G32B32A32_FLOAT;   // 4 성분 (RGBA)
			default: return DXGI_FORMAT_UNKNOWN;
			}
		}

		case D3D_REGISTER_COMPONENT_UINT32:
		{
			switch (mask)
			{
			case 0b0001:  return DXGI_FORMAT_R32_UINT;             // 1 성분 (R)
			case 0b0011:  return DXGI_FORMAT_R32G32_UINT;          // 2 성분 (RG)
			case 0b0111:  return DXGI_FORMAT_R32G32B32_UINT;       // 3 성분 (RGB)
			case 0b1111:  return DXGI_FORMAT_R32G32B32A32_UINT;    // 4 성분 (RGBA)
			default: return DXGI_FORMAT_UNKNOWN;
			}
		}

		case D3D_REGISTER_COMPONENT_SINT32:
		{
			switch (mask)
			{
			case 0b0001:  return DXGI_FORMAT_R32_SINT;             // 1 성분 (R)
			case 0b0011:  return DXGI_FORMAT_R32G32_SINT;          // 2 성분 (RG)
			case 0b0111:  return DXGI_FORMAT_R32G32B32_SINT;       // 3 성분 (RGB)
			case 0b1111:  return DXGI_FORMAT_R32G32B32A32_SINT;    // 4 성분 (RGBA)
			default: return DXGI_FORMAT_UNKNOWN;
			}
		}

		case D3D_REGISTER_COMPONENT_FLOAT16:
		{
			switch (mask)
			{
			case 0b0001:  return DXGI_FORMAT_R16_FLOAT;            // 1 성분 (R)
			case 0b0011:  return DXGI_FORMAT_R16G16_FLOAT;         // 2 성분 (RG)
			case 0b1111:  return DXGI_FORMAT_R16G16B16A16_FLOAT;   // 4 성분 (RGBA)
			default: return DXGI_FORMAT_UNKNOWN;
			}
		}

		case D3D_REGISTER_COMPONENT_UINT16:
		{
			switch (mask)
			{
			case 0b0001:  return DXGI_FORMAT_R16_UINT;             // 1 성분 (R)
			case 0b0011:  return DXGI_FORMAT_R16G16_UINT;          // 2 성분 (RG)
			case 0b1111:  return DXGI_FORMAT_R16G16B16A16_UINT;    // 4 성분 (RGBA)
			default: return DXGI_FORMAT_UNKNOWN;
			}
		}

		case D3D_REGISTER_COMPONENT_SINT16:
		{
			switch (mask)
			{
			case 0b0001:  return DXGI_FORMAT_R16_SINT;             // 1 성분 (R)
			case 0b0011:  return DXGI_FORMAT_R16G16_SINT;          // 2 성분 (RG)
			case 0b1111:  return DXGI_FORMAT_R16G16B16A16_SINT;    // 4 성분 (RGBA)
			default: return DXGI_FORMAT_UNKNOWN;
			}
		}
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	bool XMMatrixIsNaN(const DirectX::XMMATRIX& matrix)
	{
		return DirectX::XMVector4IsNaN(matrix.r[0]) || DirectX::XMVector4IsNaN(matrix.r[1]) ||
			DirectX::XMVector4IsNaN(matrix.r[2]) || DirectX::XMVector4IsNaN(matrix.r[3]);
	}

	void CalculateBoundsFrustumCorners(const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, XMFLOAT3 (&out_frustumCorners)[BoundingFrustum::CORNER_COUNT])
	{
		// 1. View-Projection 행렬 계산 및 역변환
		XMMATRIX viewProjectionMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);
		XMMATRIX invViewProjectionMatrix = XMMatrixInverse(nullptr, viewProjectionMatrix);

		// 2. NDC 좌표 정의
		static const XMFLOAT3 ndcCorners[8] =
		{
			 { -1.0f, -1.0f, 0.0f }, // Near bottom-left
			 {  1.0f, -1.0f, 0.0f }, // Near bottom-right
			 {  1.0f,  1.0f, 0.0f }, // Near top-right
			 { -1.0f,  1.0f, 0.0f }, // Near top-left
			 { -1.0f, -1.0f, 1.0f }, // Far bottom-left
			 {  1.0f, -1.0f, 1.0f }, // Far bottom-right
			 {  1.0f,  1.0f, 1.0f }, // Far top-right
			 { -1.0f,  1.0f, 1.0f }  // Far top-left
		};

		// NDC -> World 좌표 변환
		for (size_t i = 0; i < BoundingFrustum::CORNER_COUNT; ++i)
		{
			XMVECTOR corner = XMVectorSet(ndcCorners[i].x, ndcCorners[i].y, ndcCorners[i].z, 1.0f);
			corner = XMVector3TransformCoord(corner, invViewProjectionMatrix);

			XMStoreFloat3(&out_frustumCorners[i], corner);
		}
		return;
	}

	DirectX::BoundingBox CalculateBoundsLightSpace(const DirectX::XMMATRIX& ViewMatrix, const DirectX::XMFLOAT3(&frustumCorners)[DirectX::BoundingFrustum::CORNER_COUNT])
	{
		BoundingBox lightSpaceBounds;

		// Frustum Corners를 빛의 View 공간으로 변환
		XMFLOAT3 minPoint(FLT_MAX, FLT_MAX, FLT_MAX);
		XMFLOAT3 maxPoint(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (const auto& corner : frustumCorners)
		{
			XMVECTOR cornerVec = XMVector3Transform(XMLoadFloat3(&corner), ViewMatrix);

			XMFLOAT3 transformedCorner;
			XMStoreFloat3(&transformedCorner, cornerVec);

			minPoint.x = std::min<float>(minPoint.x, transformedCorner.x);
			minPoint.y = std::min<float>(minPoint.y, transformedCorner.y);
			minPoint.z = std::min<float>(minPoint.z, transformedCorner.z);

			maxPoint.x = std::max<float>(maxPoint.x, transformedCorner.x);
			maxPoint.y = std::max<float>(maxPoint.y, transformedCorner.y);
			maxPoint.z = std::max<float>(maxPoint.z, transformedCorner.z);
		}

		// BoundingBox 생성
		lightSpaceBounds.Center = {
			(minPoint.x + maxPoint.x) * 0.5f,
			(minPoint.y + maxPoint.y) * 0.5f,
			(minPoint.z + maxPoint.z) * 0.5f
		};

		lightSpaceBounds.Extents = {
			(maxPoint.x - minPoint.x) * 0.5f,
			(maxPoint.y - minPoint.y) * 0.5f,
			(maxPoint.z - minPoint.z) * 0.5f
		};

		return lightSpaceBounds;
	}

	XMMATRIX CreateOrthographicProjection(const BoundingBox& boundingBox, float nearPlane, float farPlane)
	{
		float left = boundingBox.Center.x - boundingBox.Extents.x;
		float right = boundingBox.Center.x + boundingBox.Extents.x;
		float bottom = boundingBox.Center.y - boundingBox.Extents.y;
		float top = boundingBox.Center.y + boundingBox.Extents.y;
		return XMMatrixOrthographicOffCenterLH(left, right, bottom, top, nearPlane, farPlane);
	}

	void ConvertLHtoRHViewAndProj(const XMMATRIX& viewLH, const XMMATRIX& projLH, float* viewRH_m16, float* projRH_m16)
	{
		// Z축 반전 행렬
		XMMATRIX zFlip = XMMatrixScaling(1.0f, 1.0f, -1.0f);

		// 변환된 오른손 좌표계 행렬
		XMMATRIX viewRH = viewLH * zFlip;
		XMMATRIX projRH = projLH * zFlip;

		// 결과를 m16 배열로 저장
		XMFLOAT4X4 viewMatrixRH, projMatrixRH;
		XMStoreFloat4x4(&viewMatrixRH, viewRH);
		XMStoreFloat4x4(&projMatrixRH, projRH);

		// 메모리 복사
		memcpy(viewRH_m16, &viewMatrixRH, sizeof(float) * 16);
		memcpy(projRH_m16, &projMatrixRH, sizeof(float) * 16);
	}

	DX_TEXTURE_EXTENSION GetTexureExtension(const std::wstring& flieString)
	{
		if (flieString.compare(L".tga") == 0)
			return DX_TEXTURE_EXTENSION::tga;
		else if (flieString.compare(L".dds") == 0)
			return DX_TEXTURE_EXTENSION::dds;
		else
			return DX_TEXTURE_EXTENSION::null;
	}

	void RetrieveVertexBufferData(ID3D11DeviceContext* context, ID3D11Device* device, ID3D11Buffer* vertexBuffer, void* outputData, size_t bufferSize)
	{
		//desc
		D3D11_BUFFER_DESC bufferDesc;
		vertexBuffer->GetDesc(&bufferDesc);

		// Staging Buffer 생성
		D3D11_BUFFER_DESC stagingDesc = bufferDesc;
		stagingDesc.Usage = D3D11_USAGE_STAGING; // Staging 용도로 설정
		stagingDesc.BindFlags = 0;               // Bind Flags는 0이어야 함
		stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

		ID3D11Buffer* stagingBuffer = nullptr;
		CheckHRESULT(device->CreateBuffer(&stagingDesc, nullptr, &stagingBuffer));

		if (stagingBuffer)
		{
			// GPU 데이터를 Staging Buffer로 복사
			context->CopyResource(stagingBuffer, vertexBuffer);

			// Staging Buffer에서 데이터를 CPU로 복사
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			CheckHRESULT(context->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &mappedResource));
			memcpy(outputData, mappedResource.pData, bufferSize);
			context->Unmap(stagingBuffer, 0);
			stagingBuffer->Release();
		}
	}

	void RetrieveIndexBufferData(ID3D11DeviceContext* context, ID3D11Device* device, ID3D11Buffer* indexBuffer, void* outputData, size_t bufferSize)
	{
		//desc
		D3D11_BUFFER_DESC bufferDesc;
		indexBuffer->GetDesc(&bufferDesc);

		// Staging Buffer 생성
		D3D11_BUFFER_DESC stagingDesc = bufferDesc;
		stagingDesc.Usage = D3D11_USAGE_STAGING;
		stagingDesc.BindFlags = 0;
		stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

		ID3D11Buffer* stagingBuffer = nullptr;
		CheckHRESULT(device->CreateBuffer(&stagingDesc, nullptr, &stagingBuffer));

		if (stagingBuffer)
		{
			// GPU 데이터를 Staging Buffer로 복사
			context->CopyResource(stagingBuffer, indexBuffer);

			// Staging Buffer에서 데이터를 CPU로 복사
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			CheckHRESULT(context->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &mappedResource));
			memcpy(outputData, mappedResource.pData, bufferSize);
			context->Unmap(stagingBuffer, 0);
			stagingBuffer->Release();
		}
	}
}

namespace DebugDraw
{
	using namespace DirectX;

	inline void XM_CALLCONV DrawCube(PrimitiveBatch<VertexPositionColor>* batch,
		CXMMATRIX matWorld,
		FXMVECTOR color)
	{
		static const XMVECTORF32 s_verts[8] =
		{
			{ { { -1.f, -1.f, -1.f, 0.f } } },
			{ { {  1.f, -1.f, -1.f, 0.f } } },
			{ { {  1.f, -1.f,  1.f, 0.f } } },
			{ { { -1.f, -1.f,  1.f, 0.f } } },
			{ { { -1.f,  1.f, -1.f, 0.f } } },
			{ { {  1.f,  1.f, -1.f, 0.f } } },
			{ { {  1.f,  1.f,  1.f, 0.f } } },
			{ { { -1.f,  1.f,  1.f, 0.f } } }
		};

		static const WORD s_indices[] =
		{
			0, 1,
			1, 2,
			2, 3,
			3, 0,
			4, 5,
			5, 6,
			6, 7,
			7, 4,
			0, 4,
			1, 5,
			2, 6,
			3, 7
		};

		VertexPositionColor verts[8];
		for (size_t i = 0; i < 8; ++i)
		{
			XMVECTOR v = XMVector3Transform(s_verts[i], matWorld);
			XMStoreFloat3(&verts[i].position, v);
			XMStoreFloat4(&verts[i].color, color);
		}

		batch->DrawIndexed(D3D_PRIMITIVE_TOPOLOGY_LINELIST, s_indices, static_cast<UINT>(std::size(s_indices)), verts, 8);
	}

	void XM_CALLCONV Draw(PrimitiveBatch<VertexPositionColor>* batch,
		const BoundingSphere& sphere,
		FXMVECTOR color)
	{
		XMVECTOR origin = XMLoadFloat3(&sphere.Center);

		const float radius = sphere.Radius;

		XMVECTOR xaxis = g_XMIdentityR0 * radius;
		XMVECTOR yaxis = g_XMIdentityR1 * radius;
		XMVECTOR zaxis = g_XMIdentityR2 * radius;

		DrawRing(batch, origin, xaxis, zaxis, color);
		DrawRing(batch, origin, xaxis, yaxis, color);
		DrawRing(batch, origin, yaxis, zaxis, color);
	}


	void XM_CALLCONV Draw(PrimitiveBatch<VertexPositionColor>* batch,
		const BoundingBox& box,
		FXMVECTOR color)
	{
		XMMATRIX matWorld = XMMatrixScaling(box.Extents.x, box.Extents.y, box.Extents.z);
		XMVECTOR position = XMLoadFloat3(&box.Center);
		matWorld.r[3] = XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

		DrawCube(batch, matWorld, color);
	}


	void XM_CALLCONV Draw(PrimitiveBatch<VertexPositionColor>* batch,
		const BoundingOrientedBox& obb,
		FXMVECTOR color)
	{
		XMMATRIX matWorld = XMMatrixRotationQuaternion(XMLoadFloat4(&obb.Orientation));
		XMMATRIX matScale = XMMatrixScaling(obb.Extents.x, obb.Extents.y, obb.Extents.z);
		matWorld = XMMatrixMultiply(matScale, matWorld);
		XMVECTOR position = XMLoadFloat3(&obb.Center);
		matWorld.r[3] = XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

		DrawCube(batch, matWorld, color);
	}


	void XM_CALLCONV Draw(PrimitiveBatch<VertexPositionColor>* batch,
		const BoundingFrustum& frustum,
		FXMVECTOR color)
	{
		XMFLOAT3 corners[BoundingFrustum::CORNER_COUNT];
		frustum.GetCorners(corners);

		VertexPositionColor verts[24] = {};
		verts[0].position = corners[0];
		verts[1].position = corners[1];
		verts[2].position = corners[1];
		verts[3].position = corners[2];
		verts[4].position = corners[2];
		verts[5].position = corners[3];
		verts[6].position = corners[3];
		verts[7].position = corners[0];

		verts[8].position = corners[0];
		verts[9].position = corners[4];
		verts[10].position = corners[1];
		verts[11].position = corners[5];
		verts[12].position = corners[2];
		verts[13].position = corners[6];
		verts[14].position = corners[3];
		verts[15].position = corners[7];

		verts[16].position = corners[4];
		verts[17].position = corners[5];
		verts[18].position = corners[5];
		verts[19].position = corners[6];
		verts[20].position = corners[6];
		verts[21].position = corners[7];
		verts[22].position = corners[7];
		verts[23].position = corners[4];

		for (size_t j = 0; j < std::size(verts); ++j)
		{
			XMStoreFloat4(&verts[j].color, color);
		}

		batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINELIST, verts, static_cast<UINT>(std::size(verts)));
	}


	void XM_CALLCONV DrawGrid(PrimitiveBatch<VertexPositionColor>* batch,
		FXMVECTOR xAxis,
		FXMVECTOR yAxis,
		FXMVECTOR origin,
		size_t xdivs,
		size_t ydivs,
		GXMVECTOR color)
	{
		xdivs = std::max<size_t>(1, xdivs);
		ydivs = std::max<size_t>(1, ydivs);

		for (size_t i = 0; i <= xdivs; ++i)
		{
			float percent = float(i) / float(xdivs);
			percent = (percent * 2.f) - 1.f;
			XMVECTOR scale = XMVectorScale(xAxis, percent);
			scale = XMVectorAdd(scale, origin);

			VertexPositionColor v1(XMVectorSubtract(scale, yAxis), color);
			VertexPositionColor v2(XMVectorAdd(scale, yAxis), color);
			batch->DrawLine(v1, v2);
		}

		for (size_t i = 0; i <= ydivs; i++)
		{
			FLOAT percent = float(i) / float(ydivs);
			percent = (percent * 2.f) - 1.f;
			XMVECTOR scale = XMVectorScale(yAxis, percent);
			scale = XMVectorAdd(scale, origin);

			VertexPositionColor v1(XMVectorSubtract(scale, xAxis), color);
			VertexPositionColor v2(XMVectorAdd(scale, xAxis), color);
			batch->DrawLine(v1, v2);
		}
	}


	void XM_CALLCONV DrawRing(PrimitiveBatch<VertexPositionColor>* batch,
		FXMVECTOR origin,
		FXMVECTOR majorAxis,
		FXMVECTOR minorAxis,
		GXMVECTOR color)
	{
		static const size_t c_ringSegments = 32;

		VertexPositionColor verts[c_ringSegments + 1];

		FLOAT fAngleDelta = XM_2PI / float(c_ringSegments);
		// Instead of calling cos/sin for each segment we calculate
		// the sign of the angle delta and then incrementally calculate sin
		// and cosine from then on.
		XMVECTOR cosDelta = XMVectorReplicate(cosf(fAngleDelta));
		XMVECTOR sinDelta = XMVectorReplicate(sinf(fAngleDelta));
		XMVECTOR incrementalSin = XMVectorZero();
		static const XMVECTORF32 s_initialCos =
		{
			1.f, 1.f, 1.f, 1.f
		};
		XMVECTOR incrementalCos = s_initialCos.v;
		for (size_t i = 0; i < c_ringSegments; i++)
		{
			XMVECTOR pos = XMVectorMultiplyAdd(majorAxis, incrementalCos, origin);
			pos = XMVectorMultiplyAdd(minorAxis, incrementalSin, pos);
			XMStoreFloat3(&verts[i].position, pos);
			XMStoreFloat4(&verts[i].color, color);
			// Standard formula to rotate a vector.
			XMVECTOR newCos = incrementalCos * cosDelta - incrementalSin * sinDelta;
			XMVECTOR newSin = incrementalCos * sinDelta + incrementalSin * cosDelta;
			incrementalCos = newCos;
			incrementalSin = newSin;
		}
		verts[c_ringSegments] = verts[0];

		batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, c_ringSegments + 1);
	}


	void XM_CALLCONV DrawRay(PrimitiveBatch<VertexPositionColor>* batch,
		FXMVECTOR origin,
		FXMVECTOR direction,
		bool normalize,
		FXMVECTOR color)
	{
		VertexPositionColor verts[3];
		XMStoreFloat3(&verts[0].position, origin);

		XMVECTOR normDirection = XMVector3Normalize(direction);
		XMVECTOR rayDirection = (normalize) ? normDirection : direction;

		XMVECTOR perpVector = XMVector3Cross(normDirection, g_XMIdentityR1);

		if (XMVector3Equal(XMVector3LengthSq(perpVector), g_XMZero))
		{
			perpVector = XMVector3Cross(normDirection, g_XMIdentityR2);
		}
		perpVector = XMVector3Normalize(perpVector);

		XMStoreFloat3(&verts[1].position, XMVectorAdd(rayDirection, origin));
		perpVector = XMVectorScale(perpVector, 0.0625f);
		normDirection = XMVectorScale(normDirection, -0.25f);
		rayDirection = XMVectorAdd(perpVector, rayDirection);
		rayDirection = XMVectorAdd(normDirection, rayDirection);
		XMStoreFloat3(&verts[2].position, XMVectorAdd(rayDirection, origin));

		XMStoreFloat4(&verts[0].color, color);
		XMStoreFloat4(&verts[1].color, color);
		XMStoreFloat4(&verts[2].color, color);

		batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 2);
	}


	void XM_CALLCONV DrawTriangle(PrimitiveBatch<VertexPositionColor>* batch,
		FXMVECTOR pointA,
		FXMVECTOR pointB,
		FXMVECTOR pointC,
		GXMVECTOR color)
	{
		VertexPositionColor verts[4];
		XMStoreFloat3(&verts[0].position, pointA);
		XMStoreFloat3(&verts[1].position, pointB);
		XMStoreFloat3(&verts[2].position, pointC);
		XMStoreFloat3(&verts[3].position, pointA);

		XMStoreFloat4(&verts[0].color, color);
		XMStoreFloat4(&verts[1].color, color);
		XMStoreFloat4(&verts[2].color, color);
		XMStoreFloat4(&verts[3].color, color);

		batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 4);
	}


	void XM_CALLCONV DrawQuad(PrimitiveBatch<VertexPositionColor>* batch,
		FXMVECTOR pointA,
		FXMVECTOR pointB,
		FXMVECTOR pointC,
		GXMVECTOR pointD,
		HXMVECTOR color)
	{
		VertexPositionColor verts[5];
		XMStoreFloat3(&verts[0].position, pointA);
		XMStoreFloat3(&verts[1].position, pointB);
		XMStoreFloat3(&verts[2].position, pointC);
		XMStoreFloat3(&verts[3].position, pointD);
		XMStoreFloat3(&verts[4].position, pointA);

		XMStoreFloat4(&verts[0].color, color);
		XMStoreFloat4(&verts[1].color, color);
		XMStoreFloat4(&verts[2].color, color);
		XMStoreFloat4(&verts[3].color, color);
		XMStoreFloat4(&verts[4].color, color);

		batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 5);
	}

}