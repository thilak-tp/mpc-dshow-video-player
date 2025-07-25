// Implicit header files
#include <windows.h>
#include <dshow.h>
#include <atlbase.h> // For CComPtr


// Explicit header files
#include "error_logger.h"

// Button IDs
#define ID_BUTTON_PLAY  101
#define ID_BUTTON_PAUSE 102
#define ID_BUTTON_STOP  103

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "quartz.lib")

// Global variables
HINSTANCE hInst;
HWND hPlayBtn, hPauseBtn, hStopBtn;
HWND hMainWnd, MPCVideoWnd;

IGraphBuilder* pGraph = nullptr;
IMediaControl* pControl = nullptr;
IMediaEvent* pEvent = nullptr;
IVideoWindow* pVidWin = nullptr;

// Create Filters
CComPtr<IBaseFilter> pAyncSourceFilter, pLAVSplitter, pLAVVidDecoder, pLAVAudDecoder, pMPCRenderer, pSpoutRenderer, pDefaultAudioRenderer, pDefaultVideoRenderer, pEnhancedVideoRenderer;

// Input Media File Path
LPCWSTR mediaFilePath = L"C:\\Users\\tpthi\\Downloads\\SampleVideo_1280x720_2mb.mp4";

// Manual Connection Flag
bool bConnectFiltersManually = false;


// Funciton to  check unconnected pins of a filter
HRESULT getUnconnectedPin( IBaseFilter* pFilter, PIN_DIRECTION PinDir, IPin** ppPin) 
{
    *ppPin = 0;
    IEnumPins* pEnum = 0;
    IPin* pPin = 0;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return hr;
    }
    while (pEnum->Next(1, &pPin, NULL) == S_OK)
    {
        PIN_DIRECTION ThisPinDir;
        pPin->QueryDirection(&ThisPinDir);
        if (ThisPinDir == PinDir)
        {
            IPin* pTmp = 0;
            hr = pPin->ConnectedTo(&pTmp);
            if (SUCCEEDED(hr)) // Already connected, not the pin we want.
            {
                pTmp->Release();
            }
            else // Unconnected, the pin we want.
            {
                pEnum->Release();
                *ppPin = pPin;
                return S_OK;
            }
        }
        pPin->Release();
    }
    pEnum->Release();
    // Did not find a matching pin.
    return E_FAIL;
}


// Function to connect two filters in a DirectShow graph
HRESULT connectFilters( IGraphBuilder* pGraph, IBaseFilter* pSrc, IBaseFilter* pDest)
{
    if ((pGraph == NULL) || (pSrc == NULL) || (pDest == NULL))
    {
        return E_POINTER;
    }
    //Find Output pin in source filter
    IPin* pOut = 0;
    HRESULT hr = NULL;
    hr = getUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
    if (FAILED(hr)) {
        return hr;
    }
    //Find Input pin in destination filter
    IPin* pIn = 0;
    hr = getUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
    if (FAILED(hr)) {
        return hr;
    }
    //Connnect them
    hr = pGraph->Connect(pOut, pIn);
    pIn->Release();
    pOut->Release();
    return hr;
}
void CleanUp() {
    if (pVidWin) {
        pVidWin->put_Visible(OAFALSE);
        pVidWin->put_Owner(NULL);
        pVidWin->Release();
    }
    if (pControl) pControl->Release();
    if (pEvent) pEvent->Release();
    if (pGraph) pGraph->Release();
    
    CoUninitialize();
}


