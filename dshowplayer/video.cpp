//////////////////////////////////////////////////////////////////////////
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////
#include <atlbase.h>
#include "DShowPlayer.h"
#include "DshowUtil.h"
#include <Mfidl.h>

HRESULT InitializeEVR(IBaseFilter *pEVR, HWND hwnd, IMFVideoDisplayControl ** ppWc); 
HRESULT InitWindowlessVMR9(IBaseFilter *pVMR, HWND hwnd, IVMRWindowlessControl9 ** ppWc); 
HRESULT InitWindowlessVMR(IBaseFilter *pVMR, HWND hwnd, IVMRWindowlessControl** ppWc); 

/// VMR-7 Wrapper

VMR7::VMR7() : m_pWindowless(NULL)
{

}


VMR7::~VMR7()
{
    SAFE_RELEASE(m_pWindowless);
}

HRESULT VMR7::AddToGraph(IGraphBuilder *pGraph, HWND hwnd)
{
    HRESULT hr = S_OK;

    IBaseFilter *pVMR = NULL;

	hr = AddFilterByCLSID(pGraph, CLSID_VideoMixingRenderer, &pVMR, L"VMR-7");

	// Set windowless mode on the VMR. This must be done before the VMR is connected.
	if (SUCCEEDED(hr))
	{
		hr = InitWindowlessVMR(pVMR, hwnd, &m_pWindowless);
	}

    return hr;
}


HRESULT VMR7::FinalizeGraph(IGraphBuilder *pGraph)
{
    if (m_pWindowless == NULL)
    {
        return S_OK;
    }

    HRESULT hr = S_OK;
	BOOL bRemoved = FALSE;

    IBaseFilter *pFilter = NULL;

    hr = m_pWindowless->QueryInterface(__uuidof(IBaseFilter), (void**)&pFilter);

    if (SUCCEEDED(hr))
    {
		hr = RemoveUnconnectedRenderer(pGraph, pFilter, &bRemoved);

		// If we removed the VMR, then we also need to release our 
		// pointer to the VMR's windowless control interface.
		if (bRemoved)
		{
			SAFE_RELEASE(m_pWindowless);
		}
    }

    SAFE_RELEASE(pFilter);
    return hr;
}

HRESULT VMR7::UpdateVideoWindow(HWND hwnd, const LPRECT prc)
{
    HRESULT hr = S_OK;

	if (m_pWindowless == NULL)
	{
		return S_OK; // no-op
	}

	if (prc)
	{
		hr = m_pWindowless->SetVideoPosition(NULL, prc);
	}
	else
	{

		RECT rc;
		GetClientRect(hwnd, &rc);
		hr = m_pWindowless->SetVideoPosition(NULL, &rc);
	}
    return hr;
}

HRESULT VMR7::Repaint(HWND hwnd, HDC hdc)
{
    HRESULT hr = S_OK;

	if (m_pWindowless)
	{
		hr = m_pWindowless->RepaintVideo(hwnd, hdc);
	}
    return hr;
}

HRESULT VMR7::DisplayModeChanged()
{
    HRESULT hr = S_OK;

	if (m_pWindowless)
	{
		hr = m_pWindowless->DisplayModeChanged();
	}
    return hr;


}


//-----------------------------------------------------------------------------
// InitWindowlessVMR
// Initialize the VMR-7 for windowless mode. 
//-----------------------------------------------------------------------------

HRESULT InitWindowlessVMR( 
    IBaseFilter *pVMR,				// Pointer to the VMR
	HWND hwnd,						// Clipping window
	IVMRWindowlessControl** ppWC	// Receives a pointer to the VMR.
    ) 
{ 

    IVMRFilterConfig* pConfig = NULL; 
	IVMRWindowlessControl *pWC = NULL;

	HRESULT hr = S_OK;

	// Set the rendering mode.  
    hr = pVMR->QueryInterface(IID_IVMRFilterConfig, (void**)&pConfig); 
    if (SUCCEEDED(hr)) 
    { 
        hr = pConfig->SetRenderingMode(VMRMode_Windowless); 
    }

	// Query for the windowless control interface.
    if (SUCCEEDED(hr))
    {
        hr = pVMR->QueryInterface(IID_IVMRWindowlessControl, (void**)&pWC);
	}

	// Set the clipping window.
	if (SUCCEEDED(hr))
	{
		hr = pWC->SetVideoClippingWindow(hwnd);
	}

	// Preserve aspect ratio by letter-boxing
	if (SUCCEEDED(hr))
	{
		hr = pWC->SetAspectRatioMode(VMR_ARMODE_LETTER_BOX);
	}

	// Return the IVMRWindowlessControl pointer to the caller.
	if (SUCCEEDED(hr))
	{
		*ppWC = pWC;
		(*ppWC)->AddRef();
	}

	SAFE_RELEASE(pConfig);
	SAFE_RELEASE(pWC);

	return hr; 
} 


