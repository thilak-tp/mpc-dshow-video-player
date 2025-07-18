#include <windows.h>
#include <dshow.h>
#include <strsafe.h>
#include <atlbase.h> // For CComPtr
#include <debugapi.h>



#include "IMPCVideoRenderer.h"
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "quartz.lib")


//// CLSID for LAV Video Decoder: {EE30215D-164F-4A92-A4EB-BA2F1A1DFF42}
//DEFINE_GUID(CLSID_LAVVideoDecoder,
//    0xee30215d, 0x164f, 0x4a92, 0xa4, 0xeb, 0xba, 0x2f, 0x1a, 0x1d, 0xff, 0x42);

#define ID_BUTTON_PLAY  101
#define ID_BUTTON_PAUSE 102
#define ID_BUTTON_STOP  103

HWND hPlayBtn, hPauseBtn, hStopBtn;
HWND hMainWnd, MPCVideoWnd;

IGraphBuilder* pGraph = nullptr;
IMediaControl* pControl = nullptr;
IMediaEvent* pEvent = nullptr;
IVideoWindow* pVidWin = nullptr;
IBaseFilter* pMpcRenderer = nullptr;

LPCWSTR VIDEO_FILE = L"sample.mp4";

void CleanUp() {
    if (pVidWin) {
        pVidWin->put_Visible(OAFALSE);
        pVidWin->put_Owner(NULL);
        pVidWin->Release();
    }
    if (pControl) pControl->Release();
    if (pEvent) pEvent->Release();
    if (pGraph) pGraph->Release();
    if (pMpcRenderer) pMpcRenderer->Release();
    CoUninitialize();
}
HRESULT InitGraph(HWND hWnd) {
    
    CComPtr<IMPCVideoRenderer> pMPCVR;
    CComPtr<IMPCVideoRendererFilter> pMPCSettings;
    HRESULT hr;
    //CComPtr<IMPCVideoRenderer> pMPCVR;
    //CComPtr<IMPCVideoRendererFilter> pMPCSettings;
    CComPtr<IBaseFilter> pSource;
    // Step 1: COM Init
    hr = CoInitialize(NULL);
    if (FAILED(hr)) goto FAIL_STEP1;

    // Step 2: Create Filter Graph
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                          IID_IGraphBuilder, (void**)&pGraph);
    if (FAILED(hr)) goto FAIL_STEP2;

    // Step 3: Create MPC Video Renderer Filter
    CLSID clsidMpcRenderer;
    hr = CLSIDFromString(L"{71F080AA-8661-4093-B15E-4F6903E77D0A}", &clsidMpcRenderer);
    if (FAILED(hr)) goto FAIL_STEP3;
    // Using the default video renderer
    //hr = CoCreateInstance(CLSID_VideoRenderer, NULL, CLSCTX_INPROC_SERVER,IID_IBaseFilter, (void**)&pMpcRenderer);
   
    // Using the MPC-HC video renderer
    hr = CoCreateInstance(clsidMpcRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pMpcRenderer);
    if (FAILED(hr)) goto FAIL_STEP4;

    hr = pGraph->AddFilter(pMpcRenderer, L"MPC Video Renderer");
    if (FAILED(hr)) goto FAIL_STEP5;

    // Step 4: Render media file
    hr = pGraph->RenderFile(L"C:\\Users\\tpthi\\Downloads\\SampleVideo_1280x720_2mb.mp4", NULL);
    //hr = pGraph->RenderFile(L"C:\\Users\\tpthi\\Downloads\\file_example_AVI_1920_2_3MG.avi", NULL);
    //hr = pGraph->RenderFile(L"C:\\Users\\tpthi\\Downloads\\file_example_WMV_1920_9_3MB.wmv", NULL);
    
    /*hr = pGraph->AddSourceFilter(VIDEO_FILE, L"C:\\Users\\tpthi\\Downloads\\file_example_WMV_1920_9_3MB.wmv", &pSource);
    if (FAILED(hr)) {
        MessageBox(NULL, L"Failed to add source filter", L"Error", MB_OK);
        return hr;
    }*/



    if (FAILED(hr)) goto FAIL_STEP6;
    //wprintf(L"RenderFile failed with HRESULT 0x%08X\n", hr);

    // Step 5: Query interfaces
    pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
    pGraph->QueryInterface(IID_IMediaEvent, (void**)&pEvent);
    pGraph->QueryInterface(IID_IVideoWindow, (void**)&pVidWin);


    if (pVidWin) {
        RECT rc;
        GetClientRect(hWnd, &rc);
        pVidWin->put_Owner((OAHWND)hWnd);
        pVidWin->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);
        pVidWin->SetWindowPosition(10, 60, rc.right - 20, rc.bottom - 70);
        pVidWin->put_Visible(OATRUE);
    }
    // After obtaining the filter

