#include "D3DRenderer.h"
#include <Material\BlingPhongMaterial.h>
#include <dxgi1_4.h>
#include <cassert>
#include <Psapi.h>
#include <framework.h>
#include <Math/Mathf.h>
#include <format>
#include <Component/Render/MeshRender.h>

D3DRenderer& d3dRenderer = D3DRenderer::GetInstance();

using namespace Utility;

namespace cbuffer
{
    cb_Camera camera;
    cb_Transform transform;
    cb_ShadowMap ShadowMap;
}

D3DRenderer::D3DRenderer()
{
    backgroundColor = { 0, 0, 0, 1 };

    pDevice = nullptr;
    pDeviceContext = nullptr;
    pSwapChain = nullptr;
    
    pDepthStencilView = nullptr;
    pGbufferDSV = nullptr;
    pDefaultDepthStencilState = nullptr;

    pDefaultBlendState = nullptr;
    pDefaultRRState = nullptr;

    pShadowInputLayout = nullptr;
    pShadowVertexShader = nullptr;
    pShadowSkinningInputLayout = nullptr;
    pShadowSkinningVertexShader = nullptr;
}

D3DRenderer::~D3DRenderer()
{
   
}

void D3DRenderer::Init()
{
    using namespace Utility;
    try
    {
        //dxgi 디바이스
        CheckHRESULT(CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)&pDXGIFactory));

        //모든 그래픽 카드 및 모니터 조사. 
        size_t i = 0;
        IDXGIAdapter* pAdapter = nullptr;
        while (pDXGIFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND) //그래픽 카드 정보)
        {
            CheckHRESULT(pAdapter->QueryInterface(__uuidof(IDXGIAdapter3), reinterpret_cast<void**>(&pAdapter)));
            DXGIAdapters.push_back(reinterpret_cast<IDXGIAdapter3*>(pAdapter));
            DXGIOutputs.resize(i + 1);
            size_t j = 0;
            IDXGIOutput* pOutput = nullptr;
            while (pAdapter->EnumOutputs(j, &pOutput) != DXGI_ERROR_NOT_FOUND)
            {
                CheckHRESULT(pOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&pOutput)));
                DXGIOutputs[i].push_back(reinterpret_cast<IDXGIOutput1*>(pOutput));
                ++j;
            }
            ++i;
        }

        DXGI_ADAPTER_DESC adapterDesc;      
        for (auto& Adapters : DXGIAdapters)
        {
            Adapters->GetDesc(&adapterDesc);
            Debug_wprintf(L"%lu : %s\n", adapterDesc.DeviceId, adapterDesc.Description); //글카 이름
        }

        DXGI_OUTPUT_DESC outputDesc;
        DXGIOutputs[0][0]->GetDesc(&outputDesc); //메인 모니터
        if (outputDesc.AttachedToDesktop)
        {
            Debug_wprintf(L"Moniter 1 : %s\n", outputDesc.DeviceName);
            Debug_wprintf(L"Rotation : %s\n", GetDisplayRotationToCWStr(outputDesc.Rotation));
        }

        DXGI_SWAP_CHAIN_DESC swapDesc{}; //스왑체인 속성 구조체  
        swapDesc.BufferCount = 2; //버퍼 개수
        swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //버퍼 사용 방식 지정
        swapDesc.OutputWindow = WinGameApp::GetHWND(); //핸들 윈도우
        swapDesc.Windowed = true; //창모드 유무
        swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //출력 포멧 지정.
        swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; //플립 모드 사용.   
        swapDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //전체 화면 변환시 해상도 및 모니터 설정 자동 변경 플래그
        swapChainWindowed = swapDesc.Windowed;

        //버퍼 사이즈 지정
        SIZE clientSize = WinGameApp::GetClientSize();
        swapDesc.BufferDesc.Width = clientSize.cx;
        swapDesc.BufferDesc.Height = clientSize.cy;

        //0/0은 자동 설정임. 그리고 창모드는 어차피 적용 안됨...
		swapDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapDesc.BufferDesc.RefreshRate.Denominator = 0;

        //샘플링 설정 *(MSAA)
        swapDesc.SampleDesc.Count = 1;
        swapDesc.SampleDesc.Quality = 0;

        UINT creationFlags = 0;
#ifdef _DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        //디바이스, 스왑체인, 디바이스 컨텍스트 생성
        IDXGISwapChain* swapChain = nullptr;
        CheckHRESULT(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, NULL, NULL,
            D3D11_SDK_VERSION, &swapDesc, &swapChain, &pDevice, NULL, &pDeviceContext));
  
        CheckHRESULT(swapChain->QueryInterface(IID_PPV_ARGS(&pSwapChain)));
        SafeRelease(swapChain);

        //렌더타겟뷰 생성. (백퍼버 이용)
        ID3D11Texture2D* pBackBufferTexture = nullptr; //백버퍼
        CheckHRESULT(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture)); //스왑체인 백버퍼를 가져온다.
        if(pBackBufferTexture)
            CheckHRESULT(pDevice->CreateRenderTargetView(pBackBufferTexture, nullptr, &pRenderTargetViewArray[0])); //백퍼퍼를 참조하는 뷰 생성(참조 카운트 증가.)
        SafeRelease(pBackBufferTexture);

        //디버그 드로우용 객체 초기화
        pPrimitiveBatch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(pDeviceContext);
        pBasicEffect = std::make_unique<BasicEffect>(pDevice);

        //G버퍼 생성
        CreateGbufferRTV();

        //레스터화 기본 규칙
        D3D11_RASTERIZER_DESC rasterDesc;
        ZeroMemory(&rasterDesc, sizeof(rasterDesc));
        rasterDesc.FillMode = D3D11_FILL_SOLID;
        rasterDesc.CullMode = D3D11_CULL_BACK;  // 뒷면 컬링
        rasterDesc.FrontCounterClockwise = false; // 기본값
        SetRRState(rasterDesc);

        // Create the depth stencil view
        CreateMainDSV();
        CreateGbufferDSVnSRV();

        //Set alpha blend
        D3D11_BLEND_DESC blendDesc{};
        blendDesc.AlphaToCoverageEnable = FALSE; // Alpha To Coverage 비활성화
        blendDesc.IndependentBlendEnable = FALSE; // 독립 블렌드 비활성화
        blendDesc.RenderTarget[0].BlendEnable = TRUE; // 블렌딩 활성화
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // 소스 블렌드 (소스 알파 사용)
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 대상 블렌드 (1 - 소스 알파)
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // 블렌드 연산
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; // 알파 소스 블렌드
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO; // 알파 대상 블렌드
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; // 알파 블렌드 연산
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // 모든 색상 채널 쓰기 허용
        CheckHRESULT(pDevice->CreateBlendState(&blendDesc, &pDefaultBlendState));

        SetDefaultOMState();

        //뷰포트 설정
        {
            D3D11_VIEWPORT viewport = {};
            viewport.TopLeftX = 0;
            viewport.TopLeftY = 0;
            viewport.Width = (float)clientSize.cx;
            viewport.Height = (float)clientSize.cy;
            viewport.MinDepth = 0.0f;
            viewport.MaxDepth = 1.0f;     
            ViewPortsVec.push_back(viewport);
        }
   
        //ShadowMap
        CreateShadowMapResource();

        //Deferred용
        CreateDeferredResource();

        //상수 버퍼
        D3DConstBuffer::CreateStaticCbuffer();
    }
    catch (const std::exception& ex)
    {
        const char* er = ex.what();
        Debug_printf("%s\n", er);
        assert(!"D3D Init Error");
        return;
    }
    return;
}