/// VMR-9 Wrapper

VMR9::VMR9() : m_pWindowless(NULL)
{

}


VMR9::~VMR9()
{
    SAFE_RELEASE(m_pWindowless);
}

HRESULT VMR9::AddToGraph(IGraphBuilder *pGraph, HWND hwnd)
{
    HRESULT hr = S_OK;

    IBaseFilter *pVMR = NULL;

	hr = AddFilterByCLSID(pGraph, CLSID_VideoMixingRenderer9, &pVMR, L"VMR-9");

	// Set windowless mode on the VMR. This must be done before the VMR is connected.
	if (SUCCEEDED(hr))
	{
		hr = InitWindowlessVMR9(pVMR, hwnd, &m_pWindowless);
	}


    return hr;
}


HRESULT VMR9::FinalizeGraph(IGraphBuilder *pGraph)
{
    if (m_pWindowless == NULL)
    {
        return S_OK;
    }

    HRESULT hr = S_OK;
	BOOL bRemoved = FALSE;

    IBaseFilter *pFilter = NULL;

    hr = m_pWindowless->QueryInterface(__uuidof(IBaseFilter), (void**)&pFilter);

    if (SUCCEEDED(hr))
    {
		hr = RemoveUnconnectedRenderer(pGraph, pFilter, &bRemoved);

		// If we removed the VMR, then we also need to release our 
		// pointer to the VMR's windowless control interface.
		if (bRemoved)
		{
			SAFE_RELEASE(m_pWindowless);
		}
    }

    SAFE_RELEASE(pFilter);
    return hr;
}




HRESULT VMR9::UpdateVideoWindow(HWND hwnd, const LPRECT prc)
{
    HRESULT hr = S_OK;

	if (m_pWindowless == NULL)
	{
		return S_OK; // no-op
	}

	if (prc)
	{
		hr = m_pWindowless->SetVideoPosition(NULL, prc);
	}
	else
	{

		RECT rc;
		GetClientRect(hwnd, &rc);
		hr = m_pWindowless->SetVideoPosition(NULL, &rc);
	}
    return hr;
}

HRESULT VMR9::Repaint(HWND hwnd, HDC hdc)
{
    HRESULT hr = S_OK;

	if (m_pWindowless)
	{
		hr = m_pWindowless->RepaintVideo(hwnd, hdc);
	}
    return hr;
}

HRESULT VMR9::DisplayModeChanged()
{
    HRESULT hr = S_OK;

	if (m_pWindowless)
	{
		hr = m_pWindowless->DisplayModeChanged();
	}
    return hr;


}


//-----------------------------------------------------------------------------
// InitWindowlessVMR9
// Initialize the VMR-9 for windowless mode. 
//-----------------------------------------------------------------------------