//if (MPCVideoWnd && SUCCEEDED(pMpcRenderer->QueryInterface(__uuidof(IMPCVideoRenderer), (void**)&pMPCVR))) {
//    HRESULT winSet = pMPCVR->SetVideoWindow(MPCVideoWnd);
//    if (FAILED(winSet)) {
//        MessageBox(hWnd, L"SetVideoWindow failed", L"Error", MB_OK);
//    }
//}
//
//
//if (SUCCEEDED(pMpcRenderer->QueryInterface(__uuidof(IMPCVideoRendererFilter), (void**)&pMPCSettings))) {
//    RECT rc;
//    GetClientRect(MPCVideoWnd, &rc);  // Use child window
//    pMPCSettings->SetInt("VideoWindowLeft", 0);
//    pMPCSettings->SetInt("VideoWindowTop", 0);
//    pMPCSettings->SetInt("VideoWindowWidth", rc.right);
//    pMPCSettings->SetInt("VideoWindowHeight", rc.bottom);
//    pMPCSettings->SetBool("Show", TRUE);
//}
//
//SetForegroundWindow(hWnd);
//InvalidateRect(MPCVideoWnd, NULL, TRUE);
//UpdateWindow(MPCVideoWnd);



    
    /*pMpcRenderer->QueryInterface(__uuidof(IMPCVideoRenderer), (void**)&pMPC);
    pMPC->SetVideoWindow(hWnd);*/

    /*if (SUCCEEDED(pMpcRenderer->QueryInterface(__uuidof(IMPCVideoRenderer), (void**)&pMPCVR))) {
        HRESULT winSet = pMPCVR->SetVideoWindow(hWnd);
        if (FAILED(winSet)) {
            MessageBox(NULL, L"SetVideoWindow failed", L"Debug", MB_OK | MB_ICONERROR);
        }

    }

    
    if (SUCCEEDED(pMpcRenderer->QueryInterface(__uuidof(IMPCVideoRendererFilter), (void**)&pMPCSettings))) {
        RECT rc;
        GetClientRect(hWnd, &rc);
        pMPCSettings->SetInt("VideoWindowLeft", 10);
        pMPCSettings->SetInt("VideoWindowTop", 60);
        pMPCSettings->SetInt("VideoWindowWidth", rc.right - 20);
        pMPCSettings->SetInt("VideoWindowHeight", rc.bottom - 70);
        HRESULT showResult = pMPCSettings->SetBool("Show", TRUE);
        if (FAILED(showResult)) {
            MessageBox(NULL, L"Failed to show MPC renderer", L"Debug", MB_OK);
        }
        InvalidateRect(hWnd, NULL, TRUE);
        UpdateWindow(hWnd);

    }*/
    

    return S_OK;

FAIL_STEP1:
    MessageBox(NULL, L"Failed CoInitialize", L"Error", MB_OK | MB_ICONERROR);
    return hr;
FAIL_STEP2:
    MessageBox(NULL, L"Failed to create Filter Graph", L"Error", MB_OK | MB_ICONERROR);
    return hr;
FAIL_STEP3:
    MessageBox(NULL, L"Failed CLSIDFromString (invalid CLSID)", L"Error", MB_OK | MB_ICONERROR);
    return hr;
FAIL_STEP4:
    MessageBox(NULL, L"Failed to create MPC Renderer (probably not registered)", L"Error", MB_OK | MB_ICONERROR);
    return hr;
FAIL_STEP5:
    MessageBox(NULL, L"Failed to add MPC Renderer to graph", L"Error", MB_OK | MB_ICONERROR);
    return hr;