void D3DRenderer::Uninit()
{
    //디버그용 개체
    pPrimitiveBatch.reset();
    pBasicEffect.reset();

    //dxgi 개체
    for (auto& outputs : DXGIOutputs)
    {
        SafeReleaseArray(outputs);
    }
	SafeReleaseArray(DXGIAdapters);
    SafeRelease(pDXGIFactory);
     
    //dxd11 개체
    D3DConstBuffer::ReleaseStaticCbuffer();
	SafeRelease(pPBRDirectionalLightPS);
    SafeRelease(pDeferredVertexShader);
    SafeRelease(pDeferredInputLayout);
    SafeRelease(deferredDrawData.pIndexBuffer);
    SafeRelease(deferredDrawData.pVertexBuffer);
    deferredSamplers.reset();
    D3DSamplerState::ClearSamplerResources();

    SafeRelease(pShadowInputLayout);
    SafeRelease(pShadowVertexShader);
    SafeRelease(pShadowSkinningInputLayout);
    SafeRelease(pShadowSkinningVertexShader);
    SafeReleaseArray(pShadowMapSRV);
    SafeReleaseArray(pShadowMapDSV);
    SafeReleaseArray(pShadowMap);

    SafeRelease(pDefaultRRState);
    SafeRelease(pDefaultBlendState);

    SafeReleaseArray(pRenderTargetViewArray);

    SafeReleaseArray(pGbufferSRV);
    SafeRelease(pGbufferDSV);

    SafeRelease(pDefaultDepthStencilState);

    SafeRelease(pDepthStencilView);
    {
        pSwapChain->SetFullscreenState(FALSE, NULL); //스왑체인 해제 전에는 창모드로 전환
        SafeRelease(pSwapChain);
        pDeviceContext->ClearState();   //상태 정리
        pDeviceContext->Flush();        //GPU 명령 대기
    }  
    SafeRelease(pDeviceContext);
    textureManager.ReleaseDefaultTexture();

#ifdef _DEBUG
    if constexpr (false)
    {
        ID3D11Debug* debug = nullptr;
        pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug));
        if (debug)
        {
            debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL); // 상세 정보 출력
            debug->Release();
        }
    }
#endif // _DEBUG

    ULONG refcount = SafeRelease(pDevice);
    if (refcount > 0)
    {
        __debugbreak(); //Device refcounter err.
    }
}

void D3DRenderer::reserveRenderQueue(size_t size)
{
    opaquerenderOueue.reserve(size);
    forwardrenderOueue.reserve(size);
    alphaRenderQueue.reserve(size);
}

void D3DRenderer::SetDefaultOMState()
{
    pDeviceContext->OMSetBlendState(pDefaultBlendState, nullptr, 0xFFFFFFFF);
    pDeviceContext->OMSetDepthStencilState(pDefaultDepthStencilState, 0);
}

void D3DRenderer::SetRRState(D3D11_RASTERIZER_DESC& defaultDesc)
{
    Utility::SafeRelease(pDefaultRRState);
    CreateRRState(defaultDesc, &pDefaultRRState);
}

void D3DRenderer::CreateRRState(D3D11_RASTERIZER_DESC& RASTERIZER_DESC, ID3D11RasterizerState** rasterState)
{
    Utility::CheckHRESULT(pDevice->CreateRasterizerState(&RASTERIZER_DESC, rasterState));
}

namespace BytesHelp
{
    constexpr UINT64 toMB = 1024 * 1024;  // 메가바이트 단위로 변환용
}

USAGE_VRAM_INFO D3DRenderer::GetLocalVramUsage()
{
    DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
    DXGIAdapters[0]->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);
    USAGE_VRAM_INFO info{};
    info.Budget = videoMemoryInfo.Budget / BytesHelp::toMB;
    info.Usage = videoMemoryInfo.CurrentUsage / BytesHelp::toMB;
    return info;
}

USAGE_VRAM_INFO D3DRenderer::GetNonLocalVramUsage()
{
    DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
    DXGIAdapters[0]->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &videoMemoryInfo);

    USAGE_VRAM_INFO info{};
    info.Budget = videoMemoryInfo.Budget / BytesHelp::toMB;
    info.Usage = videoMemoryInfo.CurrentUsage / BytesHelp::toMB;

    return info;
}

SYSTEM_VRAM_INFO D3DRenderer::GetSystemVramInfo()
{
    DXGI_ADAPTER_DESC desc;
    DXGIAdapters[0]->GetDesc(&desc);

    SYSTEM_VRAM_INFO info{};
    info.DedicatedVideoMemory = desc.DedicatedVideoMemory / BytesHelp::toMB;
    info.SharedSystemMemory = desc.SharedSystemMemory / BytesHelp::toMB;

    return info;
}

SYSTEM_MEMORY_INFO D3DRenderer::GetSystemMemoryInfo()
{
    HANDLE hProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX pmc{};
    pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS_EX);
    GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));          

    SYSTEM_MEMORY_INFO info{}; 
    info.PagefileUsage = pmc.PagefileUsage / BytesHelp::toMB;
    info.PrivateUsage = pmc.PrivateUsage / BytesHelp::toMB;
    info.WorkingSetSize = pmc.WorkingSetSize / BytesHelp::toMB;     

    return info;
}

void D3DRenderer::SetRTVdefault()
{
    pDeviceContext->OMSetRenderTargets(1, pRenderTargetViewArray, pGbufferDSV);
}

void D3DRenderer::ClearRTV()
{
    //clear RTV
    pDeviceContext->ClearRenderTargetView(pRenderTargetViewArray[0], backgroundColor);
    for (UINT i = 1; i < 1 + GbufferCount; i++)
    {
        constexpr float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        pDeviceContext->ClearRenderTargetView(pRenderTargetViewArray[i], clearColor);  // 화면 초기화 
    }
}

