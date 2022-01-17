#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <gl/GL.h>
#include <time.h>
#include <stdlib.h>
#include <audioclient.h>
#include <mmdeviceapi.h>

#include "ext/glext.h"
#include "ext/wglext.h"
#include "opengl_functions.h"

#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"

#include "defines.h"
#include "maths.h"
#include "platform.h"

#include "audio.h"
#include "game.h"

#include "audio.cpp"
#include "game.cpp"

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
#define REFTIMES_PER_SEC  10000000

static Platform global_platform = {};

FileString Win32ReadFileIntoString(const char *filename){
    FileString file = {};
    HANDLE file_handle = CreateFileA((LPCSTR)filename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(file_handle == INVALID_HANDLE_VALUE){
        OutputDebugString("Failed to open file. Filename is probably wrong\n");
        exit(1);
    }
    DWORD file_size = GetFileSize(file_handle, NULL);
    char *file_data = (char *)VirtualAlloc(0, file_size + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    DWORD bytes_read;
    ReadFile(file_handle, file_data, file_size, &bytes_read, 0);
    if(bytes_read != file_size){
        OutputDebugString("Failed to read file properly.");
        exit(1);
    }
    file.data = file_data;
    file.size = (u32)file_size;
    return file;
}

File Win32ReadFile(const char *filename){
    File file = {};
    HANDLE file_handle = CreateFileA((LPCSTR)filename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(file_handle == INVALID_HANDLE_VALUE){
        OutputDebugString("Failed to open file. Filename is probably wrong.");
        exit(1);
    }
    DWORD file_size = GetFileSize(file_handle, NULL);
    void *file_data = (void *)VirtualAlloc(0, file_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    DWORD bytes_read;
    ReadFile(file_handle, file_data, file_size, &bytes_read, 0);
    if(bytes_read != file_size){
        OutputDebugString("Failed to read file properly.");
        exit(1);
    }
    file.data = file_data;
    file.size = (u32)file_size;
    return file;
}

void Win32DisplayError(const char *title, const char *message){
    MessageBox(0, message, title, MB_ICONERROR | MB_OK);
}

struct Win32Sound{
    IAudioRenderClient *render_client;
    IAudioClient *audio_client;
    u32 samples_per_second;
    u32 buffer_size;
    u32 buffer_duration;
    BYTE *data;
};

void Win32InitWASAPI(Win32Sound *sound){
    //Create wasapi object
    HRESULT hr;
    IMMDeviceEnumerator *enumerator = 0;
    hr = CoInitializeEx(NULL, COINIT_SPEED_OVER_MEMORY);
    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&enumerator);
    if(enumerator){
        IMMDevice *device;
        enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
        if(device){
            IAudioClient *audio_client;
            device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void **)&audio_client);
            if(audio_client){
                REFERENCE_TIME sound_duration = REFTIMES_PER_SEC * 2;
                WAVEFORMATEX wave_format;
                wave_format.wFormatTag = WAVE_FORMAT_PCM;
                wave_format.nChannels = 2;
                wave_format.nSamplesPerSec = 44100;
                wave_format.wBitsPerSample = 16;
                wave_format.nBlockAlign = (2 * wave_format.wBitsPerSample) / 8;
                wave_format.nAvgBytesPerSec = 44100 * wave_format.nBlockAlign;
                wave_format.cbSize = 0;
                hr =  audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,  sound_duration, 0, &wave_format, 0);
                if(hr == S_OK){
                    IAudioRenderClient *render_client = 0;
                    hr = audio_client->GetService(IID_IAudioRenderClient, (void **)&render_client);
                    if(hr == S_OK){
                        u32 buffer_size;
                        audio_client->GetBufferSize(&buffer_size);
                        if(buffer_size){
                            sound->samples_per_second = 44100;
                            sound->buffer_size = buffer_size;
                            sound->render_client = render_client;
                            sound->audio_client = audio_client;
                        }else{
                            printf("sdfljslkfjsl");
                        }
                    }else{
                        Win32DisplayError("Audio Error", "Failed to get audio render client");
                    }
                }else{
                    Win32DisplayError("Audio Error", "Failed to initialise audio client");
                }
            }else{
                Win32DisplayError("Audio Error", "Failed to activate audio client");
            }
        }else{
            Win32DisplayError("Audio Error", "Failed to initialise WASAPI IMMDevice");
        }
    }else{
        Win32DisplayError("Audio Error", "Failed to initialise WASAPI vtable");
    }
}