FAIL_STEP6:
    MessageBox(NULL, L"Failed to render media file (sample.mp4 not found or codec missing)", L"Error", MB_OK | MB_ICONERROR);
    return hr;
}
//IPin* GetPin(IBaseFilter* pFilter, PIN_DIRECTION dir) {
//    IEnumPins* pEnum = nullptr;
//    IPin* pPin = nullptr;
//    if (SUCCEEDED(pFilter->EnumPins(&pEnum))) {
//        while (pEnum->Next(1, &pPin, 0) == S_OK) {
//            PIN_DIRECTION thisDir;
//            pPin->QueryDirection(&thisDir);
//            if (thisDir == dir) {
//                pEnum->Release();
//                return pPin;
//            }
//            pPin->Release();
//        }
//        pEnum->Release();
//    }
//    return nullptr;
//}
//
//HRESULT InitGraph(HWND hWnd) {
//    HRESULT hr;
//    CComPtr<IMPCVideoRenderer> pMPCVR;
//    CComPtr<IMPCVideoRendererFilter> pMPCSettings;
//    CComPtr<IBaseFilter> pSource;
//
//    hr = CoInitialize(NULL);
//    if (FAILED(hr)) return hr;
//
//    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
//        IID_IGraphBuilder, (void**)&pGraph);
//    if (FAILED(hr)) return hr;
//
//    // Add MPC-HC Video Renderer
//    CLSID clsidMpcRenderer;
//    hr = CLSIDFromString(L"{71F080AA-8661-4093-B15E-4F6903E77D0A}", &clsidMpcRenderer);
//    if (FAILED(hr)) return hr;
//    
//    /*CComPtr<IBaseFilter> pDecoder;
//    hr = CoCreateInstance(CLSID_LAVVideoDecoder, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pDecoder);
//    if (FAILED(hr)) {
//        MessageBox(hWnd, L"Failed to create LAV Decoder", L"Error", MB_OK);
//        return hr;
//    }
//
//    hr = pGraph->AddFilter(pDecoder, L"LAV Video Decoder");
//    if (FAILED(hr)) return hr;*/
//
//    hr = CoCreateInstance(clsidMpcRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pMpcRenderer);
//    if (FAILED(hr)) return hr;
//
//    hr = pGraph->AddFilter(pMpcRenderer, L"MPC Video Renderer");
//    if (FAILED(hr)) return hr;
//
//    // Add source filter (async reader + splitter handled internally)
//    hr = pGraph->AddSourceFilter(L"C:\\Users\\tpthi\\Downloads\\SampleVideo_1280x720_2mb.mp4", L"Source", &pSource);
//    if (FAILED(hr)) return hr;
//
//    // Manually connect video output to MPC renderer
//    CComPtr<IPin> pOutPin = GetPin(pSource, PINDIR_OUTPUT);
//    CComPtr<IPin> pInPin = GetPin(pMpcRenderer, PINDIR_INPUT);
//
//    if (!pOutPin || !pInPin) {
//        MessageBox(hWnd, L"Failed to find pins", L"Error", MB_OK);
//        return E_FAIL;
//    }
//
//    hr = pGraph->Connect(pOutPin, pInPin);
//    if (FAILED(hr)) {
//        MessageBox(hWnd, L"Manual Connect failed", L"Error", MB_OK);
//        return hr;
//    }
//
//    // Set MPC render target
//    if (SUCCEEDED(pMpcRenderer->QueryInterface(__uuidof(IMPCVideoRenderer), (void**)&pMPCVR))) {
//        pMPCVR->SetVideoWindow(hWnd);
//    }
//
//    if (SUCCEEDED(pMpcRenderer->QueryInterface(__uuidof(IMPCVideoRendererFilter), (void**)&pMPCSettings))) {
//        RECT rc;
//        GetClientRect(hWnd, &rc);
//        pMPCSettings->SetInt("VideoWindowLeft", 10);
//        pMPCSettings->SetInt("VideoWindowTop", 60);
//        pMPCSettings->SetInt("VideoWindowWidth", rc.right - 20);
//        pMPCSettings->SetInt("VideoWindowHeight", rc.bottom - 70);
//        pMPCSettings->SetBool("Show", TRUE);
//    }
//
//    hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
//    hr = pGraph->QueryInterface(IID_IMediaEvent, (void**)&pEvent);
//
//    return S_OK;
//}
//