void D3DRenderer::BegineDraw()
{   
    //Set camera cb
    Camera* mainCam = Camera::GetMainCamera();
    mainCam->transform.PushUpdateList();
    Transform::UpdateMatrix();
    if (!DebugSetting.LockCameraFrustum)
    {
        cullingIVM  = mainCam->GetIVM();
        cullingView   = mainCam->GetVM();
        cullingProjection = mainCam->GetPM();
    }
    cbuffer::camera.MainCamPos = mainCam->transform.position;
    cbuffer::camera.View = XMMatrixTranspose(mainCam->GetVM());
    cbuffer::camera.Projection = XMMatrixTranspose(mainCam->GetPM());
    cbuffer::camera.IVM = XMMatrixTranspose(mainCam->GetIVM());
    cbuffer::camera.IPM = XMMatrixTranspose(mainCam->GetIPM());
    D3DConstBuffer::UpdateStaticCbuffer(cbuffer::camera);

    //Set lights view
    {
        using namespace DirectionalLight;
        using namespace Utility;

        // 카메라 절두체의 중심 계산
        Vector3 frustumCenter = mainCam->transform.position +
            mainCam->transform.Forward * (mainCam->Near + mainCam->Far) * 0.5f;

        constexpr float lightNear = 1.f;
        float lightFar = mainCam->Far * 1.35f;
        float camHalfFar = mainCam->Far * 0.5f;
        for (int i = 0; i < DirectionalLights->LightsCount; i++)
        {
            Vector3 lightDir; 
            DirectionalLights->Lights[i].LightDir.Normalize(lightDir);
 
            //Light View 생성
            XMVECTOR lightPos = XMVectorSubtract(XMLoadFloat3(&frustumCenter), XMLoadFloat3(&lightDir) * camHalfFar);
			bool isNan = std::abs(lightDir.x) < Mathf::Epsilon && std::abs(lightDir.z) < Mathf::Epsilon;
			if (lightDir.Length() < Mathf::Epsilon)
			{
				lightDir = Vector3(Mathf::Epsilon, Mathf::Epsilon, Mathf::Epsilon);
			}
            shadowViews[i] = XMMatrixLookToLH(lightPos, lightDir, isNan ? Vector3::Right : Vector3::Up);

            //투영 행렬 위치
            XMFLOAT3 cameraCorners[8];
            CalculateBoundsFrustumCorners(cullingView, cullingProjection, cameraCorners);
            shadowProjections[i] = CreateOrthographicProjection(CalculateBoundsLightSpace(shadowViews[i], cameraCorners), lightNear, lightFar);
        }
    }
}

void D3DRenderer::DrawIndex(RENDERER_DRAW_DESC& darwDesc)
{
    using namespace DirectX::DX11;
    Camera* mainCam = Camera::GetMainCamera();
    if (mainCam)
    {      
        GameObject* obj = &darwDesc.pTransform->gameObject;
        obj->isCulling = CheckFrustumCulling(obj);
        if (obj->isCulling)
        {
            if (darwDesc.flags & RENDER_FALG::RENDER_ALPHA)
                alphaRenderQueue.emplace_back(darwDesc);
            else if(darwDesc.flags & RENDER_FALG::RENDER_FORWARD)
                forwardrenderOueue.emplace_back(darwDesc);
            else
                opaquerenderOueue.emplace_back(darwDesc);
        }
    }
}

