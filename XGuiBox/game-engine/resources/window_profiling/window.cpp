#define STB_IMAGE_IMPLEMENTATION

#include "window.h"
#include <sstream>
#include <fstream>
#include "../../misc/instruments.h"
#include "../../xguibox/xgui.h"
#include "../../main/menu.h"
#include "../imgui_resource/imgui_freetype.h"
#include "../byte/planet_menu.h"
#include "../byte/noise.h"
#include "../byte/RGB_LINES.h"
#include "../byte/tv.h"
#include <algorithm>
#include <objbase.h> 

#include <d3d11.h>
#include <d3dcompiler.h>
#include "../imgui_resource/stb_image.h"

extern "C" 
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

#include <ctime>
#include <random>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <DirectXMath.h>
#pragma comment(lib, "C:\\Users\\sasha\\OneDrive\\Documents\\ffmpeg lib\\ffmpeg-4.3.1-win64-dev\\lib\\avcodec.lib")
#pragma comment(lib, "C:\\Users\\sasha\\OneDrive\\Documents\\ffmpeg lib\\ffmpeg-4.3.1-win64-dev\\lib\\avformat.lib")
#pragma comment(lib, "C:\\Users\\sasha\\OneDrive\\Documents\\ffmpeg lib\\ffmpeg-4.3.1-win64-dev\\lib\\avutil.lib")
#pragma comment(lib, "C:\\Users\\sasha\\OneDrive\\Documents\\ffmpeg lib\\ffmpeg-4.3.1-win64-dev\\lib\\swscale.lib")

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "bcrypt.lib")

#include "../byte/nanosvg.h"
#include "../byte/nanosvgrast.h"
#include <shellapi.h>

#define D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD 0
#define D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS 0
#define D3D10_ERROR_FILE_NOT_FOUND 0

// FFmpeg структуры
AVFormatContext* formatContext = nullptr;
AVCodecContext* codecContext = nullptr;
AVFrame* frame = nullptr;
AVFrame* frameRGBA = nullptr;
SwsContext* swsContext = nullptr;

// Размеры видео
int videoWidth = 0, videoHeight = 0;

// DirectX текстура
ID3D11Texture2D* g_pTexture = nullptr;

bool SeekToStart() {
    if (av_seek_frame(formatContext, -1, 0, AVSEEK_FLAG_BACKWARD) < 0) {
        std::cerr << "Ошибка: не удалось перемотать видео на начало.\n";
        return false;
    }

    // Сбрасываем декодер, чтобы начать заново
    avcodec_flush_buffers(codecContext);
    return true;
}

// Инициализация FFmpeg
bool InitFFmpeg(const char* filename) {

    avformat_network_init();

    // Открытие файла
    if (avformat_open_input(&formatContext, filename, nullptr, nullptr) != 0) {
        std::cerr << "Ошибка: не удалось открыть файл.\n";
        return false;
    }

    // Поиск потоков
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cerr << "Ошибка: не удалось найти потоки.\n";
        return false;
    }

    // Поиск видеопотока
    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        std::cerr << "Ошибка: видеопоток не найден.\n";
        return false;
    }

    // Настройка кодека
    const AVCodec* codec = avcodec_find_decoder(formatContext->streams[videoStreamIndex]->codecpar->codec_id);
    if (!codec) {
        std::cerr << "Ошибка: кодек не найден.\n";
        return false;
    }

    codecContext = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(codecContext, formatContext->streams[videoStreamIndex]->codecpar) < 0) {
        std::cerr << "Ошибка: не удалось настроить контекст кодека.\n";
        return false;
    }

    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        std::cerr << "Ошибка: не удалось открыть кодек.\n";
        return false;
    }

    // Получение размеров видео
    videoWidth = codecContext->width;
    videoHeight = codecContext->height;

    // Инициализация структур для кадров
    frame = av_frame_alloc();
    frameRGBA = av_frame_alloc();

    // Настройка SWS контекста
    swsContext = sws_getContext(videoWidth, videoHeight, codecContext->pix_fmt,
        videoWidth, videoHeight, AV_PIX_FMT_RGBA,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    return true;
}

// Глобальные переменные для контроля времени
std::chrono::steady_clock::time_point lastFrameTime = std::chrono::steady_clock::now();
const double targetFrameTime = 1.0 / 35.0; // 30 кадров в секунду