// Function to create the DierectShow graph
HRESULT manualInitGraph(HWND hWnd) {

    // Initialize COM library
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "COM Initialization Failed");
        return hr;
    }

    // Create the Filter Graph Manager
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to Create Graph Manager");
        return hr;
    }

    // Load CLSIDs of all the filters, Use the CLSIDs from your System Registry
    CLSID clsidLAVSplitter, clsidLAVVideoDecoder, clsidLAVAudioDecoder, clsidMpcRenderer, clsidSpoutRenderer;
    CLSIDFromString(L"{19B0E3A6-E681-4AF6-A10E-BE02C5D25E3F}", &clsidSpoutRenderer);
    CLSIDFromString(L"{171252A0-8820-4AFE-9DF8-5C92B2D66B04}", &clsidLAVSplitter);
    CLSIDFromString(L"{EE30215D-164F-4A92-A4EB-9D4C13390F9F}", &clsidLAVVideoDecoder);
    CLSIDFromString(L"{E8E73B6B-4CB3-44A4-BE99-4F7BCB96E491}", &clsidLAVAudioDecoder);
    CLSIDFromString(L"{71F080AA-8661-4093-B15E-4F6903E77D0A}", &clsidMpcRenderer);



    //CComPtr<IBaseFilter> pAyncSourceFilter, pLAVSplitter, pLAVVidDecoder, pLAVAudDecoder, pMPCRenderer, pSpoutRenderer, pDefaultAudioRenderer, pDefaultVideoRenderer, pEnhancedVideoRenderer;



    hr = CoCreateInstance(CLSID_AsyncReader, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pAyncSourceFilter);
    if (FAILED(hr)) { 
        ErrorLogger::Log(hr, "Failed to create Async Reader");
        return hr;
    }

    hr = CoCreateInstance(clsidLAVSplitter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pLAVSplitter);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to create LAV Splitter! Check CLSID in your Registry");
        return hr;
    }

    hr = CoCreateInstance(clsidLAVVideoDecoder, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pLAVVidDecoder);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to create LAV Video Decoder! Check CLSID in your Registry");
        return hr;
    }

    hr = CoCreateInstance(clsidLAVAudioDecoder, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pLAVAudDecoder);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to create LAV Audio Decoder! Check CLSID in your Registry");
        return hr;
    }

    hr = CoCreateInstance(clsidMpcRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pMPCRenderer);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to create MPC Renderer! Check CLSID in your Registry");
        return hr;
    }
  
   /* hr = CoCreateInstance(clsidSpoutRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pSpoutRenderer);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to create SpoudRenderer! Check CLSID in your Registry");
        return hr;
    }

    hr = CoCreateInstance(CLSID_VideoRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pDefaultVideoRenderer);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to create Default Video Renderer!");
        return hr;
    }
    hr = CoCreateInstance(CLSID_EnhancedVideoRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pEnhancedVideoRenderer);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to create Enhanced Video Renderer!");
        return hr;
    }*/

    // Add filters to graph
    hr = pGraph->AddFilter(pAyncSourceFilter, L"Source");
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to Add Async Reader to the Graph");
        return hr;
    }
    hr = pGraph->AddFilter(pLAVSplitter, L"LAV Splitter");
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to Add LAV Splitter to the Graph");
        return hr;
    }
    hr = pGraph->AddFilter(pLAVVidDecoder, L"LAV Video Decoder");
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to Add LAV Video Decoder to the Graph");
        return hr;
    }
    hr = pGraph->AddFilter(pLAVAudDecoder, L"LAV Audio Decoder");
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to Add LAV Audio Decoder to the Graph");
        return hr;
    }
    hr = pGraph->AddFilter(pMPCRenderer, L"MPC Video Renderer");
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to Add MPC Video Renderer to the Graph");
        return hr;
    }
    //pGraph->AddFilter(pSpoutRenderer, L"Spout Video Renderer");
    //pGraph->AddFilter(pDefaultVideoRenderer, L"Default Video Renderer");
    //pGraph->AddFilter(pEnhancedVideoRenderer, L"Enhanced Video Renderer");

    // Load the file into AsyncReader
    CComQIPtr<IFileSourceFilter> pFileSrc(pAyncSourceFilter);

	// Load the media file into the source filter
    hr = pFileSrc->Load(mediaFilePath, NULL);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to load the Media File. Please check the media path");
        return hr;
    }


    // Connect filters
	// Connect Psource to LAV Splitter
    hr = connectFilters(pGraph, pAyncSourceFilter, pLAVSplitter);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Unable to Connect Async File Source to LAV Spitter");
        return hr;
    }
	// Connect LAV spltter to LAV video decoder
    hr = connectFilters(pGraph, pLAVSplitter, pLAVVidDecoder);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Unable to Connect LAV Splitter to LAV Video Decoder");
        return hr;
    }
	// Connect LAV Videeo Decoder to MPC Renderer
    hr = connectFilters(pGraph, pLAVVidDecoder, pMPCRenderer);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Unable to Connect LAV Video Decoder to MPC Renderer");
        return hr;
    }

    
    // Create the default audio renderer filter
    hr = CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pDefaultAudioRenderer);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to create Default Sound Renderer!");
        return hr;
    }
	// Connect LAV splitter to LAV audio decoder and then to default audio renderer
    if (SUCCEEDED(hr)) {
		// Add the default audio renderer to the graph
        hr = pGraph->AddFilter(pDefaultAudioRenderer, L"Default Audio Renderer");
        if (FAILED(hr)) {
            ErrorLogger::Log(hr, "Failed to Add Default Audio Renderer to to the Graph");
            return hr;
        }
		// Connect the LAV splitter to audio decoder 
        hr = connectFilters(pGraph, pLAVSplitter, pLAVAudDecoder);
        if (FAILED(hr)) {
            ErrorLogger::Log(hr, "Unable to Connect LAV Splitter to LAV Audio Decoder");
            return hr;
        }

		// Connect the audio decoder to the default audio renderer
        hr = connectFilters(pGraph, pLAVAudDecoder, pDefaultAudioRenderer);
        if (FAILED(hr)) {
            ErrorLogger::Log(hr, "Unable to Connect LAV Audio Decoder to Default Audio Renderer");
            return hr;
        }

    }

	// Setup video window and control interfaces
    hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Unable to Query Media Control Interface");
        return hr;
    }
    hr = pGraph->QueryInterface(IID_IMediaEvent, (void**)&pEvent);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Unable to Query Media Event Interface");
        return hr;
    }
    hr = pGraph->QueryInterface(IID_IVideoWindow, (void**)&pVidWin);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Unable to Video Window Control Interface");
        return hr;
    }

	// Set the video window
    if (pVidWin) {
        RECT rc;
        GetClientRect(hWnd, &rc);
        pVidWin->put_Owner((OAHWND)MPCVideoWnd);
        pVidWin->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        pVidWin->put_MessageDrain((OAHWND)MPCVideoWnd);
        pVidWin->SetWindowPosition(0, 0, rc.right, rc.bottom);
        pVidWin->put_Visible(OATRUE);

    }

    return S_OK;
}