static const Transform* prevTransform = nullptr; //마지막으로 참조한 Trnasform
void D3DRenderer::EndDraw()
{
    D3DConstBuffer::SetStaticCbuffer();

    //sort
    auto textureSort = [](RENDERER_DRAW_DESC a, RENDERER_DRAW_DESC b) {            
            uintptr_t textureA = reinterpret_cast<uintptr_t>((*a.pD3DTexture2D)[0]);
            uintptr_t textureB = reinterpret_cast<uintptr_t>((*b.pD3DTexture2D)[0]);
            return textureA < textureB;    
    };
    auto MeshSort = [](RENDERER_DRAW_DESC a, RENDERER_DRAW_DESC b)
        {
            uintptr_t meshA = reinterpret_cast<uintptr_t>(a.pVertexIndex);
            uintptr_t meshB = reinterpret_cast<uintptr_t>(b.pVertexIndex);
            return  meshA < meshB;
        };
    std::sort(opaquerenderOueue.begin(), opaquerenderOueue.end(), MeshSort);
    std::sort(forwardrenderOueue.begin(), forwardrenderOueue.end(), MeshSort);
    std::sort(alphaRenderQueue.begin(), alphaRenderQueue.end(), MeshSort);

    //Shadow Map Pass
    {
        for (int i = 0; i < DirectionalLight::DirectionalLights->LightsCount; i++)
        {
            pDeviceContext->ClearDepthStencilView(pShadowMapDSV[i], D3D11_CLEAR_DEPTH, 1.0f, 0);
        }

        ID3D11ShaderResourceView* nullSRV = nullptr;
        for (int i = 0; i < DirectionalLight::DirectionalLights->LightsCount; i++)
        {
            pDeviceContext->PSSetShaderResources(SHADOW_SRV0 + i, 1, &nullSRV); //SRV 바인딩 해제
        }   
        pDeviceContext->RSSetViewports(1, &shadowViewPort);            //뷰포트 설정
        pDeviceContext->PSSetShader(nullptr, nullptr, 0);              //픽셀 셰이더 해제

        for (int i = 0; i < DirectionalLight::DirectionalLights->LightsCount; i++)
        {
            //set constbuffer
            cbuffer::ShadowMap.ShadowViews[0] = XMMatrixTranspose(shadowViews[i]);
            cbuffer::ShadowMap.ShadowProjections[0] = XMMatrixTranspose(shadowProjections[i]);
            D3DConstBuffer::UpdateStaticCbuffer(cbuffer::ShadowMap);           

            pDeviceContext->OMSetRenderTargets(0, nullptr, pShadowMapDSV[i]); 
            prevTransform = nullptr;
            for (auto& item : opaquerenderOueue)
            {
                RenderShadowMap(item);
            }
            prevTransform = nullptr;
            for (auto& item : forwardrenderOueue)
            {          
                RenderShadowMap(item);
            }
            prevTransform = nullptr;
            for (auto& item : alphaRenderQueue)
            {
                RenderShadowMap(item);
            }
        }
    }

    //Deferred stage
    pDeviceContext->ClearDepthStencilView(pGbufferDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
    pDeviceContext->OMSetRenderTargets(GbufferCount, &pRenderTargetViewArray[1], pGbufferDSV);
    pDeviceContext->RSSetViewports((UINT)ViewPortsVec.size(), ViewPortsVec.data());
    if(!opaquerenderOueue.empty())
    {
        //Gbuffer Pass (Deferred 1)
        {
			prevTransform = nullptr;
			for (auto& item : opaquerenderOueue)
			{
				RenderSceneGbuffer(item);
			}
        }
        //Lights Pass (Deferred 2)       
        {
            //viewPort Transform
            D3D11_VIEWPORT& viewPort = ViewPortsVec.front();
            SIZE size = D3D11_GameApp::GetClientSize();
            float screenWidth = (float)size.cx;
            float screenHeight = (float)size.cy;
            float scaleX = screenWidth / viewPort.Width;
            float scaleY = screenHeight / viewPort.Height;
            float offsetX = -viewPort.TopLeftX / screenWidth;
            float offsetY = -viewPort.TopLeftY / screenHeight;
            cbuffer::transform.World._11 = scaleX;
            cbuffer::transform.World._12 = scaleY;
            cbuffer::transform.World._13 = offsetX;
            cbuffer::transform.World._14 = offsetY;
            D3DConstBuffer::UpdateStaticCbuffer(cbuffer::transform);
          
            ID3D11RenderTargetView* nullRTV[GbufferCount]{ nullptr, };
            pDeviceContext->OMSetRenderTargets(GbufferCount, nullRTV, nullptr);
            pDeviceContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
            pDeviceContext->OMSetRenderTargets(1, pRenderTargetViewArray, nullptr);
            const D3D11_VIEWPORT& ViewPort = GetClientSizeViewport();
            pDeviceContext->RSSetViewports(1, &ViewPort);
            pDeviceContext->PSSetShaderResources(0, GbufferCount + 1, pGbufferSRV);
            pDeviceContext->PSSetShaderResources(SHADOW_SRV0, cb_PBRDirectionalLight::MAX_LIGHT_COUNT, pShadowMapSRV); //섀도우맵
            pDeviceContext->PSSetSamplers(0, deferredSamplers->size(), deferredSamplers->data()); //샘플러
            for (int i = 0; i < DirectionalLight::DirectionalLights->LightsCount; i++)
            {
                cbuffer::ShadowMap.ShadowViews[i] = XMMatrixTranspose(shadowViews[i]);
                cbuffer::ShadowMap.ShadowProjections[i] = XMMatrixTranspose(shadowProjections[i]);
                D3DConstBuffer::UpdateStaticCbuffer(cbuffer::ShadowMap);
            }
            RenderSceneLight();
        }        
    }

    //sky box draw
    {
        ID3D11ShaderResourceView* nullSRV[GbufferCount + 1]{ nullptr, };
        pDeviceContext->PSSetShaderResources(0, GbufferCount + 1, nullSRV);
        pDeviceContext->OMSetRenderTargets(1, pRenderTargetViewArray, pGbufferDSV);
        if (SkyBoxRender* mainSkybox = SkyBoxRender::GetMainSkyBox())
        {         
            RenderSkyBox(mainSkybox);
        }
        else
        {
            for (int i = E_TEXTURE::Diffuse_IBL; i < E_TEXTURE::BRDF_LUT; ++i)
            {
                ID3D11ShaderResourceView* srv = textureManager.GetDefaultTexture(E_TEXTURE_DEFAULT::CUBE_ZERO);
                pDeviceContext->PSSetShaderResources(i, 1, &srv);
            }
            ID3D11ShaderResourceView* srv = textureManager.GetDefaultTexture(E_TEXTURE_DEFAULT::ZERO);
            pDeviceContext->PSSetShaderResources(E_TEXTURE::BRDF_LUT, 1, &srv);
        }
    }

    //opaque pass (Forward)
    {
        pDeviceContext->RSSetViewports((UINT)ViewPortsVec.size(), ViewPortsVec.data());
        prevTransform = nullptr;
        for (auto& item : forwardrenderOueue)
        {
            RenderSceneForward(item);
        }
    }
    //alpha pass (Forward)
	{           
        prevTransform = nullptr;
		for (auto& item : alphaRenderQueue)
		{
			RenderSceneForward(item);
		}
	}

    //debug
    DrawDebug();

    //clear renderQueue
    DrawCallCount = forwardrenderOueue.size() + opaquerenderOueue.size() + alphaRenderQueue.size();
    opaquerenderOueue.clear();
    forwardrenderOueue.clear();
    alphaRenderQueue.clear();

    //clear Update List
    Transform::ClearUpdateList();
}

void D3DRenderer::Present()
{   
	pSwapChain->Present(Setting.UseVSync ? 1 : 0, 0);
}

D3D11_VIEWPORT D3DRenderer::GetClientSizeViewport()
{
    SIZE size = D3D11_GameApp::GetClientSize();
    D3D11_VIEWPORT viewPort{};
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.Width = (float)size.cx;
    viewPort.Height = (float)size.cy;
    viewPort.MaxDepth = 1.f;
    viewPort.MinDepth = 0.f;
    return viewPort;
}

void D3DRenderer::CreateShadowMapResource()
{
    SafeRelease(pShadowInputLayout);
    SafeRelease(pShadowVertexShader);
    SafeRelease(pShadowSkinningInputLayout);
    SafeRelease(pShadowSkinningVertexShader);
    SafeReleaseArray(pShadowMapSRV);
    SafeReleaseArray(pShadowMapDSV);
    SafeReleaseArray(pShadowMap);

    //Shadow Map 생성
    {
        shadowViewPort.Width = (float)SHADOW_MAP_SIZE;
        shadowViewPort.Height = (float)SHADOW_MAP_SIZE;
        shadowViewPort.MinDepth = 0.0f;
        shadowViewPort.MaxDepth = 1.0f;

        D3D11_TEXTURE2D_DESC textureDesc{};
        textureDesc.Width = (UINT)shadowViewPort.Width;
        textureDesc.Height = (UINT)shadowViewPort.Height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;

        for (int i = 0; i < cb_PBRDirectionalLight::MAX_LIGHT_COUNT; i++)
        {
            CheckHRESULT(pDevice->CreateTexture2D(&textureDesc, NULL, &pShadowMap[i]));
            D3D_SET_OBJECT_NAME(pShadowMap[i], L"ShadowMap");

            if (pShadowMap[i])
            {
                D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
                descDSV.Format = DXGI_FORMAT_D32_FLOAT;
                descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                CheckHRESULT(pDevice->CreateDepthStencilView(pShadowMap[i], &descDSV, &pShadowMapDSV[i]));
                D3D_SET_OBJECT_NAME(pShadowMapDSV[i], L"ShadowMapDSV");

                D3D11_SHADER_RESOURCE_VIEW_DESC descSRV = {};
                descSRV.Format = DXGI_FORMAT_R32_FLOAT;
                descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                descSRV.Texture2D.MipLevels = 1;
                CheckHRESULT(pDevice->CreateShaderResourceView(pShadowMap[i], &descSRV, &pShadowMapSRV[i]));
                D3D_SET_OBJECT_NAME(pShadowMapSRV[i], L"ShadowMapSRV");
            }
        }

        {
            using namespace std::string_literals;
            std::wstring path = HLSLManager::EngineShaderPath + L"ShadowMapVS.hlsl"s;
            hlslManager.MakeShader(path.c_str(), &pShadowVertexShader, &pShadowInputLayout);
            path = HLSLManager::EngineShaderPath + L"ShadowMapSkinningVS.hlsl"s;
            hlslManager.MakeShader(path.c_str(), &pShadowSkinningVertexShader, &pShadowSkinningInputLayout);
        }
    }
}

void D3DRenderer::DrawDebug()
{
    pDeviceContext->OMSetRenderTargets(1, pRenderTargetViewArray, pGbufferDSV);
    pDeviceContext->RSSetViewports((UINT)ViewPortsVec.size(), ViewPortsVec.data());
    pPrimitiveBatch->Begin();  // PrimitiveBatch 시작

    Camera* mainCamera = Camera::GetMainCamera();
    pBasicEffect->SetView(mainCamera->GetVM());
    pBasicEffect->SetProjection(mainCamera->GetPM());
    ComPtr<ID3D11InputLayout> inputLayout;
    CheckHRESULT(DirectX::CreateInputLayoutFromEffect<VertexPositionColor>(pDevice, pBasicEffect.get(), inputLayout.ReleaseAndGetAddressOf()));
    pDeviceContext->IASetInputLayout(inputLayout.Get());
    pBasicEffect->SetColorAndAlpha(DebugSetting.DebugDrawColor);
    pBasicEffect->Apply(pDeviceContext);

    if (DebugSetting.DrawLightFrustum)
    {
        for (int i = 0; i < DirectionalLight::DirectionalLights->LightsCount; ++i)
        {
            Matrix shadowWorld = DirectX::XMMatrixInverse(nullptr, shadowViews[i]);
            DirectX::BoundingFrustum shadowFrustum(shadowProjections[i]);
            shadowFrustum.Transform(shadowFrustum, shadowWorld);
            DebugDraw::Draw(pPrimitiveBatch.get(), shadowFrustum);
        }
    }
    if (DebugSetting.DrawCameraFrustum)
    {      
        DirectX::BoundingFrustum cameraFrustum(cullingProjection);
        cameraFrustum.Transform(cameraFrustum, cullingIVM);
        DebugDraw::Draw(pPrimitiveBatch.get(), cameraFrustum);
    }
    if(DebugSetting.DrawObjectCullingBox)
    {
        for (auto& desc : opaquerenderOueue)
        {            
            DirectX::BoundingBox bounds = desc.pTransform->gameObject.GetBBToWorld();
            DebugDraw::Draw(pPrimitiveBatch.get(), bounds);
        }
        for (auto& desc : forwardrenderOueue)
        {
            DirectX::BoundingBox bounds = desc.pTransform->gameObject.GetBBToWorld();
            DebugDraw::Draw(pPrimitiveBatch.get(), bounds);
        }
        for (auto& desc : alphaRenderQueue)
        {
            DirectX::BoundingBox bounds = desc.pTransform->gameObject.GetBBToWorld();
            DebugDraw::Draw(pPrimitiveBatch.get(), bounds);
        }
    }
    else if(Scene::GuizmoSetting.UseImGuizmo && Scene::GuizmoSetting.SelectObject)
    {
        Transform* pTransform = &Scene::GuizmoSetting.SelectObject->transform;
        DirectX::BoundingOrientedBox bounds = pTransform->gameObject.GetOBBToWorld();
        DebugDraw::Draw(pPrimitiveBatch.get(), bounds);
    }

    for (auto& [Frustum, World] : debugFrustumVec)
    {
        DirectX::BoundingFrustum frustum(*Frustum);
        frustum.Transform(frustum, *World);
        DebugDraw::Draw(pPrimitiveBatch.get(), frustum);
    }
    pPrimitiveBatch->End();   // PrimitiveBatch 종료
}

void D3DRenderer::PushDebugFrustum(const Matrix* frustum, const Matrix* WM)
{
    debugFrustumVec.emplace_back(frustum, WM);
}

void D3DRenderer::PopDebugFrustum()
{
    if(!debugFrustumVec.empty())
        debugFrustumVec.pop_back();
}

bool D3DRenderer::CheckFrustumCulling(GameObject* obj) const
{
    if (DebugSetting.UseFrustumCulling)
    {
        BoundingBox BB;
        BB = obj->GetBBToWorld();
        BoundingFrustum cameraFrustum(cullingProjection);
        cameraFrustum.Transform(cameraFrustum, cullingIVM);
        return cameraFrustum.Intersects(BB);
    }
    else
    {
        return true;
    }
}

void D3DRenderer::RenderSkyBox(SkyBoxRender* skyBox)
{
    constexpr int index2SkyBox = E_TEXTURE::Diffuse_IBL - SkyBoxRender::Diffuse_IBL;
    RENDERER_DRAW_DESC drawDesc = skyBox->GetRendererDesc();

    //set IA
    DRAW_INDEX_DATA* data = drawDesc.pVertexIndex;
    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정점을 이어서 그릴 방식 설정.
    pDeviceContext->IASetVertexBuffers(0, 1, &data->pVertexBuffer, &data->vertexBufferStride, &data->vertexBufferOffset);
    pDeviceContext->IASetIndexBuffer(data->pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);	// INDEX값의 범위
    pDeviceContext->RSSetViewports((UINT)ViewPortsVec.size(), ViewPortsVec.data());
    pDeviceContext->IASetInputLayout(drawDesc.pInputLayout);
    pDeviceContext->VSSetShader(drawDesc.pVertexShader, nullptr, 0);
    pDeviceContext->PSSetShader(drawDesc.pPixelShader, nullptr, 0);
	pDeviceContext->RSSetState(pDefaultRRState);
    
    for (int i = 0; i < drawDesc.pSamperState->size(); i++)
    {
        ID3D11SamplerState* sampler = (*drawDesc.pSamperState)[i];
        pDeviceContext->PSSetSamplers(i, 1, &sampler);
    }
    //set texture
    for (int i = 0; i < drawDesc.pD3DTexture2D->size(); i++)
    {
        ID3D11ShaderResourceView* srv = (*drawDesc.pD3DTexture2D)[i];
        pDeviceContext->PSSetShaderResources(i, 1, &srv);
    }
    pDeviceContext->DrawIndexed(data->indicesCount, 0, 0);

    //SetIBLTexture
    for (int i = E_TEXTURE::Diffuse_IBL; i < E_TEXTURE::Null; ++i)
    {
        ID3D11ShaderResourceView* srv = skyBox->textures[i - index2SkyBox];
        if (srv)
            pDeviceContext->PSSetShaderResources(i, 1, &srv);
    }
    //reset DepthStencilState
    pDeviceContext->OMSetDepthStencilState(pDefaultDepthStencilState, 0);
}

void D3DRenderer::RenderShadowMap(RENDERER_DRAW_DESC& drawDesc)
{
    //set IA
    DRAW_INDEX_DATA* data = drawDesc.pVertexIndex;
    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정점을 이어서 그릴 방식 설정.
    pDeviceContext->IASetVertexBuffers(0, 1, &data->pVertexBuffer, &data->vertexBufferStride, &data->vertexBufferOffset);
    pDeviceContext->IASetIndexBuffer(data->pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);	// INDEX값의 범위

    //set const buffer
    if (prevTransform != drawDesc.pTransform)
    {
        const Matrix& WM = drawDesc.pTransform->GetWM();
        cbuffer::transform.World = XMMatrixTranspose(WM);
        D3DConstBuffer::UpdateStaticCbuffer(cbuffer::transform);
        prevTransform = drawDesc.pTransform;
    }
    drawDesc.pConstBuffer->SetConstBuffer();

	//Shadow Map Pass
	if (drawDesc.flags & RENDER_FALG::RENDER_SKINNING)
	{
		pDeviceContext->IASetInputLayout(pShadowSkinningInputLayout);
		pDeviceContext->VSSetShader(pShadowSkinningVertexShader, nullptr, 0);
	}
	else
	{
		pDeviceContext->IASetInputLayout(pShadowInputLayout);
		pDeviceContext->VSSetShader(pShadowVertexShader, nullptr, 0);
	}
	pDeviceContext->DrawIndexed(data->indicesCount, 0, 0);
}

void D3DRenderer::RenderSceneGbuffer(RENDERER_DRAW_DESC& drawDesc)
{
    //set IA
    DRAW_INDEX_DATA* data = drawDesc.pVertexIndex;
    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정점을 이어서 그릴 방식 설정.
    pDeviceContext->IASetVertexBuffers(0, 1, &data->pVertexBuffer, &data->vertexBufferStride, &data->vertexBufferOffset);
    pDeviceContext->IASetIndexBuffer(data->pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);	// INDEX값의 범위

    //set const buffer
    if (prevTransform != drawDesc.pTransform)
    {
        Matrix VP = Camera::GetMainCamera()->GetVM() * Camera::GetMainCamera()->GetPM();
        const Matrix& WM = drawDesc.pTransform->GetWM();

        cbuffer::transform.World = XMMatrixTranspose(WM);
        cbuffer::transform.WorldInverseTranspose = XMMatrixInverse(nullptr, WM);
        cbuffer::transform.WVP = XMMatrixTranspose(WM * VP);
        D3DConstBuffer::UpdateStaticCbuffer(cbuffer::transform);
        prevTransform = drawDesc.pTransform;
    }
    drawDesc.pConstBuffer->SetConstBuffer();
    
    //set Shader
    pDeviceContext->IASetInputLayout(drawDesc.pInputLayout);
    pDeviceContext->VSSetShader(drawDesc.pVertexShader, nullptr, 0);
    pDeviceContext->PSSetShader(drawDesc.pPixelShader, nullptr, 0);

    //Set RSState
    if (drawDesc.pRRState)
    {
        pDeviceContext->RSSetState(drawDesc.pRRState);
    }
    else
    {
        pDeviceContext->RSSetState(pDefaultRRState);
    }

	//set sampler
	pDeviceContext->PSSetSamplers(0, drawDesc.pSamperState->size(), drawDesc.pSamperState->data());

	//set texture
	for (int i = 0; i < E_TEXTURE::AmbientOcculusion; i++)
	{
		ID3D11ShaderResourceView* srv = (*drawDesc.pD3DTexture2D)[i];
		pDeviceContext->PSSetShaderResources(i, 1, &srv);
	}
	pDeviceContext->DrawIndexed(data->indicesCount, 0, 0);
}

void D3DRenderer::RenderSceneLight()
{
    //set IA
    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); 
    pDeviceContext->IASetVertexBuffers(0, 1, &deferredDrawData.pVertexBuffer, &deferredDrawData.vertexBufferStride, &deferredDrawData.vertexBufferOffset);
    pDeviceContext->IASetIndexBuffer(deferredDrawData.pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);	
    pDeviceContext->IASetInputLayout(pDeferredInputLayout);

    //set shader
    pDeviceContext->VSSetShader(pDeferredVertexShader, nullptr, 0);
    pDeviceContext->PSSetShader(pPBRDirectionalLightPS, nullptr, 0);
    pDeviceContext->DrawIndexed(deferredDrawData.indicesCount, 0, 0);
}