HRESULT InitWindowlessVMR9( 
    IBaseFilter *pVMR,				// Pointer to the VMR
	HWND hwnd,						// Clipping window
	IVMRWindowlessControl9** ppWC	// Receives a pointer to the VMR.
    ) 
{ 

    IVMRFilterConfig9 * pConfig = NULL; 
	IVMRWindowlessControl9 *pWC = NULL;

	HRESULT hr = S_OK;

	// Set the rendering mode.  
    hr = pVMR->QueryInterface(IID_IVMRFilterConfig9, (void**)&pConfig); 
    if (SUCCEEDED(hr)) 
    { 
        hr = pConfig->SetRenderingMode(VMR9Mode_Windowless); 
    }

	// Query for the windowless control interface.
    if (SUCCEEDED(hr))
    {
        hr = pVMR->QueryInterface(IID_IVMRWindowlessControl9, (void**)&pWC);
	}

	// Set the clipping window.
	if (SUCCEEDED(hr))
	{
		hr = pWC->SetVideoClippingWindow(hwnd);
	}

	// Preserve aspect ratio by letter-boxing
	if (SUCCEEDED(hr))
	{
		hr = pWC->SetAspectRatioMode(VMR9ARMode_LetterBox);
	}

	// Return the IVMRWindowlessControl pointer to the caller.
	if (SUCCEEDED(hr))
	{
		*ppWC = pWC;
		(*ppWC)->AddRef();
	}

	SAFE_RELEASE(pConfig);
	SAFE_RELEASE(pWC);

	return hr; 
} 





/// EVR Wrapper

EVR::EVR() : m_pEVR(NULL), m_pVideoDisplay(NULL)
{

}


EVR::~EVR()
{
    SAFE_RELEASE(m_pEVR);
    SAFE_RELEASE(m_pVideoDisplay);
}

HRESULT EVR::AddToGraph(IGraphBuilder *pGraph, HWND hwnd)
{
    HRESULT hr = S_OK;

    IBaseFilter *pEVR = NULL;

	hr = AddFilterByCLSID(pGraph, CLSID_EnhancedVideoRenderer, &pEVR, L"EVR");

	if (SUCCEEDED(hr))
	{
		hr = InitializeEVR(pEVR, hwnd, &m_pVideoDisplay);
	}

    if (SUCCEEDED(hr))
    {
        // Note: Because IMFVideoDisplayControl is a service interface,
        // you cannot QI the pointer to get back the IBaseFilter pointer.
        // Therefore, we need to cache the IBaseFilter pointer.
    
        m_pEVR = pEVR;
        m_pEVR->AddRef();
    }

    SAFE_RELEASE(pEVR);

    return hr;
}


HRESULT EVR::FinalizeGraph(IGraphBuilder *pGraph)
{
    if (m_pEVR == NULL)
    {
        return S_OK;
    }

    HRESULT hr = S_OK;
	BOOL bRemoved = FALSE;

	hr = RemoveUnconnectedRenderer(pGraph, m_pEVR, &bRemoved);

	if (bRemoved)
	{
		SAFE_RELEASE(m_pEVR);
        SAFE_RELEASE(m_pVideoDisplay);
	}

    return hr;
}


HRESULT EVR::UpdateVideoWindow(HWND hwnd, const LPRECT prc)
{
    HRESULT hr = S_OK;

	if (m_pVideoDisplay == NULL)
	{
		return S_OK; // no-op
	}

	if (prc)
	{
		hr = m_pVideoDisplay->SetVideoPosition(NULL, prc);
	}
	else
	{

		RECT rc;
		GetClientRect(hwnd, &rc);
		hr = m_pVideoDisplay->SetVideoPosition(NULL, &rc);
	}
    return hr;
}

HRESULT EVR::Repaint(HWND hwnd, HDC hdc)
{
    HRESULT hr = S_OK;

	if (m_pVideoDisplay)
	{
		hr = m_pVideoDisplay->RepaintVideo();
	}
    return hr;
}

HRESULT EVR::DisplayModeChanged()
{
    return S_OK; // No-op
}


//-----------------------------------------------------------------------------
// InitializeEVR
// Initialize the EVR filter. 
//-----------------------------------------------------------------------------

HRESULT InitializeEVR( 
    IBaseFilter *pEVR,				// Pointer to the EVR
	HWND hwnd,						// Clipping window
	IMFVideoDisplayControl** ppDisplayControl
    ) 
{ 

	HRESULT hr = S_OK;

    IMFGetService *pService = NULL;
	IMFVideoDisplayControl *pDisplay = NULL;

    hr = pEVR->QueryInterface(__uuidof(IMFGetService), (void**)&pService); 
    if (SUCCEEDED(hr)) 
    { 
        hr = pService->GetService(
            MR_VIDEO_RENDER_SERVICE, 
            __uuidof(IMFVideoDisplayControl),
            (void**)&pDisplay
            );
    }

	// Set the clipping window.
	if (SUCCEEDED(hr))
	{
		hr = pDisplay->SetVideoWindow(hwnd);
	}

	// Preserve aspect ratio by letter-boxing
	if (SUCCEEDED(hr))
	{
		hr = pDisplay->SetAspectRatioMode(MFVideoARMode_PreservePicture);
	}

	// Return the IMFVideoDisplayControl pointer to the caller.
	if (SUCCEEDED(hr))
	{
		*ppDisplayControl = pDisplay;
		(*ppDisplayControl)->AddRef();
	}

	SAFE_RELEASE(pService);
	SAFE_RELEASE(pDisplay);

	return hr; 
} 


