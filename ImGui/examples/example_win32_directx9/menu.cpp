// dear imgui: standalone example application for DirectX 9
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include "imgui_internal.h"
#include <d3dx9.h>
#include "rage.h"
#include "aa.h"
#include "legit.h"
#include "players.h"
#include "weapons.h"
#include "grenades.h"
#include "world.h"
#include "view.h"
#include "main.h"
#include "inventory.h"
#include "scripts.h"
#include "configs.h"
#include "soufiw.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include "hashes.h"
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

ImFont* tolstiy = nullptr;
ImFont* tolstiy2 = nullptr;
ImFont* ne_takoi_tolstiy = nullptr;
ImFont* ne_takoi_tolstiy2 = nullptr;
IDirect3DTexture9* r = nullptr;
IDirect3DTexture9* a = nullptr;
IDirect3DTexture9* l = nullptr;
IDirect3DTexture9* p = nullptr;
IDirect3DTexture9* w = nullptr;
IDirect3DTexture9* g = nullptr;
IDirect3DTexture9* wo = nullptr;
IDirect3DTexture9* v = nullptr;
IDirect3DTexture9* m = nullptr;
IDirect3DTexture9* i = nullptr;
IDirect3DTexture9* s = nullptr;
IDirect3DTexture9* c = nullptr;
IDirect3DTexture9* soufiww = nullptr;
IDirect3DTexture9* logow = nullptr;

float animspeed;
bool customize;


// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};
// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


static int tabs = 0;