void D3DRenderer::RenderSceneForward(RENDERER_DRAW_DESC& drawDesc)
{
    //set IA
    DRAW_INDEX_DATA* data = drawDesc.pVertexIndex;
    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정점을 이어서 그릴 방식 설정.
    pDeviceContext->IASetVertexBuffers(0, 1, &data->pVertexBuffer, &data->vertexBufferStride, &data->vertexBufferOffset);
    pDeviceContext->IASetIndexBuffer(data->pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);	// INDEX값의 범위

    //set const buffer
    if (prevTransform != drawDesc.pTransform)
    {
        Matrix VP = Camera::GetMainCamera()->GetVM() * Camera::GetMainCamera()->GetPM();
        const Matrix& WM = drawDesc.pTransform->GetWM();

        cbuffer::transform.World = XMMatrixTranspose(WM);
        cbuffer::transform.WorldInverseTranspose = XMMatrixInverse(nullptr, WM);
        cbuffer::transform.WVP = XMMatrixTranspose(WM * VP);
        D3DConstBuffer::UpdateStaticCbuffer(cbuffer::transform);
        prevTransform = drawDesc.pTransform;
    }
    drawDesc.pConstBuffer->SetConstBuffer();

    //Render pass
    {
        pDeviceContext->IASetInputLayout(drawDesc.pInputLayout);
        pDeviceContext->VSSetShader(drawDesc.pVertexShader, nullptr, 0);
        pDeviceContext->PSSetShader(drawDesc.pPixelShader, nullptr, 0);

        if (drawDesc.pRRState)
        {
            pDeviceContext->RSSetState(drawDesc.pRRState);
        }
        else
        {
            pDeviceContext->RSSetState(pDefaultRRState);
        }

        //set sampler
        for (int i = 0; i < drawDesc.pSamperState->size(); i++)
        {
            ID3D11SamplerState* sampler = (*drawDesc.pSamperState)[i];
            pDeviceContext->PSSetSamplers(i, 1, &sampler);
        }
        //set texture
        for (int i = 0; i < drawDesc.pD3DTexture2D->size(); i++)
        {
            ID3D11ShaderResourceView* srv = (*drawDesc.pD3DTexture2D)[i];
            pDeviceContext->PSSetShaderResources(i, 1, &srv);
        }
        pDeviceContext->DrawIndexed(data->indicesCount, 0, 0);
    }
}