// Function to create the DierectShow graph
HRESULT autoInitGraph(HWND hWnd) {

    // Initialize COM library
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        MessageBox(NULL, L"CoInitialize failed", L"Error", MB_OK | MB_ICONERROR);
        return hr;
    }

    // Create the Filter Graph Manager
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);
    if (FAILED(hr)) {
        MessageBox(NULL, L"Failed to create Filter Graph", L"Error", MB_OK | MB_ICONERROR);
        return hr;
    }

    // Load CLSIDs of MPC Renderer, Use the CLSIDs from your System Registry
    CLSID clsidMpcRenderer;
  
    CLSIDFromString(L"{71F080AA-8661-4093-B15E-4F6903E77D0A}", &clsidMpcRenderer);



    hr = CoCreateInstance(clsidMpcRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pMPCRenderer);
    if (FAILED(hr)) return hr;


    // Check for Default Video Renderer
    hr = CoCreateInstance(CLSID_VideoRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pDefaultVideoRenderer);
    if (FAILED(hr)) return hr;


    // Add filters to graph
    hr = pGraph->AddFilter(pMPCRenderer, L"MPC Video Renderer");
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to Add MPC Video Renderer to the Graph");
        return hr;
    }
    /*pGraph->AddFilter(pDefaultVideoRenderer, L"Default Video Renderer");
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to Add Default Video Renderer to the Graph");
        return hr;
    }*/

	// Render the file using the graph
    hr = pGraph->RenderFile(mediaFilePath, NULL);
    if( FAILED(hr)) {
        ErrorLogger::Log(hr, "Failed to Render the Media File. Please check the media path");
        return hr;
	}
 




    // Setup video window and control interfaces
    hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Unable to Query Media Control Interface");
        return hr;
	}
    hr =pGraph->QueryInterface(IID_IMediaEvent, (void**)&pEvent);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Unable to Query Media Event Interface");
        return hr;
    }
    hr =pGraph->QueryInterface(IID_IVideoWindow, (void**)&pVidWin);
    if (FAILED(hr)) {
        ErrorLogger::Log(hr, "Unable to Video Window Control Interface");
        return hr;
	}

    // Set the video window
    if (pVidWin) {
        RECT rc;
        GetClientRect(hWnd, &rc);
        pVidWin->put_Owner((OAHWND)MPCVideoWnd);
        pVidWin->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        pVidWin->put_MessageDrain((OAHWND)MPCVideoWnd);
        pVidWin->SetWindowPosition(0, 0, rc.right, rc.bottom);
        pVidWin->put_Visible(OATRUE);

    }

    return S_OK;
}