void Tabs()
{
    ImVec2 pos;
    ImDrawList* draw;

    pos = ImGui::GetWindowPos();
    draw = ImGui::GetWindowDrawList();

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Button] = ImColor(35, 35, 45, 0);
    colors[ImGuiCol_ButtonActive] = ImColor(63, 63, 60, 255);
    colors[ImGuiCol_ButtonHovered] = ImColor(63, 63, 60, 255);
    ImGui::PushFont(tolstiy2);
    ImGui::SetCursorPos(ImVec2(4, 393));
    if (ImGui::tab("s", 0 == tabs))
        tabs = 0;
    ImGui::SetCursorPos(ImVec2(260, 393));
    if (ImGui::tab("t", 1 == tabs))
        tabs = 1;
    ImGui::SetCursorPos(ImVec2(520, 393));
    if (ImGui::tab("v", 2 == tabs))
        tabs = 2;
    ImGui::PopFont();
}
// Main code
int main(int, char**)
{
    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Remastered"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }ImFont* tolstiy = nullptr;

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGuiStyle* style = &ImGui::GetStyle();
    ImGui::StyleColorsDark();

    style->Alpha = 1.f;
    style->WindowRounding = 1.f;
    style->FramePadding = ImVec2(4, 3);
    style->WindowPadding = ImVec2(8, 8);
    style->ItemInnerSpacing = ImVec2(4, 4);
    style->ItemSpacing = ImVec2(8, 4);
    style->FrameRounding = 5.f;
    style->ScrollbarSize = 2.f;
    style->ScrollbarRounding = 12.f;


    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_ChildBg] = ImColor(24, 29, 59, 0);
    colors[ImGuiCol_Border] = ImVec4(0.07f, 0.07f, 0.11f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImColor(25, 25, 33, 255);
    colors[ImGuiCol_FrameBgActive] = ImColor(25, 25, 33, 255);
    colors[ImGuiCol_FrameBgHovered] = ImColor(25, 25, 33, 255);
    colors[ImGuiCol_Header] = ImColor(25, 25, 33, 255);
    colors[ImGuiCol_HeaderActive] = ImColor(28, 28, 36, 255);
    colors[ImGuiCol_HeaderHovered] = ImColor(30, 30, 38, 255);
    colors[ImGuiCol_PopupBg] = ImColor(35, 35, 45, 255);
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    //ImGui::StyleColorsClassic();
    tolstiy = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/verdana.ttf", 40.0f);
    tolstiy2 = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/undefeated.ttf", 40.0f);
    ne_takoi_tolstiy = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/verdana.ttf", 13.0f);
    ne_takoi_tolstiy2 = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/verdana.ttf", 15.0f);

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);



    // Our state
    bool show_demo_window = true, loader_window = false;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 0.00f);
    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {

        if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImVec2 pos;
        ImDrawList* draw;
        if (soufiww == nullptr)
            D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice, &soufiw, sizeof(soufiw), 100, 100, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &soufiww);
        if (logow == nullptr)
            D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice, &logo, sizeof(logo), 100, 100, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &logow);

        {
            ImGui::Begin("Remastered", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);
            {
                pos = ImGui::GetWindowPos();
                draw = ImGui::GetWindowDrawList();

                ImGui::SetWindowSize(ImVec2(600, 480));

                
                draw->AddRectFilled(ImVec2(pos.x + 190, pos.y + 0), ImVec2(pos.x + 807, pos.y + 657), ImColor(8, 8, 8), 8.f, 10);
                

                
               draw->AddRectFilled(ImVec2(pos.x + 0, pos.y + 0), ImVec2(pos.x + 197, pos.y + 657), ImColor(8, 8, 8), 10.f, 5);
                

               
                draw->AddText(tolstiy, 35.f, ImVec2(pos.x + 58, pos.y + 13), ImColor(128, 0, 128), "Remastered");
                draw->AddText(tolstiy, 35.f, ImVec2(pos.x + 58, pos.y + 14), ImColor(255, 255, 255), "Remastered");
                draw->AddCircleImageFilled(logow, ImVec2(pos.x + 31, pos.y + 33), 26.f, ImColor(255, 255, 255), 360);
                

                
                draw->AddLine(ImVec2(pos.x + 1, pos.y + 65), ImVec2(pos.x + 700, pos.y + 65), ImColor(128, 0, 128));
                draw->AddLine(ImVec2(pos.x + 1, pos.y + 420), ImVec2(pos.x + 700, pos.y + 420), ImColor(128, 0, 128));


                Tabs();


                ImGui::SetCursorPos(ImVec2(100, 100));
                draw->AddCircleImageFilled(soufiww, ImVec2(pos.x + 560, pos.y + 33), 23.f, ImColor(255, 255, 255), 360);
                draw->AddText(ne_takoi_tolstiy, 15.f, ImVec2(pos.x + 490, pos.y + 23), ImColor(255, 255, 255), "mason");
             
                ImGui::PushFont(ne_takoi_tolstiy);
                bool te, te1, te2, te3, te4;
                if (tabs == 0)
                {
                    ImGui::BeginGroup();
                    draw->AddRectFilled(ImVec2(pos.x + 10, pos.y + 80), ImVec2(pos.x + 283, pos.y + 400), ImColor(13, 11, 16));
                    draw->AddLine(ImVec2(pos.x + 10, pos.y + 110), ImVec2(pos.x + 283, pos.y + 111), ImColor(128, 0, 128));
                    draw->AddText(ne_takoi_tolstiy, 15.f, ImVec2(pos.x + 128, pos.y + 88), ImColor(255, 255, 255, 255), "Aim");

                    ImGui::SetCursorPos(ImVec2(23, 118));
                    ImGui::Checkbox("Aimlock", &te);

                    ImGui::EndGroup();

                    ImGui::BeginGroup();
                    draw->AddRectFilled(ImVec2(pos.x + 299, pos.y + 80), ImVec2(pos.x + 590, pos.y + 400), ImColor(13, 11, 16));
                    draw->AddLine(ImVec2(pos.x + 299, pos.y + 110), ImVec2(pos.x + 590, pos.y + 111), ImColor(128, 0, 128));
                    draw->AddText(ne_takoi_tolstiy, 15.f, ImVec2(pos.x + 415, pos.y + 88), ImColor(255, 255, 255, 255), "Trigger");

                    ImGui::SetCursorPos(ImVec2(312, 118));
                    ImGui::Checkbox("Triggerbot", &te1);
                    ImGui::EndGroup();
                }

                if (tabs == 1)
                {
                    ImGui::BeginGroup();
                    draw->AddRectFilled(ImVec2(pos.x + 10, pos.y + 80), ImVec2(pos.x + 283, pos.y + 400), ImColor(13, 11, 16));
                    draw->AddLine(ImVec2(pos.x + 10, pos.y + 110), ImVec2(pos.x + 283, pos.y + 111), ImColor(128, 0, 128));
                    draw->AddText(ne_takoi_tolstiy, 15.f, ImVec2(pos.x + 128, pos.y + 88), ImColor(255, 255, 255, 255), "ESP");

                    ImGui::SetCursorPos(ImVec2(23, 118));
                    ImGui::Checkbox("Name", &te2);

                    ImGui::EndGroup();

                    ImGui::BeginGroup();
                    draw->AddRectFilled(ImVec2(pos.x + 299, pos.y + 80), ImVec2(pos.x + 590, pos.y + 400), ImColor(13, 11, 16));
                    draw->AddLine(ImVec2(pos.x + 299, pos.y + 110), ImVec2(pos.x + 590, pos.y + 111), ImColor(128, 0, 128));
                    draw->AddText(ne_takoi_tolstiy, 15.f, ImVec2(pos.x + 415, pos.y + 88), ImColor(255, 255, 255, 255), "Chams");

                    ImGui::SetCursorPos(ImVec2(312, 118));
                    ImGui::Checkbox("Chams", &te3);


                    ImGui::EndGroup();
                }

                if (tabs == 2)
                {
                    ImGui::BeginGroup();
                    draw->AddRectFilled(ImVec2(pos.x + 10, pos.y + 80), ImVec2(pos.x + 283, pos.y + 400), ImColor(13, 11, 16));
                    draw->AddLine(ImVec2(pos.x + 10, pos.y + 110), ImVec2(pos.x + 283, pos.y + 111), ImColor(128, 0, 128));
                    draw->AddText(ne_takoi_tolstiy, 15.f, ImVec2(pos.x + 128, pos.y + 88), ImColor(255, 255, 255, 255), "Misc");

                    ImGui::SetCursorPos(ImVec2(23, 118));
                    ImGui::Checkbox("Poop", &te4);

                    ImGui::EndGroup();

                    ImGui::BeginGroup();
                    draw->AddRectFilled(ImVec2(pos.x + 299, pos.y + 80), ImVec2(pos.x + 590, pos.y + 400), ImColor(13, 11, 16));
                    draw->AddLine(ImVec2(pos.x + 299, pos.y + 110), ImVec2(pos.x + 590, pos.y + 111), ImColor(128, 0, 128));
                    draw->AddText(ne_takoi_tolstiy, 15.f, ImVec2(pos.x + 415, pos.y + 88), ImColor(255, 255, 255, 255), "Config");

                    ImGui::EndGroup();
                }

                draw->AddRect(ImVec2(pos.x + 0, pos.y + 0), ImVec2(pos.x + 600, pos.y + 480), ImColor(128, 0, 128), 10.f, 5);

                ImGui::PopFont();
                //
                ImGui::End();
            }
        }

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * 255.0f), (int)(clear_color.y * 255.0f), (int)(clear_color.z * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