void D3DRenderer::CreateDeferredResource()
{
	SafeRelease(pPBRDirectionalLightPS);
	SafeRelease(pDeferredVertexShader);
    SafeRelease(pDeferredInputLayout);
    SafeRelease(deferredDrawData.pIndexBuffer);
    SafeRelease(deferredDrawData.pVertexBuffer);  
    deferredSamplers.reset();
    deferredSamplers = std::make_unique<D3DSamplerState>();
    deferredSamplers->resize(3);

    //linear sampler
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    deferredSamplers->SetSamplerState(0, sampDesc);

    //Shadow Sampler
    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    deferredSamplers->SetSamplerState(1, sampDesc);
    {
        using namespace std::string_literals;
        std::wstring path = HLSLManager::EngineShaderPath + L"DeferredVS.hlsl"s;
        hlslManager.MakeShader(path.c_str(), &pDeferredVertexShader, &pDeferredInputLayout);
        path = HLSLManager::EngineShaderPath + L"PBRDirectionalLightPS.hlsl"s;
        hlslManager.MakeShader(path.c_str(), &pPBRDirectionalLightPS);
    }

    struct Vertex {
        Vector4 position;  // NDC 좌표
        Vector2 uv;       // UV 좌표
    };
    Vertex vertices[] = {
    { XMFLOAT4(-1.0f,  1.0f, 0.0f, 1.f), XMFLOAT2(0.0f, 0.0f) }, // Top-Left
    { XMFLOAT4( 1.0f,  1.0f, 0.0f, 1.f), XMFLOAT2(1.0f, 0.0f) }, // Top-Right
    { XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.f), XMFLOAT2(0.0f, 1.0f) }, // Bottom-Left
    { XMFLOAT4( 1.0f, -1.0f, 0.0f, 1.f), XMFLOAT2(1.0f, 1.0f) }  // Bottom-Right
    };
    deferredDrawData.vertexBufferOffset = 0;
    deferredDrawData.vertexBufferStride = sizeof(Vertex);
    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;
    CheckHRESULT(d3dRenderer.GetDevice()->CreateBuffer(&bd, &vbData, &deferredDrawData.pVertexBuffer));

    unsigned int indices[] = {
    0, 1, 2, // 첫 번째 삼각형: Top-Left, Top-Right, Bottom-Left
    2, 1, 3  // 두 번째 삼각형: Bottom-Left, Top-Right, Bottom-Right
    };
    deferredDrawData.indicesCount = ARRAYSIZE(indices);
    bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(UINT) * ARRAYSIZE(indices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;
    CheckHRESULT(d3dRenderer.GetDevice()->CreateBuffer(&bd, &ibData, &deferredDrawData.pIndexBuffer));
}             

