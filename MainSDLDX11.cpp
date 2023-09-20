// dear imgui: standalone example application for SDL2 + DirectX 11
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_sdl.h"
#include "imgui/examples/imgui_impl_dx11.h"
#include <d3d11.h>
#include <stdio.h>
#include <chrono>  // for high_resolution_clock
#include "SDL2/include/SDL.h"
#include "SDL2/include/SDL_syswm.h"
#include "imgui/imgui_memory_editor.h"

// Data
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

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

        for (int y = 0 ; y < CPpu::e_ScanLines_VisibleY-8 ; y++)
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
                ImVec2 pv1 = ImVec2(tl.x + x*nPixelSize, tl.y);
                ImVec2 pv2 = ImVec2(tl.x + x*nPixelSize, tl.y+CPpu::e_ScanLines_VisibleY*nPixelSize);
                int nColor = (x%32)?0x80808080:0x80FFFFFF;
                draw_list->AddLine(pv1, pv2, nColor);
            }
            for (int y = 0 ; y < CPpu::e_ScanLines_VisibleY ; y += nSize)
            {
                ImVec2 pv1 = ImVec2(tl.x, tl.y + y*nPixelSize);
                ImVec2 pv2 = ImVec2(tl.x + CPpu::e_ScanLines_VisibleX*nPixelSize, tl.y + y*nPixelSize);
                int nColor = (y%32)?0x80808080:0x80FFFFFF;
                draw_list->AddLine(pv1, pv2, nColor);
            }
        }
    }

    ImGui::End();
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
SDL_AudioDeviceID m_AudioDev;

float g_fAudioMasterVolume = 0.25f;
float g_fAudioDebug02 = 50.f;
float g_fAudioDebug03 = 50.f;
float g_fAudioDebug04 = 50.f;

