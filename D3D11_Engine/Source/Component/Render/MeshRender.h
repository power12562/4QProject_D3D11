#pragma once
#include <Component/Base/RenderComponent.h>
#include <D3DCore/D3DConstBuffer.h>
#include <D3DCore/D3DTexture2D.h>
#include <D3DCore/D3DSamplerState.h>
#include <string>

class MeshRender : public RenderComponent
{
	inline static std::list<MeshRender*> instanceList;
	std::list<MeshRender*>::iterator myIter;
public:
	static void ReloadShaderAll();

public:
	MeshRender();
	virtual ~MeshRender() override;
public:
	virtual void Start() = 0;
protected:
	virtual void FixedUpdate() = 0;
	virtual void Update() = 0;
	virtual void LateUpdate() = 0;
	virtual void Render() = 0;

public:
	virtual void CreateMesh() = 0;
protected:
	DRAW_INDEX_RESOURCE meshResource;

public:
	void SetMeshResource(const wchar_t* path);
	int MeshID = -1;
public:
	D3DConstBuffer constBuffer;
	D3DTexture2D textures;
	D3DSamplerState samplerState;

public:
	/*메시 머터리얼의 BaseColor 기억용. CopyFBX에서 해당 컬러를 복사해갑니다.*/
	DirectX::SimpleMath::Color baseColor{1,1,1,1};

public:
	//RENDER_FLAG
	int RenderFlags;

public:
	bool IsVSShader() { return pInputLayout && pVertexShader; }
	bool IsPSShader() { return !!pPixelShader; }

public:
	void CopyShader(MeshRender& rhs);
public:
	void SetVertexShader(const wchar_t* path);
	void SetPixelShader(const wchar_t* path);
public:
	void ResetVertexShader();
	void ResetPixelShader();

	inline const std::wstring& GetVertexShaderPath() { return vertexShaderPath; }
	inline const std::wstring& GetPixelShaderPath() { return pixelShaderPath; }
private:
	std::wstring vertexShaderPath;
	ID3D11InputLayout* pInputLayout = nullptr;
	ID3D11VertexShader* pVertexShader = nullptr;
	std::wstring pixelShaderPath;
	ID3D11PixelShader* pPixelShader = nullptr;

public:
	void SetFillMode(D3D11_FILL_MODE mode);
	void SetCullMode(D3D11_CULL_MODE mode);
	void SetRRState(D3D11_RASTERIZER_DESC& desc);
	void ResetRRState();

private:
	D3D11_RASTERIZER_DESC currRRdesc;
	ID3D11RasterizerState* pRRState = nullptr;
public:
	RENDERER_DRAW_DESC GetRendererDesc();
};