// MPC VR Wrapper

MPCVR::MPCVR() : m_pMCPVR(NULL), m_pVideoDisplay(NULL)
{

}

//-----------------------------------------------------------------------------
// InitializeMPCVR
// Initialize the MPCVR filter. 
//-----------------------------------------------------------------------------

HRESULT InitializeMPCVR(
    IBaseFilter* pMPCVR,
    HWND hwnd,
    IVideoWindow** ppVideoWindow
)
{
    if (!pMPCVR || !ppVideoWindow)
        return E_POINTER;

    IVideoWindow* pVidWin = nullptr;
    HRESULT hr = pMPCVR->QueryInterface(IID_PPV_ARGS(&pVidWin));
    if (FAILED(hr)) return hr;

    // Set owner and style
    hr = pVidWin->put_Owner((OAHWND)hwnd);
    pVidWin->put_MessageDrain((OAHWND)hwnd);

    if (FAILED(hr)) {
        pVidWin->Release();
        return hr;
    }

    pVidWin->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);
    pVidWin->put_WindowStyleEx(WS_EX_NOPARENTNOTIFY);


    RECT rc;
    GetClientRect(hwnd, &rc);
    pVidWin->put_Visible(OATRUE);
    pVidWin->SetWindowPosition(0, 0, rc.right, rc.bottom);
    

    *ppVideoWindow = pVidWin;
    (*ppVideoWindow)->AddRef();

    SAFE_RELEASE(pVidWin);
    return S_OK;
}

MPCVR::~MPCVR()
{
    SAFE_RELEASE(m_pMCPVR);
    SAFE_RELEASE(m_pVideoDisplay);
}
HRESULT MPCVR::AddToGraph(IGraphBuilder* pGraph, HWND hwnd)
{
    HRESULT hr = S_OK;

    CLSID clsidMpcRenderer;
    hr = CLSIDFromString(L"{71F080AA-8661-4093-B15E-4F6903E77D0A}", &clsidMpcRenderer);
    if (FAILED(hr)) return hr;

    IBaseFilter* pMpcRenderer = nullptr;
    hr = CoCreateInstance(clsidMpcRenderer, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pMpcRenderer));
    if (FAILED(hr)) return hr;

    hr = pGraph->AddFilter(pMpcRenderer, L"MPC Video Renderer");
    if (FAILED(hr)) {
        SAFE_RELEASE(pMpcRenderer);
        return hr;
    }

    // Initialize IVideoWindow
    hr = InitializeMPCVR(pMpcRenderer, hwnd, &m_pVideoDisplay);
    if (SUCCEEDED(hr)) {
        m_pMCPVR = pMpcRenderer;
        m_pMCPVR->AddRef();
    }

    SAFE_RELEASE(pMpcRenderer);
    return hr;
}
//HRESULT MPCVR::AddToGraph(IGraphBuilder* pGraph, HWND hwnd)
//{
//    HRESULT hr = S_OK;
//    CLSID clsidMpcRenderer;
//    hr = CLSIDFromString(L"{71F080AA-8661-4093-B15E-4F6903E77D0A}", &clsidMpcRenderer);
//    if (FAILED(hr)) return hr;
//
//    CComPtr<IBaseFilter> pMpcRenderer;
//    hr = CoCreateInstance(clsidMpcRenderer, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pMpcRenderer));
//    if (SUCCEEDED(hr)) {
//        hr = pGraph->AddFilter(pMpcRenderer, L"MPC Video Renderer");
//        // Optionally, set video window owner here if supported
//    }
//
//    //InitializeMPCVR();
//    return hr;
//
//    
//}

