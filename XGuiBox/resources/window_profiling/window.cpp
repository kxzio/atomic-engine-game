#include "window.h"
#include <sstream>
#include "../../misc/instruments.h"
#include "../../xguibox/xgui.h"
#include "../../menu.h"
#include "../imgui_resource/imgui_freetype.h"
#include "../planet_menu.h"
#include "../noise.h"
#include "../RGB_LINES.h"
#include "../USA.h"
#include "../Turk.h"
#include "../tv.h"
#include <algorithm>

#include <d3d9.h>
#include <d3dx9.h>

extern "C" 
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

#include <ctime>
#include <random>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#pragma comment(lib, "C:\\Users\\sasha\\OneDrive\\Documents\\ffmpeg lib\\ffmpeg-4.3.1-win64-dev\\lib\\avcodec.lib")
#pragma comment(lib, "C:\\Users\\sasha\\OneDrive\\Documents\\ffmpeg lib\\ffmpeg-4.3.1-win64-dev\\lib\\avformat.lib")
#pragma comment(lib, "C:\\Users\\sasha\\OneDrive\\Documents\\ffmpeg lib\\ffmpeg-4.3.1-win64-dev\\lib\\avutil.lib")
#pragma comment(lib, "C:\\Users\\sasha\\OneDrive\\Documents\\ffmpeg lib\\ffmpeg-4.3.1-win64-dev\\lib\\swscale.lib")

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "bcrypt.lib")

#include "../nanosvg.h"
#include "../nanosvgrast.h"

IDirect3DTexture9* g_videoTexture = nullptr;
AVFormatContext* g_formatContext = nullptr;
AVCodecContext* g_codecContext = nullptr;
AVFrame* g_frame = nullptr;
AVPacket* g_packet = nullptr;
struct SwsContext* g_swsContext = nullptr;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool isPaused = false;

void PauseRendering(bool pause) {
    isPaused = pause;
}

// Настройки для видео
const char* g_videoPath = "123.mp4"; // Укажите путь к видео
int g_videoStreamIndex = -1;


void UpdateVideoFrame(IDirect3DDevice9* device) {
    // Проверяем, что g_formatContext был инициализирован
    if (g_formatContext == nullptr) {
        std::cerr << "AVFormatContext is not initialized!" << std::endl;
        return;
    }


    // Чтение кадра из видео потока
    int ret = av_read_frame(g_formatContext, g_packet);
    if (ret < 0) {

        // Если достигнут конец видео, перематываем на начало
        if (ret == AVERROR_EOF) {
            av_seek_frame(g_formatContext, g_videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD); // Перематываем на начало
            avcodec_flush_buffers(g_codecContext); // Сбрасываем декодер
        }
        return;
    }

    // Декодируем кадр
    if (g_packet->stream_index == g_videoStreamIndex) {
        int response = avcodec_send_packet(g_codecContext, g_packet);
        if (response >= 0) {
            response = avcodec_receive_frame(g_codecContext, g_frame);
            if (response == 0) {
                // Конвертируем кадр в формат RGBA
                uint8_t* dest[4] = { nullptr };
                int destLinesize[4] = { 0 };

                // Выделяем память для конвертированного кадра
                av_image_alloc(dest, destLinesize, g_codecContext->width, g_codecContext->height, AV_PIX_FMT_RGBA, 1);

                // Преобразуем YUV в RGBA
                sws_scale(g_swsContext, g_frame->data, g_frame->linesize, 0, g_codecContext->height, dest, destLinesize);

                // Модификация данных: Удаляем зеленый канал (или заменяем его на другой цвет)
                for (int y = 0; y < g_codecContext->height; ++y) {
                    for (int x = 0; x < g_codecContext->width; ++x) {
                        uint8_t* pixel = dest[0] + y * destLinesize[0] + x * 4;

                        // Удаление зеленого канала
                        pixel[0] = 0; // Убираем зеленый канал, оставляя только красный и синий
                        //pixel[2] = 0; // Убираем зеленый канал, оставляя только красный и синий
                    }
                }

                // Копируем данные в текстуру
                D3DLOCKED_RECT lockedRect;
                g_videoTexture->LockRect(0, &lockedRect, nullptr, 0);

                // Копируем данные из буфера в текстуру
                uint8_t* textureData = (uint8_t*)lockedRect.pBits;
                for (int y = 0; y < g_codecContext->height; ++y) {
                    memcpy(textureData + y * lockedRect.Pitch, dest[0] + y * destLinesize[0], g_codecContext->width * 4);
                }

                g_videoTexture->UnlockRect(0);

                // Освобождаем память для временного кадра
                av_freep(&dest[0]);
            }
        }
    }

    // Очистка пакета после использования
    av_packet_unref(g_packet);
}

