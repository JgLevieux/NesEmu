#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_dx9.h"
#include "imgui/examples/imgui_impl_win32.h"
#include "imgui/imgui_memory_editor.h"
#include "SDL2/include/SDL.h"
#include <d3d9.h>
#include <chrono>  // for high_resolution_clock
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include "NesEmu/Nes.h"

CNes m_Nes;
MemoryEditor MemStack;
MemoryEditor mem_edit;
MemoryEditor MemPPUReg;
MemoryEditor MemPPU;
MemoryEditor MemZpg;
MemoryEditor MemNameTable;
MemoryEditor MemAttributeTable;

bool g_bDrawSprite = true;
bool g_bDrawBackground = true;

char *g_RomName = "TestsAuto\\nestest.nes";

// Data
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void DrawOnePixel(int x, int y, int nColor, int nPixelSize = 1)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 tl = ImGui::GetCursorScreenPos();
    ImVec2 p1 = ImVec2(x*nPixelSize + tl.x, y*nPixelSize + tl.y);
    ImVec2 p2 = ImVec2(x*nPixelSize + nPixelSize + tl.x, y*nPixelSize + nPixelSize + tl.y);
    draw_list->AddRectFilled(p1, p2, nColor);
}

void DrawChar(char *pName, uint8_t *pAdr)
{
	ImGui::SetNextWindowSize(ImVec2(128+32, 128+32));
	ImGui::Begin(pName, 0, ImGuiWindowFlags_NoResize);
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 tl = ImGui::GetCursorScreenPos();
        ImVec2 p1 = ImVec2(-1 + tl.x, -1 + tl.y);
        ImVec2 p2 = ImVec2(129 + tl.x, 129 + tl.y);
        draw_list->AddRect(p1, p2, 0xFFFFFFFF);

        for (int y = 0 ; y < 16 ; y++)       // 16 sprites en Y
        {
            for (int x = 0 ; x < 16 ; x++)      // 16 sprites en X
            {
                uint8_t *pChrRam = &pAdr[x*16+y*16*16];
                for (int suby = 0 ; suby < 8 ; suby++)      // 8 pixels par sprites.
                {
                    for (int subx = 0 ; subx < 8 ; subx++)      // 8 pixels par sprites.
                    {
                        uint8_t u8Color = ((pChrRam[0] >> (7 - subx)) & 0x01) | (((pChrRam[8] >> (7 - subx)) & 0x01) << 1);
                        if (u8Color)
                        {
                            u8Color = (u8Color*(256/4))&0xFF;
                            uint32_t u32Color = (u8Color << 16) | (u8Color << 8) | u8Color | 0xFF000000;
                            DrawOnePixel(x*8 + subx, y*8 + suby, u32Color);
                        }
                    }
                    pChrRam++;
                }
            }
        }
    }
    ImGui::End();
}