void resizeVideoWindow(HWND hWnd) {
    if (pVidWin) {
        RECT rc;
        GetClientRect(hWnd, &rc);
        pVidWin->SetWindowPosition(10, 60, rc.right - 20, rc.bottom - 70);
    }
}

// Window Procedure for handling messages
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {


    switch (msg) {
    case WM_CREATE:

       // Creating the Play button
        hPlayBtn = CreateWindow(L"BUTTON", L"Play", WS_CHILD | WS_VISIBLE,
            10, 10, 60, 30, hWnd, (HMENU)ID_BUTTON_PLAY, NULL, NULL);
        // Creating the Pause button
        hPauseBtn = CreateWindow(L"BUTTON", L"Pause", WS_CHILD | WS_VISIBLE,
            80, 10, 60, 30, hWnd, (HMENU)ID_BUTTON_PAUSE, NULL, NULL);
        // Creating the Stop button
        hStopBtn = CreateWindow(L"BUTTON", L"Stop", WS_CHILD | WS_VISIBLE,
            150, 10, 60, 30, hWnd, (HMENU)ID_BUTTON_STOP, NULL, NULL);

		// Creating the MPC Video Window with the Video Player window
        MPCVideoWnd = CreateWindow(
            L"STATIC", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            10, 60, 760, 480,
            hWnd, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
			// Play when Play button is clicked
        case ID_BUTTON_PLAY:
            if (pControl) {
                HRESULT hr = pControl->Run();

            }
            break;
			// Pause when Pause button is clicked
        case ID_BUTTON_PAUSE:
            if (pControl) pControl->Pause();
            break;
			// Stop when Stop button is clicked
        case ID_BUTTON_STOP:
            if (pControl) pControl->Stop();
            break;
        }
        break;

    case WM_SIZE:
		// Resize the video window when the main window is resized
        resizeVideoWindow(MPCVideoWnd);

		// Adjust the size of the MPC Video Window
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

// Main function to initialize the application
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    
	// Initialize the Windwow Class Object
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"MPC Player";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(64, 64, 64));
    wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(101));
    // Register the Windo Class Object
    if(!RegisterClass(&wc)) {
        ErrorLogger::Log("Failed to Register Window Class");
        return -1;
	}

	// Create the Main Window
    hMainWnd = CreateWindow(wc.lpszClassName, L"MPC Video Player",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 800, 600,
        NULL, NULL, hInst, NULL);

    if (!hMainWnd) {
        ErrorLogger::Log("Failed to Create Main Window");
        return -1;
    }

	// If the user wants to connect filters manually
    if(bConnectFiltersManually) {
        if (FAILED(manualInitGraph(hMainWnd))) {
            ErrorLogger::Log("Manual Graph Init Failed");
            return -1;
        }
    }
    else {
		// If the user wants to connect filters automatically
        if (FAILED(autoInitGraph(hMainWnd))) {
            ErrorLogger::Log("Automatic Graph Init Failed");
            return -1;
        }
    
    }
    
	// Set the instance handle
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

