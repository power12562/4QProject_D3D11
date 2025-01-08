#include "StartApp.h"
#include <StartScene.h>

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    StartApp app;
    app.Initialize(hInstance);
    sceneManager.LoadScene<StartScene>();
    app.Run();
    app.Uninitialize();

    return 0;
}