bool UpdateTexture(ID3D11Device* device, ID3D11DeviceContext* context)
{
    using namespace std::chrono;

    // Рассчитать время, прошедшее с последнего обновления
    steady_clock::time_point currentTime = steady_clock::now();
    double elapsedTime = duration<double>(currentTime - lastFrameTime).count();

    // Если с момента последнего кадра прошло недостаточно времени, выходим
    if (elapsedTime < targetFrameTime) {
        return true; // Пропускаем обновление
    }

    // Обновляем время последнего кадра
    lastFrameTime = currentTime;

    AVPacket packet;

    // Читаем кадры
    while (true) {
        if (av_read_frame(formatContext, &packet) < 0) {
            // Если достигнут конец видео, перемотать на начало
            if (av_seek_frame(formatContext, -1, 0, AVSEEK_FLAG_BACKWARD) < 0) {
                std::cerr << "Ошибка: не удалось перемотать видео на начало.\n";
                return false;
            }
            avcodec_flush_buffers(codecContext); // Сброс буферов декодера
            continue;
        }

        if (packet.stream_index == formatContext->streams[0]->index) {
            if (avcodec_send_packet(codecContext, &packet) == 0) {
                if (avcodec_receive_frame(codecContext, frame) == 0) {
                    // Конвертация в формат RGBA
                    uint8_t* dest[4] = { nullptr };
                    int destLinesize[4] = { 0 };
                    av_image_alloc(dest, destLinesize, videoWidth, videoHeight, AV_PIX_FMT_RGBA, 1);

                    sws_scale(
                        swsContext,
                        frame->data,
                        frame->linesize,
                        0,
                        videoHeight,
                        dest,
                        destLinesize
                    );

                    // Описание текстуры
                    D3D11_TEXTURE2D_DESC desc = {};
                    desc.Width = videoWidth;
                    desc.Height = videoHeight;
                    desc.MipLevels = 1;
                    desc.ArraySize = 1;
                    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                    desc.SampleDesc.Count = 1;
                    desc.Usage = D3D11_USAGE_DYNAMIC;
                    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

                    // Данные текстуры
                    D3D11_SUBRESOURCE_DATA initData = {};
                    initData.pSysMem = dest[0];
                    initData.SysMemPitch = destLinesize[0];

                    // Освобождение старой текстуры и создание новой
                    if (g_pTexture) g_pTexture->Release();
                    if (FAILED(device->CreateTexture2D(&desc, &initData, &g_pTexture))) {
                        std::cerr << "Ошибка: не удалось создать текстуру.\n";
                        av_freep(&dest[0]);
                        av_packet_unref(&packet);
                        return false;
                    }

                    // Освобождение старого представления текстуры и создание нового
                    if (g_window.g_pTextureView) g_window.g_pTextureView->Release();
                    if (FAILED(device->CreateShaderResourceView(g_pTexture, nullptr, &g_window.g_pTextureView))) {
                        std::cerr << "Ошибка: не удалось создать представление текстуры.\n";
                        av_freep(&dest[0]);
                        av_packet_unref(&packet);
                        return false;
                    }

                    // Освобождение ресурсов кадра
                    av_freep(&dest[0]);
                    av_packet_unref(&packet);
                    return true; // Успешное обновление текстуры
                }
            }
        }

        av_packet_unref(&packet);
    }
}

// Очистка ресурсов
void CleanupFFmpeg() {
    if (frame) av_frame_free(&frame);
    if (frameRGBA) av_frame_free(&frameRGBA);
    if (codecContext) avcodec_free_context(&codecContext);
    if (formatContext) avformat_close_input(&formatContext);
    if (swsContext) sws_freeContext(swsContext);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool isPaused = false;

void PauseRendering(bool pause) {
    isPaused = pause;
}

bool LoadTextureFromFile(ID3D11Device* device, const char* filename, ID3D11ShaderResourceView** textureSRV) {
    // Загружаем изображение через stb_image
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha); // Загружаем изображение с альфа-каналом
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return false;
    }

    // Создаем структуру для описания текстуры
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Используем формат с альфа-каналом
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

    // Заполняем данные для текстуры
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = data;
    initData.SysMemPitch = width * 4; // 4 байта на пиксель (RGBA)
    initData.SysMemSlicePitch = width * height * 4;

    // Создаем текстуру
    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = device->CreateTexture2D(&textureDesc, &initData, &texture);
    if (FAILED(hr)) {
        std::cerr << "Failed to create texture from data!" << std::endl;
        stbi_image_free(data);
        return false;
    }

    // Создаем Shader Resource View для текстуры
    hr = device->CreateShaderResourceView(texture, nullptr, textureSRV);
    if (FAILED(hr)) {
        std::cerr << "Failed to create Shader Resource View!" << std::endl;
        texture->Release();
        stbi_image_free(data);
        return false;
    }

    // Освобождаем память, занятую изображением
    texture->Release();
    stbi_image_free(data);

    return true;
}

