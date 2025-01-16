

#include "RendererTestApp.h"
#include <dxgi1_6.h>
#include <filesystem>
#pragma comment(lib, "dxgi")
#include <framework.h>
#include <ranges>


RendererTestApp::RendererTestApp()
{
    this->windowName = L"New Renderer Test";
    this->SetBorderlessWindowed();
    //this->SetOptimalScreenSize();
}

RendererTestApp::~RendererTestApp()
{


}