void ResizeVideoWindow(HWND hWnd) {
    if (pVidWin) {
        RECT rc;
        GetClientRect(hWnd, &rc);
        pVidWin->SetWindowPosition(10, 60, rc.right - 20, rc.bottom - 70);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        hPlayBtn = CreateWindow(L"BUTTON", L"Play", WS_CHILD | WS_VISIBLE,
            10, 10, 60, 30, hWnd, (HMENU)ID_BUTTON_PLAY, NULL, NULL);
        hPauseBtn = CreateWindow(L"BUTTON", L"Pause", WS_CHILD | WS_VISIBLE,
            80, 10, 60, 30, hWnd, (HMENU)ID_BUTTON_PAUSE, NULL, NULL);
        hStopBtn = CreateWindow(L"BUTTON", L"Stop", WS_CHILD | WS_VISIBLE,
            150, 10, 60, 30, hWnd, (HMENU)ID_BUTTON_STOP, NULL, NULL);
        
        MPCVideoWnd =  CreateWindow(
            L"STATIC", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            10, 60, 760, 480,
            hWnd, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BUTTON_PLAY:
            if (pControl) {
                HRESULT hr = pControl->Run();
             
            }
            break;

        case ID_BUTTON_PAUSE:
            if (pControl) pControl->Pause();
            break;
        case ID_BUTTON_STOP:
            if (pControl) pControl->Stop();
            break;
        }
        break;

    case WM_SIZE:
        ResizeVideoWindow(hWnd);
    
   
        if (MPCVideoWnd) {
            MoveWindow(MPCVideoWnd, 10, 60, LOWORD(lParam) - 20, HIWORD(lParam) - 70, TRUE);
        }
        


        break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        CleanUp();
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"VideoPlayerWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    hMainWnd = CreateWindow(wc.lpszClassName, L"MPC Video Player",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 800, 600,
        NULL, NULL, hInst, NULL);

    if (!hMainWnd) return 0;

    if (FAILED(InitGraph(hMainWnd))) {
        MessageBox(NULL, L"Failed to initialize DirectShow graph", L"Error", MB_OK);
        return -1;
    }

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}