bool LoadTextureFromMemory(ID3D11Device* device, const void* data, size_t size, ID3D11ShaderResourceView** textureSRV) {
    // Загружаем изображение из памяти через stb_image
    int width, height, channels;
    unsigned char* imageData = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(data), static_cast<int>(size), &width, &height, &channels, STBI_rgb_alpha);
    if (!imageData) {
        std::cerr << "Failed to load texture from memory!" << std::endl;
        return false;
    }

    // Создаем структуру для описания текстуры
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Используем формат с альфа-каналом
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

    // Заполняем данные для текстуры
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = imageData;
    initData.SysMemPitch = width * 4; // 4 байта на пиксель (RGBA)
    initData.SysMemSlicePitch = width * height * 4;

    // Создаем текстуру
    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = device->CreateTexture2D(&textureDesc, &initData, &texture);
    if (FAILED(hr)) {
        std::cerr << "Failed to create texture from data!" << std::endl;
        stbi_image_free(imageData);
        return false;
    }

    // Создаем Shader Resource View для текстуры
    hr = device->CreateShaderResourceView(texture, nullptr, textureSRV);
    if (FAILED(hr)) {
        std::cerr << "Failed to create Shader Resource View!" << std::endl;
        texture->Release();
        stbi_image_free(imageData);
        return false;
    }

    // Освобождаем память, занятую изображением
    texture->Release();
    stbi_image_free(imageData);

    return true;
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

// Обновленный код для загрузки текстур
void window_profiling::load_textures(ID3D11Device* device, ID3D11DeviceContext* context) {
    // Инициализация видео

    // Загрузка текстур из памяти
    LoadTextureFromMemory(g_pd3dDevice, &planet_menu, sizeof(planet_menu), &Logotype);
    LoadTextureFromMemory(g_pd3dDevice, &noise, sizeof(noise), &Noise);
    LoadTextureFromMemory(g_pd3dDevice, &tv, sizeof(tv), &Tv);
    LoadTextureFromMemory(g_pd3dDevice, &RGB_LINES, sizeof(RGB_LINES), &RGB);


    // Загрузка текстур стран
    for (int i = 0; i < ARRAYSIZE(countries_file_name); i++) {
        if (countries_file_name[i]) {
            LoadTextureFromFile(device, countries_file_name[i], &countries[i]);
        }
    }

    LoadTextureFromFile(device, "noise_add.png", &Additional_Noise);
    LoadTextureFromFile(device, "vhs.jpg", &Color_Diss);
    LoadTextureFromFile(device, "logo.png", &Logo);
    LoadTextureFromFile(device, "grid.png", &Grid);

}

void window_profiling::unload_textures()
{
    if (Tv)                 Tv->Release();
    if (Noise)              Noise->Release();
    if (Logotype)           Logotype->Release();
    if (RGB)                RGB->Release();


    for (int i = 0; i < ARRAYSIZE(countries_file_name); i++)
    {
        if (countries[i]) countries[i]->Release();
    }

}
void change_resolution(int width, int height)
{
    g_window.g_pd3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr); // Удаляем рендер-таргеты
    g_window.g_pd3dDeviceContext->Flush(); // Завершаем команды GPU

    if (g_window.g_pRenderTargetView) {
        g_window.g_pRenderTargetView->Release();
        g_window.g_pRenderTargetView = nullptr;
    }

    HRESULT hr = g_window.g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_window.g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    if (SUCCEEDED(hr)) {
        hr = g_window.g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_window.g_pRenderTargetView);
        pBackBuffer->Release();
    }

    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<FLOAT>(width);
    vp.Height = static_cast<FLOAT>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_window.g_pd3dDeviceContext->RSSetViewports(1, &vp);



}
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static POINTS ptsBegin;

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {

    case WM_APP + 1:  // Обработка клика по иконке
        if (lParam == WM_LBUTTONDOWN)  // Левый клик
        {

        }
        break;

    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE) {
            // Окно теряет фокус — выйти из полноэкранного режима
            BOOL isFullscreen;
            g_window.g_pSwapChain->GetFullscreenState(&isFullscreen, nullptr);
            if (isFullscreen) {
                g_window.g_pSwapChain->SetFullscreenState(FALSE, nullptr);
            }
        }
        else {
            // Окно восстанавливает фокус — вернуться в полноэкранный режим
            BOOL isFullscreen;
            g_window.g_pSwapChain->GetFullscreenState(&isFullscreen, nullptr);
            if (!isFullscreen) {
                g_window.g_pSwapChain->SetFullscreenState(TRUE, nullptr);
            }
        }
        break;

    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED) {
            // Окно сворачивается — выйти из полноэкранного режима
            BOOL isFullscreen;
            g_window.g_pSwapChain->GetFullscreenState(&isFullscreen, nullptr);
            if (isFullscreen) {
                g_window.g_pSwapChain->SetFullscreenState(FALSE, nullptr);
            }
        }
        else if (wParam == SIZE_RESTORED) {
            // Окно восстанавливается — вернуться в полноэкранный режим
            BOOL isFullscreen;
            g_window.g_pSwapChain->GetFullscreenState(&isFullscreen, nullptr);
            if (!isFullscreen) {
                g_window.g_pSwapChain->SetFullscreenState(TRUE, nullptr);
            }
        }
        break;

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