void DrawBackground(void)
{
    int nNbPixelDrawn = 0;
    static int nPixelSize = 2;
    static int nGridSize = 0;
	ImGui::SetNextWindowSize(ImVec2((float)(CPpu::e_ScanLines_VisibleX*nPixelSize+16), (float)(CPpu::e_ScanLines_VisibleY*nPixelSize+64)));
	ImGui::Begin("Screen", 0, ImGuiWindowFlags_NoResize);
    {
        if (ImGui::Button("+"))
            nPixelSize++;
        ImGui::SameLine();
        if (ImGui::Button("-"))
            nPixelSize--;
        if (nPixelSize < 1)
            nPixelSize = 1;
        ImGui::SameLine();
        if (ImGui::Button("Grid"))
        {
            nGridSize++;
            if (nGridSize == 3)
                nGridSize = 0;
        }
        ImGui::SameLine();
        ImGui::Checkbox("Background", &g_bDrawBackground);
        ImGui::SameLine();
        ImGui::Checkbox("Sprite", &g_bDrawSprite);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 tl = ImGui::GetCursorScreenPos();

        ImVec2 p1 = ImVec2(-1 + tl.x, -1 + tl.y);
        ImVec2 p2 = ImVec2(CPpu::e_ScanLines_VisibleX*nPixelSize+1 + tl.x, CPpu::e_ScanLines_VisibleY*nPixelSize+1 + tl.y);
        draw_list->AddRect(p1, p2, 0xFFFFFFFF);

        for (int y = 0 ; y < CPpu::e_ScanLines_VisibleY ; y++)
        {
            for (int x = 0 ; x < CPpu::e_ScanLines_VisibleX ; x++)
            {
                int nIndex = x + y*CPpu::e_ScanLines_VisibleX;
                uint32_t nPixel = m_Nes.m_Ppu.m_ScreenRender[nIndex];
                if ((nPixel & 0xF0000000) && !g_bDrawBackground)
                    continue;
                if ((nPixel & 0x0F000000) && !g_bDrawSprite)
                    continue;
                nPixel |= 0xFF000000;
                DrawOnePixel(x, y, nPixel, nPixelSize);
                if (nPixel != 0)
                    nNbPixelDrawn++;
            }
        }

        if (nGridSize != 0)
        {
            int nSize = nGridSize == 1?8:32;
            for (int x = 0 ; x < CPpu::e_ScanLines_VisibleX ; x += nSize)
            {
                ImVec2 p1 = ImVec2(tl.x + x*nPixelSize, tl.y);
                ImVec2 p2 = ImVec2(tl.x + x*nPixelSize, tl.y+CPpu::e_ScanLines_VisibleY*nPixelSize);
                int nColor = (x%32)?0x80808080:0x80FFFFFF;
                draw_list->AddLine(p1, p2, nColor);
            }
            for (int y = 0 ; y < CPpu::e_ScanLines_VisibleY ; y += nSize)
            {
                ImVec2 p1 = ImVec2(tl.x, tl.y + y*nPixelSize);
                ImVec2 p2 = ImVec2(tl.x + CPpu::e_ScanLines_VisibleX*nPixelSize, tl.y + y*nPixelSize);
                int nColor = (y%32)?0x80808080:0x80FFFFFF;
                draw_list->AddLine(p1, p2, nColor);
            }
        }
    }

    ImGui::End();
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            g_d3dpp.BackBufferWidth  = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
            if (hr == D3DERR_INVALIDCALL)
                IM_ASSERT(0);
            ImGui_ImplDX9_CreateDeviceObjects();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int nNbCallFillAudio = 0;
bool g_bPlaySound = true;
std::chrono::time_point<std::chrono::high_resolution_clock> TimeAudio;
std::chrono::duration<double> AudioElapseTime;
double g_fCurrentAudioTime = 0.0f;
#define AUDIO_FREQ 48000
#define AUDIO_BUFFER_TIME 1
#define AUDIO_BUFFER_SIZE (AUDIO_FREQ * AUDIO_BUFFER_TIME)
float g_fAudioBuffer[AUDIO_BUFFER_SIZE] = {0};
int g_nIndexAudioBuffer = 0;

float g_fAudioDebug01 = 100.f;
float g_fAudioDebug02 = 50.f;
float g_fAudioDebug03 = 50.f;
float g_fAudioDebug04 = 50.f;

int m_nLastWaveFormPos = 0;

void FillAudio(void* param, Uint8* stream, int len)
{
    int samples = len / sizeof(float);
    float *pfSoundBuffer = (float *)stream;

    if (!g_bPlaySound)
    {
        for (int i = 0; i < samples; i++)
            pfSoundBuffer[i] = 0.0f;
        return;
    }

    std::chrono::time_point<std::chrono::high_resolution_clock> NewTime = std::chrono::high_resolution_clock::now();
    AudioElapseTime = NewTime - TimeAudio;
    double fAudioElapseTime = AudioElapseTime.count();
    TimeAudio = NewTime;

    for (int i = 0; i < samples; i++)
    {
        int nIndexInWaveForm = m_Nes.m_Apu.m_nWaveFormPos - (samples-i);
        if (nIndexInWaveForm < 0)
            nIndexInWaveForm += APU_WAVE_FORM_MAX_SIZE;
        pfSoundBuffer[i] = m_Nes.m_Apu.m_fWaveForm[nIndexInWaveForm];

        g_fAudioBuffer[g_nIndexAudioBuffer] = len;//1.0f/fAudioElapseTime;//m_Nes.m_Apu.m_nWaveFormPos;// - m_nLastWaveFormPos;
        g_nIndexAudioBuffer += 1;
        g_nIndexAudioBuffer %= (int)AUDIO_BUFFER_SIZE;
    }

    m_nLastWaveFormPos = m_Nes.m_Apu.m_nWaveFormPos;
    //memcpy(pfSoundBuffer, m_Nes.m_Apu.m_fWaveForm, len);

/*    nNbCallFillAudio = g_nIndexAudioBuffer;

    int nNewIndex = (int)fmod(g_fCurrentAudioTime, AUDIO_BUFFER_TIME) * AUDIO_FREQ;

    double fCurrentTime = g_fCurrentAudioTime;
    double fTimePerSample = fAudioElapseTime / (double)samples;

    for (int i = 0; i < samples; i++)
    {
        g_fAudioBuffer[g_nIndexAudioBuffer] = SDL_sinf((float)fCurrentTime);

        pfSoundBuffer[i] = g_fAudioBuffer[g_nIndexAudioBuffer];

        g_nIndexAudioBuffer += 1;
        g_nIndexAudioBuffer %= (int)AUDIO_BUFFER_SIZE;
        fCurrentTime += fTimePerSample*g_fAudioDebug01;
    }

    g_fCurrentAudioTime += fAudioElapseTime*g_fAudioDebug01;
    */
}

int main(int, char**)
{
    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("NesEmu"), NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindow(_T("NesEmu"), _T("NesEmu"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    LPDIRECT3D9 pD3D;
    if ((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
    {
        UnregisterClass(_T("NesEmu"), wc.hInstance);
        return 0;
    }
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // Present without vsync, maximum unthrottled framerate

    // Create the D3DDevice
    if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
    {
        pD3D->Release();
        UnregisterClass(_T("ImGui Example"), wc.hInstance);
        return 0;
    }

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Keyboard Controls
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    // Setup style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them. 
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple. 
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    TimeAudio = std::chrono::high_resolution_clock::now();
    SDL_Init(SDL_INIT_AUDIO);
    SDL_AudioSpec spec;
    SDL_memset(&spec, 0, sizeof(spec));
    spec.freq = AUDIO_FREQ; // 4 100 Hz, 48 000 Hz, 96 000 Hz, 192 000 Hz (standard) 
    spec.format = AUDIO_F32SYS;
    spec.channels = 1;
    spec.samples = (spec.freq / 60);
    spec.callback = [](void* param, Uint8* stream, int len)
    {
        FillAudio(param, stream, len);
    };
    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(nullptr, 0, &spec, &spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    SDL_PauseAudioDevice(dev, SDL_FALSE);

    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    m_Nes.Reset(g_RomName);

    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    UpdateWindow(hwnd);
    while (msg.message != WM_QUIT)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

	// Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

	// Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        //if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

    // Misc.
        static bool bDisplayOnlyScreen = false;
		ImGui::Begin("Info", NULL);
        {
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            if (!bDisplayOnlyScreen)
            {
                if (ImGui::Button("Display only screen"))
                    bDisplayOnlyScreen = true;
            }
            else
            {
                if (ImGui::Button("Display all"))
                    bDisplayOnlyScreen = false;
            }
        }
        ImGui::End();
    // Rom loader.
		ImGui::Begin("Reset ROM", NULL);
        {
            if (ImGui::Button("Nestest"))
            {
                g_RomName = "TestsAuto\\nestest.nes";
                m_Nes.Reset(g_RomName);
            }
            ImGui::SameLine();
            if (ImGui::Button("SpriteHit"))
            {
                g_RomName = "TestsAuto\\ppu_sprite_hit.nes";
                g_RomName = "TestsAuto\\09-timing.nes";
                m_Nes.Reset(g_RomName);
            }
            ImGui::SameLine();
            if (ImGui::Button("Mario"))
            {
                g_RomName = "roms\\Mario Bros. (World).nes";
                m_Nes.Reset(g_RomName);
            }
            ImGui::SameLine();
            if (ImGui::Button("Super Mario"))
            {
                g_RomName = "roms\\Super Mario Bros. (World).nes";
                m_Nes.Reset(g_RomName);
            }
            if (ImGui::Button("Donkey Kong"))
            {
                g_RomName = "roms\\Donkey Kong (World) (Rev A).nes";
                m_Nes.Reset(g_RomName);
            }
            ImGui::SameLine();
            if (ImGui::Button("Tetris"))
            {
                g_RomName = "roms\\Tetris (Europe).nes";
                m_Nes.Reset(g_RomName);
            }
            ImGui::SameLine();
            if (ImGui::Button("Zelda"))
            {
                g_RomName = "roms\\Legend of Zelda, The (USA).nes";
                m_Nes.Reset(g_RomName);
            }
        }
        ImGui::End();

    // Input.
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
            m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_Down);
        else
            m_Nes.m_NesRam.m_Controller_0.ResetButton(CNesRam::SPaddle::e_PaddleButtons_Down);
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
            m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_Up);
        else
            m_Nes.m_NesRam.m_Controller_0.ResetButton(CNesRam::SPaddle::e_PaddleButtons_Up);
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
            m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_Right);
        else
            m_Nes.m_NesRam.m_Controller_0.ResetButton(CNesRam::SPaddle::e_PaddleButtons_Right);
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
            m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_Left);
        else
            m_Nes.m_NesRam.m_Controller_0.ResetButton(CNesRam::SPaddle::e_PaddleButtons_Left);

        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Z)))
            m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_A);
        else
            m_Nes.m_NesRam.m_Controller_0.ResetButton(CNesRam::SPaddle::e_PaddleButtons_A);
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_X)))
            m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_B);
        else
            m_Nes.m_NesRam.m_Controller_0.ResetButton(CNesRam::SPaddle::e_PaddleButtons_B);

        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Space)))
            m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_Select);
        else
            m_Nes.m_NesRam.m_Controller_0.ResetButton(CNesRam::SPaddle::e_PaddleButtons_Select);
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Enter)))
            m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_Start);
        else
            m_Nes.m_NesRam.m_Controller_0.ResetButton(CNesRam::SPaddle::e_PaddleButtons_Start);

        ImGui::Begin("Input", NULL);
        {
            if (ImGui::Button("A"))
                m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_A);
            ImGui::SameLine();
            if (ImGui::Button("B"))
                m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_B);
            if (ImGui::Button("Up"))
                m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_Up);
            if (ImGui::Button("Down"))
                m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_Down);
            if (ImGui::Button("Left"))
                m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_Left);
            if (ImGui::Button("Right"))
                m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_Right);
            if (ImGui::Button("Select"))
                m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_Select);
            ImGui::SameLine();
            if (ImGui::Button("Start"))
                m_Nes.m_NesRam.m_Controller_0.SetButton(CNesRam::SPaddle::e_PaddleButtons_Start);
        }
        ImGui::End();

        ImGui::Begin("PPU Debug", NULL);
        {
            int ScrollX = m_Nes.m_Ppu.m_u8ScrollX; ImGui::DragInt("ScrollX", &ScrollX, 1.0f, 0, 255, "%d"); m_Nes.m_Ppu.m_u8ScrollX = ScrollX;
        }
        ImGui::End();

        if (!bDisplayOnlyScreen)
        {
        // Memedit & viewer.
            mem_edit.DrawWindow("Memory Editor", m_Nes.m_NesRam.m_CPURam, 64*1024);

            MemStack.Cols = 8;
            MemStack.GotoAddrAndHighlight(m_Nes.m_6502.m_SP, m_Nes.m_6502.m_SP+1);
            MemStack.GotoAddr = -1;
            MemStack.DrawWindow("Stack", m_Nes.m_NesRam.m_CPURam+0x100, 0x100);

            MemZpg.Cols = 8;
            MemZpg.DrawWindow("Zero Page", m_Nes.m_NesRam.m_CPURam, 256);

            MemPPUReg.Cols = 8;
            MemPPUReg.DrawWindow("Ppu Register", m_Nes.m_NesRam.m_PPUReg, 8);

            MemPPU.DrawWindow("Ppu Ram", m_Nes.m_NesRam.m_PPURam, 0x4000);

            static uint8_t *nNameTable = m_Nes.m_NesRam.m_NameTable0;
		    ImGui::Begin("NameTable select", NULL);
            {
                if (ImGui::Button("0")) nNameTable = m_Nes.m_NesRam.m_NameTable0;
                ImGui::SameLine();
                if (ImGui::Button("1")) nNameTable = m_Nes.m_NesRam.m_NameTable1;
                if (ImGui::Button("2")) nNameTable = m_Nes.m_NesRam.m_NameTable2;
                ImGui::SameLine();
                if (ImGui::Button("3")) nNameTable = m_Nes.m_NesRam.m_NameTable3;
            }
            ImGui::End();

            MemNameTable.Cols = 0x20;
            MemNameTable.DrawWindow("NameTable", nNameTable, 0x1E*0x20); // Nametable

            MemAttributeTable.Cols = 8;
            MemAttributeTable.DrawWindow("AttributeTable", m_Nes.m_NesRam.m_NameTable0+0x1E*0x20, 64); // AttributeTable

        // Register
		    ImGui::Begin("Register", NULL);
		    {
                static bool bIsEditable = true;

                char pText[256];
                sprintf_s(pText, "A=$%02x X=$%02x Y=$%02x", m_Nes.m_6502.m_A, m_Nes.m_6502.m_X, m_Nes.m_6502.m_Y);
			    ImGui::Text(pText);

                sprintf_s(pText, "SP=$%02x PC=$%04x SR=$%02x", m_Nes.m_6502.m_SP, m_Nes.m_6502.m_PC, m_Nes.m_6502.m_SR);
			    ImGui::Text(pText);

			    ImGui::Text("NV-BDIZC");
                sprintf_s(pText, "%d%d-%d%d%d%d%d", m_Nes.m_6502.m_N, m_Nes.m_6502.m_V, m_Nes.m_6502.m_B, m_Nes.m_6502.m_D, m_Nes.m_6502.m_I, m_Nes.m_6502.m_Z, m_Nes.m_6502.m_C);
			    ImGui::Text(pText);

                sprintf_s(pText, "PPU=%03d,%03d CPU=%d ", m_Nes.m_Ppu.m_nScanLines, m_Nes.m_Ppu.m_nCycle, m_Nes.m_6502.m_nCycle);
			    ImGui::Text(pText);

                ImGui::Checkbox("Editable", &bIsEditable);
                if (bIsEditable)
                {
                    int A = m_Nes.m_6502.m_A; ImGui::DragInt("A", &A, 1.0f, 0, 255, "%d"); m_Nes.m_6502.m_A = A;
                    int X = m_Nes.m_6502.m_X; ImGui::DragInt("X", &X, 1.0f, 0, 255, "%d"); m_Nes.m_6502.m_X = X;
                    int Y = m_Nes.m_6502.m_Y; ImGui::DragInt("Y", &Y, 1.0f, 0, 255, "%d"); m_Nes.m_6502.m_Y = Y;
                    int PC = m_Nes.m_6502.m_PC; ImGui::DragInt("PC", &PC, 1.0f, 0, 65535, "%d"); m_Nes.m_6502.m_PC = PC;
                    int SP = m_Nes.m_6502.m_SP; ImGui::DragInt("SP", &SP, 1.0f, 0, 65535, "%d"); m_Nes.m_6502.m_SP = SP;
                }
            }
            ImGui::End();

            ImGui::Begin("Disasm Log", NULL);
            {
                if (ImGui::Button("Save to log"))
                {
                    FILE *pFileLog = fopen("TestsAuto\\disasmlog.txt", "wt");
                    for (int i = 0 ; i < m_Nes.m_nNbLogLines ; i++)
                    {
                        fwrite(m_Nes.m_cDisasembleLogText[i], 1, strlen(m_Nes.m_cDisasembleLogText[i]), pFileLog);
                        fwrite("\n", 1, strlen("\n"), pFileLog);
                    }
                    fclose(pFileLog);
                }
                for (int i = 0 ; i < m_Nes.m_nNbLogLines ; i++)
                {
                    char pcTxt[256];
                    sprintf_s(pcTxt, "%d : %s", i+1, m_Nes.m_cDisasembleLogText[i]);
                    ImGui::Text(pcTxt);
                }
            }
            ImGui::End();

        // Breakpoint
		    ImGui::Begin("Breakpoint", NULL);
            {
                static char pcBreak00[16] = "";
                static bool bEnableB00 = false;
                ImGui::Checkbox("", &bEnableB00);
                ImGui::SameLine();
                ImGui::InputText("B00", pcBreak00, 16);
                if (bEnableB00)
                {
                    int nAdrBreakPoint = 0;
                    sscanf_s(pcBreak00, "%x", &nAdrBreakPoint);
                    m_Nes.m_6502.SetBreakpoint(nAdrBreakPoint, 0);
                }
                else
                {
                    m_Nes.m_6502.SetBreakpoint(-1, 0);
                }
            }
            ImGui::End();
        }

		ImGui::Begin("APU Registers", NULL);
        {
            char pcTxt[256];
            double fAudioTime = AudioElapseTime.count();
            sprintf_s(pcTxt, "nNbCallFillAudio : %d - %f (%f)", nNbCallFillAudio, fAudioTime, 1.0f/fAudioTime);
            ImGui::Text(pcTxt);

            ImGui::Checkbox("Sound", &g_bPlaySound);

            ImGui::DragFloat("01", &g_fAudioDebug01, 1.0f, 1, 255, "%f");
            ImGui::DragFloat("02", &g_fAudioDebug02, 1.0f, 1, 255, "%f");
            ImGui::DragFloat("03", &g_fAudioDebug03, 1.0f, 1, 255, "%f");
            ImGui::DragFloat("04", &g_fAudioDebug04, 1.0f, 1, 255, "%f");
        }
        ImGui::End();

        ImGui::Begin("APU Waveform", NULL);
        {
            ImGui::PlotLines("APU", g_fAudioBuffer, AUDIO_BUFFER_SIZE, g_nIndexAudioBuffer, "APU waveform", -4096.0f, 4096.0f, ImVec2(0,100));
        }
        ImGui::End();

    // Disasm
        unsigned short nAdr = m_Nes.m_6502.m_PC;
		ImGui::Begin("Disasm", NULL);
		{
            static int nNbStepInto = 1;

            static bool bRunOneFrameAtATime = false;
            if (!bRunOneFrameAtATime)
            {
                if (ImGui::Button("Run"))
                    bRunOneFrameAtATime = true;
            }
            else
            {
                if (ImGui::Button("Stop"))
                    bRunOneFrameAtATime = false;
            }
            if (bRunOneFrameAtATime)
            {
                bool bBreak = false;
                do
                {
                    bBreak = m_Nes.Process(true);
                } while (!m_Nes.m_bPassFrame && !bBreak);
                if (bBreak)
                    bRunOneFrameAtATime = false;
            }

            if (ImGui::Button("Step One Frame"))
            {
                m_Nes.m_nNbLogLines = 0;
                do
                {
                    m_Nes.Process(true, false);
                } while (!m_Nes.m_bPassFrame);
            }

            if (ImGui::Button("Step Into"))
            {
                for (int i = 0 ; i < nNbStepInto ; i++)
                {
                    m_Nes.Process(true);
                }
            }

            ImGui::DragInt("NbStep", &nNbStepInto, 1.0f, 0, 255, "%d");

            if (ImGui::Button("Run to BRK"))
            {
                bool bBreak = false;
                while (!bBreak)
                {
                    bBreak = m_Nes.Process(false, true);
                }
            }

            if (!bDisplayOnlyScreen)
            {
                for (int i = 0 ; i < 32 ; i++)
                {
                    char pText[256];
                    nAdr = m_Nes.m_6502.Disasm(nAdr, pText);
			        ImGui::Text(pText);
                }
            }
        }
        ImGui::End();

    // Draw
        DrawBackground();

        if (!bDisplayOnlyScreen)
        {
            DrawChar("PatternTable0", m_Nes.m_NesRam.m_PatternTable0);
            DrawChar("PatternTable1", m_Nes.m_NesRam.m_PatternTable1);
        }

	// Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x*255.0f), (int)(clear_color.y*255.0f), (int)(clear_color.z*255.0f), (int)(clear_color.w*255.0f));
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
        {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            g_pd3dDevice->Reset(&g_d3dpp);
            ImGui_ImplDX9_CreateDeviceObjects();
        }
    }

    SDL_Quit();

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (g_pd3dDevice) g_pd3dDevice->Release();
    if (pD3D) pD3D->Release();
    DestroyWindow(hwnd);
    UnregisterClass(_T("ImGui Example"), wc.hInstance);

    return 0;
}
