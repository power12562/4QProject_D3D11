#include "EditerTestApp.h"
#include <EditerTestScene.h>

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    EditerTestApp app;
    app.Initialize(hInstance);
    sceneManager.LoadScene<EditerTestScene>();
    app.Run();
    app.Uninitialize();

    return 0;
}