void D3DRenderer::CreateMainDSV()
{
    if (pSwapChain)
    {
        DXGI_SWAP_CHAIN_DESC1 modeDesc;
        CheckHRESULT(pSwapChain->GetDesc1(&modeDesc));

        //깊이 버퍼 생성
        SafeRelease(pDepthStencilView);
        D3D11_TEXTURE2D_DESC descDepth = {};
        descDepth.Width = modeDesc.Width;
        descDepth.Height = modeDesc.Height;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.SampleDesc.Count = 1;
        descDepth.SampleDesc.Quality = 0;
        descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;

        ID3D11Texture2D* textureDepthStencil = nullptr;
        CheckHRESULT(pDevice->CreateTexture2D(&descDepth, nullptr, &textureDepthStencil));

        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
        descDSV.Format = descDepth.Format;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;

        if (textureDepthStencil)
            CheckHRESULT(pDevice->CreateDepthStencilView(textureDepthStencil, &descDSV, &pDepthStencilView));
        SafeRelease(textureDepthStencil);
    }
}

void D3DRenderer::CreateGbufferRTV()
{
    DXGI_SWAP_CHAIN_DESC1 desc;
    pSwapChain->GetDesc1(&desc);

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = desc.Width;
    textureDesc.Height = desc.Height;
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;  // 렌더 타겟으로 사용
    textureDesc.CPUAccessFlags = 0;

    //GbufferCount에 해당하는 인덱스는 깊이 값 기록용
    for (int i = 1; i < 1 + GbufferCount; i++)
    {
        SafeRelease(pRenderTargetViewArray[i]);
        SafeRelease(pGbufferSRV[i - 1]);

        ID3D11Texture2D* newBuffer;
        CheckHRESULT(pDevice->CreateTexture2D(&textureDesc, nullptr, &newBuffer));
        D3D_SET_OBJECT_NAME(newBuffer, L"d3dRenderer");

        if (newBuffer)
        {
            CheckHRESULT(pDevice->CreateRenderTargetView(newBuffer, nullptr, &pRenderTargetViewArray[i])); //백퍼퍼를 참조하는 뷰 생성(참조 카운트 증가.)
            D3D_SET_OBJECT_NAME(pRenderTargetViewArray[i], L"GbufferRTV");

            CheckHRESULT(pDevice->CreateShaderResourceView(newBuffer, nullptr, &pGbufferSRV[i - 1]));
            D3D_SET_OBJECT_NAME(pGbufferSRV[i - 1], L"GbufferSRV");
        }          
        SafeRelease(newBuffer);
    }
}

void D3DRenderer::CreateGbufferDSVnSRV()
{
    if (pSwapChain)
    {
        DXGI_SWAP_CHAIN_DESC1 modeDesc;
        CheckHRESULT(pSwapChain->GetDesc1(&modeDesc));

        SafeRelease(pGbufferSRV[GbufferCount]); //이 자리는 DSV용
        SafeRelease(pGbufferDSV);

        D3D11_TEXTURE2D_DESC descDepth = {};
        descDepth.Width = modeDesc.Width;
        descDepth.Height = modeDesc.Height;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.SampleDesc.Count = 1;
        descDepth.SampleDesc.Quality = 0;
        descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;

        ID3D11Texture2D* textureDepthStencil = nullptr;
        CheckHRESULT(pDevice->CreateTexture2D(&descDepth, nullptr, &textureDepthStencil));
        D3D_SET_OBJECT_NAME(textureDepthStencil, L"Gbuffer Depth Buffer");
        if (textureDepthStencil)
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
            descDSV.Format = DXGI_FORMAT_D32_FLOAT;
            descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            CheckHRESULT(pDevice->CreateDepthStencilView(textureDepthStencil, &descDSV, &pGbufferDSV));
            D3D_SET_OBJECT_NAME(pGbufferDSV, L"GbufferDSV");

            D3D11_SHADER_RESOURCE_VIEW_DESC descSRV = {};
            descSRV.Format = DXGI_FORMAT_R32_FLOAT;
            descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            descSRV.Texture2D.MipLevels = 1;
            CheckHRESULT(pDevice->CreateShaderResourceView(textureDepthStencil, &descSRV, &pGbufferSRV[GbufferCount]));
            D3D_SET_OBJECT_NAME(pGbufferSRV[GbufferCount], L"pGbufferDepthSRV");
            SafeRelease(textureDepthStencil);
        }
    }
}

DXGI_MODE_DESC1 D3DRenderer::GetDisplayMode(int AdapterIndex, int OutputIndex)
{
    DXGI_MODE_DESC1 targetDesc{}; //기본 초기화 값 사용
    targetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_MODE_DESC1 outDesc{};
    Utility::CheckHRESULT(DXGIOutputs[AdapterIndex][OutputIndex]->FindClosestMatchingMode1(&targetDesc, &outDesc, nullptr));
    return outDesc;
}

DXGI_MODE_DESC1 D3DRenderer::GetClosestResolution(int AdapterIndex, int OutputIndex, SIZE resolution)
{
    DXGI_MODE_DESC1 targetDesc{};
    targetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    targetDesc.Width = resolution.cx;
    targetDesc.Height = resolution.cy;
    DXGI_MODE_DESC1 outDesc{};
    Utility::CheckHRESULT(DXGIOutputs[AdapterIndex][OutputIndex]->FindClosestMatchingMode1(&targetDesc, &outDesc, nullptr));
    return outDesc;
}

std::vector<DXGI_MODE_DESC1> D3DRenderer::GetDisplayModeList(int AdapterIndex, int OutputIndex)
{
    using namespace Utility;
    std::vector<DXGI_MODE_DESC1> descList(1);
    
    //출력 모드 리스트 가져오기 (주사율 포함)
    UINT numModes = 0;
    CheckHRESULT(DXGIOutputs[AdapterIndex][OutputIndex]->GetDisplayModeList1(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, nullptr));

    if (numModes > 1)
    {
        descList.resize(numModes);
        CheckHRESULT(DXGIOutputs[AdapterIndex][OutputIndex]->GetDisplayModeList1(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, descList.data()));
    }
    return descList;
}