void ApplyVHSLinesEffect(ImDrawList* drawList, const ImVec2& screenSize) {
    const int numLines = 50;  // Количество линий
    const float lineWidth = 2.0f;  // Ширина линий
    const float minLineHeight = 1.0f;  // Минимальная высота линии
    const float maxLineHeight = 5.0f;  // Максимальная высота линии

    // Генерация случайных линий
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(0, static_cast<int>(screenSize.y));

    for (int i = 0; i < numLines; ++i) {
        // Генерация случайных вертикальных позиций и высоты линии
        float yPos = distrib(gen);
        float lineHeight = minLineHeight + static_cast<float>(rand() % static_cast<int>(maxLineHeight - minLineHeight));

        // Генерация случайной прозрачности
        float alpha = 0.2f + static_cast<float>(rand() % 100) / 500.0f;  // Прозрачность от 0.2 до 0.4

        // Рисуем линии
        drawList->AddLine(
            ImVec2(0, yPos),
            ImVec2(screenSize.x, yPos + lineHeight),
            ImColor(10, 10, 10, static_cast<int>(255 * alpha)),
            lineWidth
        );
    }
}

void ApplyVHSLinesEffect2(ImDrawList* drawList, const ImVec2& screenSize) {
    const int numLines = 100;  // Количество линий
    const float lineWidth = 1.0f;  // Ширина линий
    const float minLineHeight = 1.0f;  // Минимальная высота линии
    const float maxLineHeight = 5.0f;  // Максимальная высота линии

    for (int i = 0; i < numLines; ++i) {
        // Генерация случайных вертикальных позиций и высоты линии
        float yPos = (screenSize.y / numLines) * i;
        float lineHeight = minLineHeight;

        // Рисуем линии
        drawList->AddLine(
            ImVec2(0, yPos),
            ImVec2(screenSize.x, yPos + lineHeight),
            ImColor(0, 0, 0, static_cast<int>(70)),
            lineWidth
        );
    }
}
HRESULT CompileShaderFromFileManual(const wchar_t* fileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** blobOut)
{
    // Çàãðóçêà ôàéëà øåéäåðà
    std::ifstream shaderFile(fileName, std::ios::binary);
    if (!shaderFile.is_open()) {
        std::wstring errorMessage = L"Failed to open shader file: ";
        errorMessage += fileName;
        MessageBox(NULL, errorMessage.c_str(), L"Shader Error", MB_ICONERROR);
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    // ×òåíèå ôàéëà â áóôåð
    std::vector<char> shaderData((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
    shaderFile.close();

    // Êîìïèëÿöèÿ øåéäåðà
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompile(
        shaderData.data(),
        shaderData.size(),
        nullptr,        // Èìÿ ôàéëà (îïöèîíàëüíî)
        nullptr,        // Ìàêðîñû
        nullptr,        // Âêëþ÷åíèÿ
        entryPoint,     // Òî÷êà âõîäà
        shaderModel,    // Ìîäåëü øåéäåðà
        0,              // Ôëàãè êîìïèëÿöèè
        0,              // Ôëàãè ýôôåêòîâ
        blobOut,        // Âûõîäíîé blob
        &errorBlob      // Blob ñ îøèáêàìè
    );

    if (FAILED(hr)) {
        std::wstring errorMessage = L"Shader compile failed. Error: ";
        if (errorBlob) {
            std::string errorStr((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize());
            errorMessage += std::wstring(errorStr.begin(), errorStr.end());  // Ïðåîáðàçóåì std::string â std::wstring
            errorBlob->Release();
        }
        else {
            errorMessage += L"Unknown error";
        }
        MessageBox(NULL, errorMessage.c_str(), L"Shader Error", MB_ICONERROR);
        return hr;
    }

    if (errorBlob) errorBlob->Release();
    return hr;
}

void window_profiling::create_window()
{

    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Window"), NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindow(_T("Window"), _T("Defcon"), WS_OVERLAPPEDWINDOW, 0, 0, this->window_size.x, this->window_size.y, NULL, NULL, wc.hInstance, NULL);

    UINT numerator, denominator;

    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = this->window_size.x;
    swapChainDesc.BufferDesc.Height = this->window_size.y;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 144;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

    UINT createDeviceFlags = 0;
#if defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel[] = 
    {
        D3D_FEATURE_LEVEL_10_0  // Уровень 10.0
    };


    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &g_pSwapChain,
        &g_pd3dDevice,
        featureLevel,
        &g_pd3dDeviceContext
    );


    if (FAILED(hr)) {
        std::cerr << "Failed to create DirectX 11 device and swap chain!" << std::endl;
        return;
    }

    // Создание рендер-таргета
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    g_pSwapChain->Present(1, 0); // 1 - Включает ожидание вертикального синхроимпульса

    pBackBuffer->Release();

    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

    // Установка вьюпорта
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(this->window_size.x);
    viewport.Height = static_cast<float>(this->window_size.y);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    g_pd3dDeviceContext->RSSetViewports(1, &viewport);

    g_menu.change_res_x = this->window_size.x;
    g_menu.change_res_y = this->window_size.y;

    // Инициализация ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ImFontConfig cfg;
    cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;

    static bool loaded = false;

    if (!loaded)
    {
        this->load_textures(g_pd3dDevice, g_pd3dDeviceContext);
        for (int i = 0; i < g_xgui.fonts.size(); i++)
        {
            ImGuiIO& io = ImGui::GetIO();
            switch (g_xgui.fonts[i].font_mode)
            {
            case 0:
            {
                if (i == 9 || i == 9)
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

    BOOL isFullscreen;
    g_window.g_pSwapChain->GetFullscreenState(&isFullscreen, nullptr);
    if (!isFullscreen) {
        g_window.g_pSwapChain->SetFullscreenState(TRUE, nullptr);
    }

    InitFFmpeg("123.mkv");

    while (msg.message != WM_QUIT) 
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                continue;
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> distrib(0, 300);
        int random_y = distrib(gen);

        FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        // Очистка экрана
        g_pd3dDeviceContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);

        // Начало нового кадра ImGui
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        UpdateTexture(g_pd3dDevice, g_pd3dDeviceContext);
        ImGui::NewFrame();

        // Рендеринг интерфейса
        g_menu.render(*this);

        ApplyVHSLinesEffect(ImGui::GetForegroundDrawList(), ImVec2(window_size.x, window_size.y));

        ID3D11Texture2D* texture;

        ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(window_size.x, window_size.y), ImColor(30, 30, 255, 8));

        ImGui::GetForegroundDrawList()->AddImage(
            (ImTextureID)g_window.Tv,
            ImVec2(0, 0),
            ImVec2(this->window_size.x, this->window_size.y),
            ImVec2(0, 0), ImVec2(1, 1), ImColor(100, 200, 150, 9)
        );

        ImGui::GetForegroundDrawList()->AddImage(
            (ImTextureID)Noise,
            ImVec2(0, 0),
            ImVec2(this->window_size.x, this->window_size.y + random_y),
            ImVec2(0, 0), ImVec2(1, 1), ImColor(128, 128, 128, 45)
        );

        ImGui::GetForegroundDrawList()->AddImage(
            (ImTextureID)Color_Diss,
            ImVec2(0, 0),
            ImVec2(this->window_size.x, this->window_size.y),
            ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 7)
        );

        ImGui::GetForegroundDrawList()->AddImage(
            (ImTextureID)Additional_Noise,
            ImVec2(0, 0),
            ImVec2(this->window_size.x, this->window_size.y),
            ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 5)
        );

        ImGui::EndFrame();
        ImGui::Render();

        // Рендеринг ImGui
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


        // Обновление экрана
        g_pSwapChain->Present(1, 0);


    }

    // Освобождение ресурсов
    this->unload_textures();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    g_pRenderTargetView->Release();
    g_pSwapChain->Release();
    g_pd3dDeviceContext->Release();
    g_pd3dDevice->Release();
    UnregisterClass(_T("Window"), wc.hInstance);
}