void Win32FillSoundBuffer(u32 samples_to_write, i16 *samples, Win32Sound sound){
    BYTE *data;
    HRESULT hr = sound.render_client->GetBuffer(samples_to_write, &data);
    if(data){
        i16 *sample_out = (i16 *)data;
        i16 *sample_in  = samples;
        for(int i = 0; i < samples_to_write; i++){
            *sample_out++ = *sample_in++; //left
            *sample_out++ = *sample_in++; //right
        }
    }
    hr = sound.render_client->ReleaseBuffer(samples_to_write, 0);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param){
    if(u_msg == WM_DESTROY){
        global_platform.running = false;
    } 
    else if(u_msg == WM_KEYDOWN || u_msg == WM_SYSKEYDOWN || u_msg == WM_KEYUP || u_msg == WM_SYSKEYUP){
        u32 key_code = (u32)w_param;
        b32 was_down = ((l_param & (1 << 30)) != 0);
        b32 is_down  = ((l_param & (1 << 31)) == 0);
        
        u32 pressed_key = KEY_NONE;
        if(key_code == 'W'){
            pressed_key = KEY_W;
        } else if(key_code == 'A'){
            pressed_key = KEY_A;
        } else if(key_code == 'S'){
            pressed_key = KEY_S;
        } else if(key_code == 'D'){
            pressed_key = KEY_D;
        } else if(key_code == VK_SPACE){
            pressed_key = KEY_SPACE;
        }
        
        if(was_down != is_down){
            //char log[256];
            //snprintf(log, 256, "%d", is_down);
            //OutputDebugString(log);
            global_platform.key_down[pressed_key] = is_down;
        }
    }
    else if(u_msg == WM_SIZE){
        RECT client_rect;
        GetClientRect(hwnd, &client_rect);
        global_platform.window_width = client_rect.right-client_rect.left;
        global_platform.window_height = client_rect.bottom-client_rect.top;
    }
    return DefWindowProc(hwnd, u_msg, w_param, l_param);
}