void D3DRenderer::ToggleFullscreenMode()
{
    HWND hwnd = WinGameApp::GetHWND();
    SIZE clientSize = WinGameApp::GetClientSize();
    if (swapChainWindowed)
    {   
        DXGI_SWAP_CHAIN_DESC1 swapDesc{}; //스왑체인 속성 구조체  
        swapDesc.BufferCount = 2; //버퍼 개수
        swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //버퍼 사용 방식 지정
        swapDesc.Stereo = false; //스테레오 유무
        swapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  //출력 포멧 지정.
        swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;      //플립 모드.
        swapDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //전체 화면 변환시 해상도 및 모니터 설정 자동 변경 플래그
   
        //버퍼 크기 지정. 사용 가능한 가장 가까운 해상도 찾는다.
        DXGI_MODE_DESC1 decs = GetClosestResolution(0, 0, clientSize);
        swapDesc.Width = decs.Width;
        swapDesc.Height = decs.Height;

        //샘플링 설정 *(MSAA)
        swapDesc.SampleDesc.Count = 1;
        swapDesc.SampleDesc.Quality = 0;

        UINT creationFlags = 0;
#ifdef _DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        swapChainWindowed = !swapChainWindowed;
        ReCreateSwapChain(&swapDesc);
        WinGameApp::WinToScreenCenter(hwnd);
    }
    else
    {     
        DXGI_SWAP_CHAIN_DESC1 swapDesc{}; //스왑체인 속성 구조체  
        swapDesc.BufferCount = 2; //버퍼 개수
        swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //버퍼 사용 방식 지정
        swapDesc.Stereo = false; //스테레오 유무
        swapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  //출력 포멧 지정.
        swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;      //플립 모드.
        swapDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //전체 화면 변환시 해상도 및 모니터 설정 자동 변경 플래그

        //버퍼 사이즈 지정
        swapDesc.Width = clientSize.cx;
        swapDesc.Height = clientSize.cy;

        //샘플링 설정 *(MSAA)
        swapDesc.SampleDesc.Count = 1;
        swapDesc.SampleDesc.Quality = 0;

        UINT creationFlags = 0;
#ifdef _DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        swapChainWindowed = !swapChainWindowed;
        ReCreateSwapChain(&swapDesc);
        WinGameApp::WinToScreenCenter(hwnd);
    }
}

void D3DRenderer::ReCreateSwapChain(DXGI_SWAP_CHAIN_DESC1* swapChainDesc)
{
    if (pSwapChain)
    {    
        DXGI_SWAP_CHAIN_DESC1 pervDesc;
        CheckHRESULT(pSwapChain->GetDesc1(&pervDesc));
        float resolutionScaleX = (float)swapChainDesc->Width / (float)pervDesc.Width;
        float resolutionScaleY = (float)swapChainDesc->Height / (float)pervDesc.Height;

        pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr); //렌더타겟 바인딩 해제
        unsigned long ref = SafeRelease(pRenderTargetViewArray[0]); //RTV 리소스 제거
        CheckHRESULT(pSwapChain->SetFullscreenState(FALSE, NULL));
   
        ref = SafeRelease(pSwapChain);
        pDeviceContext->ClearState();   //상태 정리
        pDeviceContext->Flush();        //GPU 명령 대기

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc{};
        fullScreenDesc.Windowed = swapChainWindowed;
        CheckHRESULT(pDXGIFactory->CreateSwapChainForHwnd(pDevice, WinGameApp::GetHWND(), swapChainDesc, &fullScreenDesc, nullptr, &pSwapChain));
        CheckHRESULT(pSwapChain->SetFullscreenState(!swapChainWindowed, NULL));
        if (!swapChainWindowed) //전체화면 모드면.
        {
            SIZE resolution{ (LONG)swapChainDesc->Width, (LONG)swapChainDesc->Height };
            DXGI_MODE_DESC1 modeDesc = GetClosestResolution(0, 0, resolution);
            DEVMODE devMode{};
            devMode.dmSize = sizeof(DEVMODE);
            devMode.dmPelsWidth = resolution.cx;  // 새로 설정할 해상도 너비
            devMode.dmPelsHeight = resolution.cy; // 새로 설정할 해상도 높이
            devMode.dmBitsPerPel = GetColorBitDepth(modeDesc.Format);  
            devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
            LONG result = ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
            if (result != DISP_CHANGE_SUCCESSFUL) 
            {
                __debugbreak(); //해상도 변경 실패
            }
        }
   
        DXGI_MODE_DESC modeDesc{};
        modeDesc.Width = swapChainDesc->Width;
        modeDesc.Height = swapChainDesc->Height;
        modeDesc.Format = swapChainDesc->Format;
        CheckHRESULT(pSwapChain->ResizeTarget(&modeDesc));
        CheckHRESULT(pSwapChain->ResizeBuffers(2, swapChainDesc->Width, swapChainDesc->Height, swapChainDesc->Format, swapChainDesc->BufferUsage));

        //렌더타겟뷰 재 생성
        ID3D11Texture2D* pBackBufferTexture = nullptr; //백버퍼
        CheckHRESULT(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture)); //스왑체인 백버퍼를 가져온다.
        if (pBackBufferTexture)
            CheckHRESULT(pDevice->CreateRenderTargetView(pBackBufferTexture, NULL, &pRenderTargetViewArray[0])); //백퍼퍼를 참조하는 뷰 생성(참조 카운트 증가.)
        SafeRelease(pBackBufferTexture);      

        SetDefaultOMState();  
        //뷰포트 크기 재조정
        for (auto& viewport : ViewPortsVec)
        {
            viewport.Width = std::round(viewport.Width * resolutionScaleX);
            viewport.Height = std::round(viewport.Height * resolutionScaleY);
        }

        CreateMainDSV();
        CreateGbufferRTV();
        CreateGbufferDSVnSRV();
    }
}

const wchar_t* D3DRenderer::GetDisplayRotationToCWStr(DXGI_MODE_ROTATION rotation)
{
    switch (rotation)
    {
    case DXGI_MODE_ROTATION_UNSPECIFIED:
        return L"UNSPECIFIED";
    case DXGI_MODE_ROTATION_IDENTITY:
        return L"IDENTITY";
    case DXGI_MODE_ROTATION_ROTATE90:
        return L"ROTATE90";
    case DXGI_MODE_ROTATION_ROTATE180:
        return L"ROTATE180";
    case DXGI_MODE_ROTATION_ROTATE270:
        return L"ROTATE270";
    default:
        return L"UNKNOWN";
    }
}

DWORD D3DRenderer::GetColorBitDepth(DXGI_FORMAT format)
{
    switch (format) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        return 32; // 8-bit per channel, 4 channels (R, G, B, A)

    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        return 32; // 8-bit per channel, 4 channels (B, G, R, A)

    case DXGI_FORMAT_R16G16B16A16_FLOAT:
        return 64; // 16-bit per channel, 4 channels (R, G, B, A)

    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        return 128; // 32-bit per channel, 4 channels (R, G, B, A)

    case DXGI_FORMAT_R16G16B16A16_UNORM:
        return 64; // 16-bit per channel, 4 channels (R, G, B, A)

    case DXGI_FORMAT_R32G32_FLOAT:
        return 64; // 32-bit per channel, 2 channels (R, G)

    case DXGI_FORMAT_R32_FLOAT:
        return 32; // 32-bit per channel, 1 channel (R)

    case DXGI_FORMAT_R8G8B8A8_SNORM:
        return 32; // 8-bit per channel, 4 channels (R, G, B, A)

    case DXGI_FORMAT_R8_UNORM:
        return 8; // 8-bit per channel, 1 channel (R)

    case DXGI_FORMAT_R16_UNORM:
        return 16; // 16-bit per channel, 1 channel (R)

    case DXGI_FORMAT_R32_UINT:
        return 32; // 32-bit per channel, 1 channel (R)

    case DXGI_FORMAT_D32_FLOAT:
        return 32; // Depth buffer, 32-bit (single channel)

    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        return 32; // Depth 24-bit, Stencil 8-bit

    default:
        throw std::runtime_error("Unsupported format.");
    }
}