void InitVideo(IDirect3DDevice9* device) {
    // Инициализация FFmpeg
    g_formatContext = avformat_alloc_context();
    if (avformat_open_input(&g_formatContext, g_videoPath, nullptr, nullptr) != 0) {
        std::cerr << "Не удалось открыть видео!" << std::endl;
        return;
    }
    if (avformat_find_stream_info(g_formatContext, nullptr) < 0) {
        std::cerr << "Не удалось найти информацию о потоках!" << std::endl;
        return;
    }

    // Поиск видеопотока
    for (unsigned int i = 0; i < g_formatContext->nb_streams; i++) {
        if (g_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            g_videoStreamIndex = i;
            break;
        }
    }
    if (g_videoStreamIndex == -1) {
        std::cerr << "Видео-поток не найден!" << std::endl;
        return;
    }

    // Настройка кодека
    AVCodecParameters* codecParams = g_formatContext->streams[g_videoStreamIndex]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
    g_codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(g_codecContext, codecParams);
    if (avcodec_open2(g_codecContext, codec, nullptr) < 0) {
        std::cerr << "Не удалось открыть кодек!" << std::endl;
        return;
    }

    // Создание текстуры
    device->CreateTexture(
        g_codecContext->width,
        g_codecContext->height,
        1,
        0,
        D3DFMT_A8R8G8B8,
        D3DPOOL_MANAGED,
        &g_videoTexture,
        nullptr
    );

    // Создание кадров и пакетов
    g_frame = av_frame_alloc();
    g_packet = av_packet_alloc();

    // Настройка SwsContext для преобразования YUV -> RGB
    g_swsContext = sws_getContext(
        g_codecContext->width, g_codecContext->height, g_codecContext->pix_fmt,
        g_codecContext->width, g_codecContext->height, AV_PIX_FMT_RGBA,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );
}

const char* countries_file_name[] =
{
     "Worldmap\\USA.png",
     "Worldmap\\ЕС.png",
     "Worldmap\\Northeurope.png",
     "Worldmap\\Australia.png",
     "Worldmap\\Russia.png",
     "Worldmap\\China.png",
     "Worldmap\\Churki.png",
     "Worldmap\\EastEC.png",
     "Worldmap\\east-europe.png",
     "Worldmap\\Indo-China.png",
     "Worldmap\\Indostan.png",
     "Worldmap\\LatinUSA.png",
     "Worldmap\\North-WellWellWell.png",
     "Worldmap\\Samurai.png",
     "Worldmap\\SouthernUSA.png",
     "Worldmap\\MidWellWellWell.png",
     "Worldmap\\South-WellWellWell.png",
     "Worldmap\\Turk.png",
     "Worldmap\\Zakavkazie.png"
};

void window_profiling::unload_textures()
{
    if (g_videoTexture)     g_videoTexture->Release();
    if (Tv)                 Tv->Release();
    if (Noise)              Noise->Release();
    if (Logotype)           Logotype->Release();
    if (RGB)                RGB->Release();


    for (int i = 0; i < ARRAYSIZE(countries_file_name); i++)
    {
        if (countries[i]) countries[i]->Release();
    }
}
void window_profiling::load_textures()