int WINAPI wWinMain(HINSTANCE h_instance, HINSTANCE h_prev_instance, PWSTR p_cmd_line, int n_cmd_show){
    srand(time(NULL));
    global_platform.initial_window_width  = 1280;
    global_platform.initial_window_height = 720;
    global_platform.running = true;
    global_platform.OSReadFileToString = Win32ReadFileIntoString;
    global_platform.OSReadFile = Win32ReadFile;
    global_platform.OSDisplayError = Win32DisplayError; 
    
    /* Window Creation */
    LPCSTR class_name  = "Asteroids Window Class";
    LPCSTR window_name = "Asteroids !"; 
    WNDCLASSA window_class = {};
    window_class.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc   = WindowProc;
    window_class.hInstance     = h_instance;
    window_class.lpszClassName = class_name;
    RegisterClassA(&window_class);
    HWND hwnd = CreateWindowA(class_name, window_name, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, global_platform.initial_window_width, global_platform.initial_window_height, NULL, NULL, h_instance, NULL);
    if(hwnd == NULL){
        OutputDebugString("Failed to load window\n");
        return 1;
    }
    ShowWindow(hwnd, SW_SHOW);
    
    
    /* OpenGL Initialisation */
    HDC dc    = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = 32;
    pfd.cDepthBits   = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType   = PFD_MAIN_PLANE;
    i32 pixel_format = ChoosePixelFormat(dc, &pfd);
    if(!SetPixelFormat(dc, pixel_format, &pfd)){
        OutputDebugString("Failed to create pixel format for OpenGL.\n");
        return 1;
    }
    HGLRC hglrc = wglCreateContext(dc); 
    if(!wglMakeCurrent(dc, hglrc)){
        OutputDebugString("Failed to make OpenGL context current.\n");
        return 1;
    }
    LoadOpenGLFunctions();
    
    const i32 attrib_list[] =
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        0, // End
    };
    u32 num_formats;
    if(!wglChoosePixelFormatARB(dc, attrib_list, NULL, 1, &pixel_format, &num_formats)){
        OutputDebugString("Failed to create gl pixel format.");
        return 1;
    }
    const i32 context_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    HGLRC opengl_context;
    if(!(opengl_context = wglCreateContextAttribsARB(dc, hglrc, context_attribs))){
        OutputDebugString("Failed to create OpenGl context attribs");
        return 1;
    }
    wglMakeCurrent(dc, 0);
    wglDeleteContext(hglrc);
    wglMakeCurrent(dc, opengl_context);
    wglSwapIntervalEXT(1);
    
    /* Timer Initialisation */
    b32 sleep_is_granular = (timeBeginPeriod(1) == TIMERR_NOERROR);
    LARGE_INTEGER performance_frequency;
    QueryPerformanceFrequency(&performance_frequency);
    f32 target_ms = 1.0f / 60.0f; 
    global_platform.dt = target_ms;
    
    /* Audio Initialisation */
    Win32Sound win32_sound = {};
    Win32InitWASAPI(&win32_sound);
    global_platform.samples = (i16 *)VirtualAlloc(0, win32_sound.samples_per_second * sizeof(i16) * 2, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    global_platform.samples_per_second = win32_sound.samples_per_second;
    u32 running_sample_index = 0;
    win32_sound.audio_client->Start();
    while(global_platform.running){
        LARGE_INTEGER start;
        QueryPerformanceCounter(&start);
        MSG msg = {};
        while(PeekMessageA(&msg, hwnd, NULL, NULL, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        {
            UINT32 padding_frames;
            if(SUCCEEDED(win32_sound.audio_client->GetCurrentPadding(&padding_frames))){
                u32 latency_frame_count = win32_sound.samples_per_second / 45;
                global_platform.samples_to_write = latency_frame_count - padding_frames;
                if(global_platform.samples_to_write > latency_frame_count){
                    global_platform.samples_to_write = latency_frame_count;
                }
            }
            i16 *current_sample = global_platform.samples;
            for(int i = 0; i < global_platform.samples_to_write; i++){
                *current_sample++ = 0;
                *current_sample++ = 0;
            }
        }
        GameUpdateAndRender(&global_platform);
        Win32FillSoundBuffer(global_platform.samples_to_write, global_platform.samples, win32_sound);
        
        LARGE_INTEGER end;
        QueryPerformanceCounter(&end);
        f32 ms_per_frame = (f32)(end.QuadPart - start.QuadPart) / performance_frequency.QuadPart;
        char log[256];
        snprintf(log, 256, "%f\n", ms_per_frame);
        OutputDebugString(log);
        if(ms_per_frame < target_ms){
            while(ms_per_frame < target_ms){
                if(sleep_is_granular){
                    f32 ms_to_sleep = target_ms - ms_per_frame;
                    Sleep(ms_to_sleep * 1000);
                }
                LARGE_INTEGER sleep_end;
                QueryPerformanceCounter(&sleep_end);
                ms_per_frame += (sleep_end.QuadPart - ms_per_frame) / performance_frequency.QuadPart;
            }
        }else{
            OutputDebugString("Missed frame!");
        }
        /*
        LARGE_INTEGER actual_end;
        QueryPerformanceCounter(&actual_end);
        char log[256];
        snprintf(log, 256, "%f\n", (f32)(actual_end.QuadPart - start.QuadPart) / performance_frequency.QuadPart);
        OutputDebugString(log);
*/
        
        wglSwapLayerBuffers(dc, WGL_SWAP_MAIN_PLANE);
    }
    return 0;
}