#pragma once
#include <windows.h>

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