{
    InitVideo(g_pd3dDevice);

    D3DXCreateTextureFromFileInMemory(g_pd3dDevice, &planet_menu, sizeof(planet_menu), &Logotype);
    D3DXCreateTextureFromFileInMemory(g_pd3dDevice, &noise, sizeof(noise), &Noise);
    D3DXCreateTextureFromFileInMemory(g_pd3dDevice, &tv, sizeof(tv), &Tv);
    D3DXCreateTextureFromFileInMemory(g_pd3dDevice, &RGB_LINES, sizeof(RGB_LINES), &RGB);

    for (int i = 0; i < ARRAYSIZE(countries_file_name); i++)
    {
        if (countries_file_name[i])
            D3DXCreateTextureFromFileA(g_pd3dDevice, countries_file_name[i], &countries[i]);
    }
}
void window_profiling::handle_device_lost() {

    HRESULT hr = g_pd3dDevice->TestCooperativeLevel();

    if (hr == D3DERR_DEVICELOST) {
        Sleep(100);  // можно попробовать различные задержки
    }
    else if (hr == D3DERR_DEVICENOTRESET) {
        // Устройство нужно перезагрузить

        unload_textures();

        HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
        if (FAILED(hr)) {
            // Выводим конкретный HRESULT код для диагностики
            std::wostringstream oss;
            oss << L"Failed to reset device. HRESULT: 0x" << std::hex << hr;
            MessageBox(NULL, oss.str().c_str(), L"Error", MB_OK | MB_ICONERROR);
        }

        load_textures();

    }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static POINTS ptsBegin;

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {

    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE) {
            // Окно свернуто или потеряло фокус
            PauseRendering(true);
        }
        else {
            // Окно восстановлено
            PauseRendering(false);
        }
        return 0;

    case WM_SIZE:
        g_xgui.real_size_of_window_x = LOWORD(lParam);
        g_xgui.real_size_of_window_y = HIWORD(lParam);
        g_xgui.hwnd_of_process = hWnd;
        InvalidateRect(hWnd, NULL, TRUE);

        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
            if (hr == D3DERR_INVALIDCALL)
                IM_ASSERT(0);
            ImGui_ImplDX9_CreateDeviceObjects();
        }
        return 0;

    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_LBUTTONDOWN:
        ptsBegin = MAKEPOINTS(lParam);
        SetCapture(hWnd);
        break;
    case WM_MOUSEMOVE:
        if (wParam & MK_LBUTTON)
        {
            // Optional mouse move handling
        }
        break;
    case WM_LBUTTONUP:
        ReleaseCapture();
        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
class FPS_Limiter {
public:
    FPS_Limiter(int targetFPS) {
        targetFrameTime = 1000 / targetFPS;
    }

    void WaitForNextFrame() {

        DWORD currentTime = GetTickCount64();
        DWORD timeSpent = currentTime - lastFrameTime;

        if (timeSpent < targetFrameTime) {
            DWORD sleepTime = targetFrameTime - timeSpent;
            Sleep(sleepTime); 
        }

        lastFrameTime = GetTickCount64();
    }

private:
    DWORD lastFrameTime = GetTickCount64();
    DWORD targetFrameTime;  // Время для каждого кадра (в миллисекундах)
};
void window_profiling::set_window_size(ImVec2 par)
{
    this->window_size = par;
}

void window_profiling::set_window_pos(ImVec2 par)
{
    this->window_pos = par;
}

void window_profiling::set_window_type(int type)
{
    this->type_of_window = type;
}

void window_profiling::make_it_fullscreen()
{
    HDC hDCScreen = GetDC(NULL);

    int screen_x = GetDeviceCaps(hDCScreen, HORZRES);
    int screen_y = GetDeviceCaps(hDCScreen, VERTRES);

    this->window_pos = ImVec2(0, 0);
    this->window_size = ImVec2(screen_x, screen_y);
    this->type_of_window = types_of_window::POPUP;

    ReleaseDC(NULL, hDCScreen);
}


bool IsWindowMinimized(HWND hWnd) { return IsIconic(hWnd); }

