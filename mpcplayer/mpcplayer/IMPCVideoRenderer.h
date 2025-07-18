#pragma once
#include <Unknwn.h>

interface __declspec(uuid("84E7C79F-9F8F-4026-92D4-1DCC05D50CFC")) IMPCVideoRenderer : public IUnknown{
    virtual HRESULT STDMETHODCALLTYPE SetVideoWindow(HWND hWnd) = 0;
};

interface __declspec(uuid("B79BB0A0-33AC-4D5F-A13B-65EC379679F5")) IMPCVideoRendererFilter : public IUnknown{
    virtual HRESULT STDMETHODCALLTYPE SetBool(LPCSTR field, BOOL value) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetInt(LPCSTR field, int value) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetFloat(LPCSTR field, float value) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetString(LPCSTR field, LPCSTR value) = 0;
};