void InitSDLAudio(void)
{
    TimeAudio = std::chrono::high_resolution_clock::now();
    SDL_AudioSpec spec;
    SDL_memset(&spec, 0, sizeof(spec));
    spec.freq = AUDIO_FREQ; // 4 100 Hz, 48 000 Hz, 96 000 Hz, 192 000 Hz (standard) 
    spec.format = AUDIO_F32SYS;
    spec.channels = 1;
    spec.samples = (Uint16)(spec.freq / 60);
    spec.callback = NULL;
    m_AudioDev = SDL_OpenAudioDevice(nullptr, 0, &spec, &spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    SDL_PauseAudioDevice(m_AudioDev, SDL_FALSE);
}

void FillAudioData(bool bPlaySound)
{
    int nSizeToQueue = m_Nes.m_Apu.m_nWaveFormPos;//AUDIO_FREQ / 60;
    m_Nes.m_Apu.m_nWaveFormPos = 0;
    float pfSoundBuffer[(AUDIO_FREQ / 60) * 4];

    if (!g_bPlaySound || !bPlaySound)
    {
        for (int i = 0; i < nSizeToQueue; i++)
            pfSoundBuffer[i] = 0.0f;
        return;
    }

    std::chrono::time_point<std::chrono::high_resolution_clock> NewTime = std::chrono::high_resolution_clock::now();
    AudioElapseTime = NewTime - TimeAudio;
    //double fAudioElapseTime = AudioElapseTime.count();
    TimeAudio = NewTime;

    for (int i = 0; i < nSizeToQueue; i++)
    {
        pfSoundBuffer[i] = m_Nes.m_Apu.m_fWaveForm[i] * g_fAudioMasterVolume;

        //g_fAudioBuffer[g_nIndexAudioBuffer] = 1.0f/fAudioElapseTime;//m_Nes.m_Apu.m_nWaveFormPos;// - m_nLastWaveFormPos;
        //g_fAudioBuffer[g_nIndexAudioBuffer] = m_Nes.m_Apu.m_nWaveFormPos - m_nLastWaveFormPos;
        g_fAudioBuffer[g_nIndexAudioBuffer] = m_Nes.m_Apu.m_fWaveForm[i];
        //g_fAudioBuffer[g_nIndexAudioBuffer] = m_Nes.m_Apu.m_nCycle / 100.0f;
        g_nIndexAudioBuffer += 1;
        g_nIndexAudioBuffer %= (int)AUDIO_BUFFER_SIZE;
    }

    char *pErr = NULL;
    int ret = SDL_QueueAudio(m_AudioDev, pfSoundBuffer, nSizeToQueue * sizeof(float));
    if (ret != 0)
        pErr = (char *)SDL_GetError();
}

// Main code
int main(int, char**)
{
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Setup window
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_MAXIMIZED);
    SDL_Window* window = SDL_CreateWindow("Jgl NesEmu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1010, window_flags);
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    HWND hwnd = (HWND)wmInfo.info.win.window;

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForD3D(window);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = false;
    //bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    InitSDLAudio();

    m_Nes.Reset(g_RomName);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED && event.window.windowID == SDL_GetWindowID(window))
            {                
                // Release all outstanding references to the swap chain's buffers before resizing.
                CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
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
            int ScrollX = m_Nes.m_Ppu.m_u8ScrollX; ImGui::DragInt("ScrollX", &ScrollX, 1.0f, 0, 255, "%d"); m_Nes.m_Ppu.m_u8ScrollX = (uint8_t)ScrollX;
        }
        ImGui::End();

        if (!bDisplayOnlyScreen)
        {
        // Memedit & viewer.
            mem_edit.DrawWindow("Memory Editor", m_Nes.m_NesRam.m_CPURam, 64*1024);

            MemStack.Cols = 8;
            MemStack.GotoAddrAndHighlight(m_Nes.m_6502.m_SP, m_Nes.m_6502.m_SP+1);
            MemStack.GotoAddr = (size_t)-1;
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
                    int A = m_Nes.m_6502.m_A; ImGui::DragInt("A", &A, 1.0f, 0, 255, "%d"); m_Nes.m_6502.m_A = (uint8_t)A;
                    int X = m_Nes.m_6502.m_X; ImGui::DragInt("X", &X, 1.0f, 0, 255, "%d"); m_Nes.m_6502.m_X = (uint8_t)X;
                    int Y = m_Nes.m_6502.m_Y; ImGui::DragInt("Y", &Y, 1.0f, 0, 255, "%d"); m_Nes.m_6502.m_Y = (uint8_t)Y;
                    int PC = m_Nes.m_6502.m_PC; ImGui::DragInt("PC", &PC, 1.0f, 0, 65535, "%d"); m_Nes.m_6502.m_PC = (uint16_t)PC;
                    int SP = m_Nes.m_6502.m_SP; ImGui::DragInt("SP", &SP, 1.0f, 0, 65535, "%d"); m_Nes.m_6502.m_SP = (uint8_t)SP;
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
                    m_Nes.m_6502.SetBreakpoint((uint16_t)nAdrBreakPoint, 0);
                }
                else
                {
                    m_Nes.m_6502.SetBreakpoint((uint16_t)-1, 0);
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

            ImGui::DragFloat("Master Volume", &g_fAudioMasterVolume, 0.05f, 0.0f, 1.0f, "%f");
            ImGui::DragFloat("02", &g_fAudioDebug02, 1.0f, 1, 255, "%f");
            ImGui::DragFloat("03", &g_fAudioDebug03, 1.0f, 1, 255, "%f");
            ImGui::DragFloat("04", &g_fAudioDebug04, 1.0f, 1, 255, "%f");
        }
        ImGui::End();

        ImGui::Begin("APU Waveform", NULL);
        {
            ImGui::PlotLines("APU", g_fAudioBuffer, AUDIO_BUFFER_SIZE, g_nIndexAudioBuffer, "APU waveform", -1.0f, 1.0f, ImVec2(0,100));
        }
        ImGui::End();

    // Disasm
        static bool bRunOneFrameAtATime = false;
        uint16_t nAdr = m_Nes.m_6502.m_PC;
		ImGui::Begin("Disasm", NULL);
		{
            static int nNbStepInto = 1;

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
        ImGui::Render();
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync

        FillAudioData(bRunOneFrameAtATime);
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}