//#include <windows.h>
//#include <dshow.h>
//#include <atlbase.h>
//#include "IMPCVideoRenderer.h"
//#pragma comment(lib, "strmiids.lib")
//#pragma comment(lib, "quartz.lib")
//
//#define ID_BUTTON_PLAY  101
//#define ID_BUTTON_PAUSE 102
//#define ID_BUTTON_STOP  103
//
//HWND hMainWnd, hPlayBtn, hPauseBtn, hStopBtn;
//
//IGraphBuilder* pGraph = nullptr;
//IMediaControl* pControl = nullptr;
//IMediaEvent* pEvent = nullptr;
//IBaseFilter* pMpcRenderer = nullptr;
//
//const LPCWSTR VIDEO_FILE = L"C:\\Users\\tpthi\\Downloads\\SampleVideo_1280x720_2mb.mp4";
//
//void CleanUp() {
//    if (pControl) pControl->Release();
//    if (pEvent) pEvent->Release();
//    if (pGraph) pGraph->Release();
//    if (pMpcRenderer) pMpcRenderer->Release();
//    CoUninitialize();
//}
//
//HRESULT InitGraph(HWND hWnd) {
//    HRESULT hr;
//    CComPtr<IMPCVideoRenderer> pMPCVR;
//    CComPtr<IMPCVideoRendererFilter> pMPCSettings;
//
//    hr = CoInitialize(NULL);
//    if (FAILED(hr)) return hr;
//
//    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);
//    if (FAILED(hr)) return hr;
//
//    CLSID clsidMpcRenderer;
//    hr = CLSIDFromString(L"{71F080AA-8661-4093-B15E-4F6903E77D0A}", &clsidMpcRenderer);
//    if (FAILED(hr)) return hr;
//    //hr = CoCreateInstance(CLSID_VideoRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pMpcRenderer);
//    hr = CoCreateInstance(clsidMpcRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pMpcRenderer);
//    if (FAILED(hr)) return hr;
//
//    hr = pGraph->AddFilter(pMpcRenderer, L"MPC Video Renderer");
//    if (FAILED(hr)) return hr;
//
//    // Tell the renderer where to draw
//    if (SUCCEEDED(pMpcRenderer->QueryInterface(__uuidof(IMPCVideoRenderer), (void**)&pMPCVR))) {
//        pMPCVR->SetVideoWindow(hWnd);
//    }
//
//    // Optional: set render window size
//    if (SUCCEEDED(pMpcRenderer->QueryInterface(__uuidof(IMPCVideoRendererFilter), (void**)&pMPCSettings))) {
//        RECT rc; GetClientRect(hWnd, &rc);
//        pMPCSettings->SetInt("VideoWindowLeft", 10);
//        pMPCSettings->SetInt("VideoWindowTop", 60);
//        pMPCSettings->SetInt("VideoWindowWidth", rc.right - 20);
//        pMPCSettings->SetInt("VideoWindowHeight", rc.bottom - 70);
//        pMPCSettings->SetBool("Show", TRUE);
//    }
//
//    hr = pGraph->RenderFile(VIDEO_FILE, NULL);
//    if (FAILED(hr)) return hr;
//
//    pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
//    pGraph->QueryInterface(IID_IMediaEvent, (void**)&pEvent);
//
//    return S_OK;
//}
//
//void ResizeMPCWindow(HWND hWnd) {
//    CComPtr<IMPCVideoRendererFilter> pMPCSettings;
//    if (pMpcRenderer && SUCCEEDED(pMpcRenderer->QueryInterface(__uuidof(IMPCVideoRendererFilter), (void**)&pMPCSettings))) {
//        RECT rc; GetClientRect(hWnd, &rc);
//        pMPCSettings->SetInt("VideoWindowWidth", rc.right - 20);
//        pMPCSettings->SetInt("VideoWindowHeight", rc.bottom - 70);
//    }
//}
//
//LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//    switch (msg) {
//    case WM_CREATE:
//        hPlayBtn = CreateWindow(L"BUTTON", L"Play", WS_CHILD | WS_VISIBLE, 10, 10, 60, 30, hWnd, (HMENU)ID_BUTTON_PLAY, NULL, NULL);
//        hPauseBtn = CreateWindow(L"BUTTON", L"Pause", WS_CHILD | WS_VISIBLE, 80, 10, 60, 30, hWnd, (HMENU)ID_BUTTON_PAUSE, NULL, NULL);
//        hStopBtn = CreateWindow(L"BUTTON", L"Stop", WS_CHILD | WS_VISIBLE, 150, 10, 60, 30, hWnd, (HMENU)ID_BUTTON_STOP, NULL, NULL);
//        break;
//
//    case WM_COMMAND:
//        switch (LOWORD(wParam)) {
//        case ID_BUTTON_PLAY: if (pControl) pControl->Run(); break;
//        case ID_BUTTON_PAUSE: if (pControl) pControl->Pause(); break;
//        case ID_BUTTON_STOP: if (pControl) pControl->Stop(); break;
//        }
//        break;
//
//    case WM_SIZE: ResizeMPCWindow(hWnd); break;
//    case WM_CLOSE: DestroyWindow(hWnd); break;
//    case WM_DESTROY: CleanUp(); PostQuitMessage(0); break;
//    default: return DefWindowProc(hWnd, msg, wParam, lParam);
//    }
//    return 0;
//}
//
//int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
//    WNDCLASS wc = {};
//    wc.lpfnWndProc = WndProc;
//    wc.hInstance = hInst;
//    wc.lpszClassName = L"MPCPlayerClass";
//    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
//    RegisterClass(&wc);
//
//    hMainWnd = CreateWindow(wc.lpszClassName, L"MPC Video Player", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
//        100, 100, 800, 600, NULL, NULL, hInst, NULL);
//
//    if (!hMainWnd) return 0;
//
//    if (FAILED(InitGraph(hMainWnd))) {
//        MessageBox(NULL, L"Failed to initialize DirectShow graph", L"Error", MB_OK | MB_ICONERROR);
//        return -1;
//    }
//
//    MSG msg = {};
//    while (GetMessage(&msg, NULL, 0, 0)) {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);
//    }
//    return (int)msg.wParam;
//}