HRESULT MPCVR::FinalizeGraph(IGraphBuilder* pGraph)
{
    if (m_pMCPVR == NULL)
        return S_OK;

    HRESULT hr = S_OK;
    BOOL bRemoved = FALSE;

    hr = RemoveUnconnectedRenderer(pGraph, m_pMCPVR, &bRemoved);

    if (bRemoved)
    {
        SAFE_RELEASE(m_pMCPVR);
        SAFE_RELEASE(m_pVideoDisplay);
    }

    return hr;
}
//HRESULT MPCVR::FinalizeGraph(IGraphBuilder* pGraph)
//{
//    if (m_pMCPVR == NULL)
//    {
//        return S_OK;
//    }
//
//    HRESULT hr = S_OK;
//    BOOL bRemoved = FALSE;
//
//    hr = RemoveUnconnectedRenderer(pGraph, m_pMCPVR, &bRemoved);
//
//    if (bRemoved)
//    {
//        SAFE_RELEASE(m_pMCPVR);
//        SAFE_RELEASE(m_pVideoDisplay);
//    }
//
//    return hr;
//}

HRESULT MPCVR::UpdateVideoWindow(HWND hwnd, const LPRECT prc)
{
    if (!m_pVideoDisplay)
        return S_OK;

    HRESULT hr = S_OK;
    RECT rc;

    if (prc)
        rc = *prc;
    else
        GetClientRect(hwnd, &rc);

    hr = m_pVideoDisplay->SetWindowPosition(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
    return hr;
}
//HRESULT MPCVR::UpdateVideoWindow(HWND hwnd, const LPRECT prc)
//{
//    HRESULT hr = S_OK;
//
//    return hr;
//}

HRESULT MPCVR::Repaint(HWND hwnd, HDC hdc)
{/*
    if (!m_pVideoDisplay)
        return S_OK;
    
    return m_pVideoDisplay->RepaintVideo(hwnd, hdc);*/
    return S_OK; // No-op, as MPC VR does not support RepaintVideo in this context.
}
//HRESULT MPCVR::Repaint(HWND hwnd, HDC hdc)
//{
//    HRESULT hr = S_OK;
//
//   
//    return hr;
//}

HRESULT MPCVR::DisplayModeChanged()
{
    return S_OK; // No-op
}
//HRESULT MPCVR::DisplayModeChanged()
//{
//    return S_OK; // No-op
//}


//HRESULT InitializeMPCVR(
//    IBaseFilter* pMPCVR,				// Pointer to the EVR
//    HWND hwnd,						// Clipping window
//    IVideoWindow** ppDisplayControl
//
//)
//{
//
//    HRESULT hr = S_OK;
//
//    IMFGetService* pService = NULL;
//    IVideoWindow* pVideoWin = nullptr;
//
//    hr = pMPCVR->QueryInterface(__uuidof(pVideoWin), (void**)&pService);
//    if (SUCCEEDED(hr))
//    {
//        hr = pService->GetService(
//            MR_VIDEO_RENDER_SERVICE,
//            __uuidof(IMFVideoDisplayControl),
//            (void**)&pVideoWin
//        );
//    }
//
//    // Set the clipping window.
//    if (SUCCEEDED(hr))
//    {
//        hr = pVideoWin->put_Owner((OAHWND)hwnd);
//    }
//
//    // Preserve aspect ratio by letter-boxing
//    if (SUCCEEDED(hr))
//    {
//        pVideoWin->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);
//        RECT rc;
//        GetClientRect(hwnd, &rc);
//        pVideoWin->SetWindowPosition(0, 0, rc.right, rc.bottom);
//        pVideoWin->put_Visible(OATRUE);
//        pVideoWin->Release();
//        //hr = pVideoWin->SetAspectRatioMode(MFVideoARMode_PreservePicture);
//    }
//
//    // Return the IMFVideoDisplayControl pointer to the caller.
//    if (SUCCEEDED(hr))
//    {
//        *ppDisplayControl = pVideoWin;
//        (*ppDisplayControl)->AddRef();
//    }
//
//    SAFE_RELEASE(pService);
//    SAFE_RELEASE(pVideoWin);
//
//    return hr;
//}
