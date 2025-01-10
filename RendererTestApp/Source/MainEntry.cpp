#include "RendererTestApp.h"
#include <RendererTestScene.h>

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    RendererTestApp app;
    app.Initialize(hInstance);
    app.Run();
    app.RendererTestApp::Uninitialize();

    return 0;
}