void window_profiling::create_window() 
{
    FPS_Limiter fps(150);

    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Window"), NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindow(_T("Window"), _T("Defcon"), types_of_window::POPUP, 0, 0, this->window_size.x, this->window_size.y, NULL, NULL, wc.hInstance, NULL);

    LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D) return;

    D3DPRESENT_PARAMETERS g_d3dpp = {};
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8; // Формат цвета
    g_d3dpp.BackBufferWidth = GetSystemMetrics(SM_CXSCREEN);  // Ширина экрана
    g_d3dpp.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN); // Высота экрана
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Избегает блокировок
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16; // Используйте аппаратный z-буфер

    pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &g_d3dpp, &g_pd3dDevice);

    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING,              FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE,               FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);

    RECT scissorRect = { 0, 0, this->window_size.x, this->window_size.y }; // Параметры экрана, где рендерить
    g_pd3dDevice->SetScissorRect(&scissorRect);
    g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

    D3DVIEWPORT9 viewport;
    viewport.X = 0;
    viewport.Y = 0;
    viewport.Width = this->window_size.x;
    viewport.Height = this->window_size.y;
    viewport.MinZ = 0.0f;
    viewport.MaxZ = 1.0f;

    g_pd3dDevice->SetViewport(&viewport);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    ImFontConfig cfg;
    cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;

    static bool loaded = false;

    if (!loaded)
    {
        this->load_textures();

        for (int i = 0; i < g_xgui.fonts.size(); i++)
        {
            ImGuiIO& io = ImGui::GetIO();
            switch (g_xgui.fonts[i].font_mode)
            {
            case 0:
            {
                if (i == 2 || i == 3)
                    cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_LightHinting;
                else
                    cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;

                std::stringstream ss;
                ss << "C:\\Windows\\Fonts\\" << g_xgui.fonts[i].font_name << ".ttf";
                g_xgui.fonts[i].font_addr = io.Fonts->AddFontFromFileTTF(ss.str().c_str(), g_xgui.fonts[i].size_of_font, &cfg, io.Fonts->GetGlyphRangesCyrillic());
            }
            break;
            case 1:
                g_xgui.fonts[i].font_addr = io.Fonts->AddFontFromMemoryTTF(g_xgui.fonts[i].font_data, g_xgui.fonts[i].font_data_size, g_xgui.fonts[i].size_of_font, &cfg, io.Fonts->GetGlyphRangesCyrillic());
                break;
            case -1:
                MessageBox(0, L"Font not initializated", L"Ok", MB_OK);
                return;
            case 3:
            {
                ImFontConfig fontConfig;
                fontConfig.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_NoHinting;
                fontConfig.SizePixels = 16.0f;

                io.Fonts->Clear();
                io.Fonts->AddFontDefault(&fontConfig);
            }
            }
        }
        avformat_network_init();
        loaded = true;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (msg.message != WM_QUIT) {

        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                continue;
        }

        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

        g_pd3dDevice->BeginScene();

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        //UpdateVideoFrame(g_pd3dDevice); // Обновление кадра видео
        ImGui::NewFrame();


        // Рендеринг объектов
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> distrib(0, 300);
        int random_y = distrib(gen);

        
        // Ваш рендеринг с эффектом
        ImGui::GetForegroundDrawList()->AddImage(
            (ImTextureID)Noise,
            ImVec2(0, 0),
            ImVec2(this->window_size.x, this->window_size.y + random_y),
            ImVec2(0, 0), ImVec2(1, 1), ImColor(128, 128, 128, 45)
        );

        
        //ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(this->window_size.x, 30), ImColor(255, 255, 255, 20));

        // Другие изображения
        
        
        ImGui::GetForegroundDrawList()->AddImage(
            (ImTextureID)Tv,
            ImVec2(0, 0),
            ImVec2(this->window_size.x, this->window_size.y),
            ImVec2(0, 0), ImVec2(1, 1), ImColor(128, 255, 128, 14)
        );
        

        ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(this->window_size.x, this->window_size.y), ImColor(0, 0, 0));

        /*
        ImGui::GetBackgroundDrawList()->AddImage(
            (ImTextureID)g_videoTexture,
            ImVec2(0, 0),
            ImVec2(this->window_size.x, this->window_size.y),
            ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 40)
        );
        */

        // Рендеринг меню
        g_menu.render(*this);

        ImGui::EndFrame();
        ImGui::Render();

         
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        g_pd3dDevice->EndScene();
        g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        fps.WaitForNextFrame();
    }

    // Освобождаем ресурсы
    this->unload_textures();

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    g_pd3dDevice->Release();
    // Выключаем тест обрезки после рендеринга
    g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

    pD3D->Release();
    UnregisterClass(_T("Window"), wc.hInstance);
}