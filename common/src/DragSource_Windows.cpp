#ifdef _WIN32

#include "DragSource.h"
#include <windows.h>
#include <shlobj.h> // For shell drag-and-drop
#include <objidl.h> // For COM interfaces

class FileDropSource : public IDropSource
{
public:
    ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
    ULONG STDMETHODCALLTYPE Release() override { return 1; }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) override
    {
        if (riid == IID_IUnknown || riid == IID_IDropSource)
        {
            *ppv = this;
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL escapePressed, DWORD keyState) override
    {
        if (escapePressed)
            return DRAGDROP_S_CANCEL;
        if (!(keyState & MK_LBUTTON))
            return DRAGDROP_S_DROP;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD effect) override { return DRAGDROP_S_USEDEFAULTCURSORS; }
};

void DragSource::startDraggingFile(const std::string &filePath, void *nativeView)
{
    // Convert std::string to wide string (Windows uses wchar_t)
    std::wstring wFilePath(filePath.begin(), filePath.end());

    // Allocate DROPFILES structure
    size_t memSize = sizeof(DROPFILES) + (wFilePath.size() + 2) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GHND, memSize);
    if (!hMem)
        return;

    // Fill in the DROPFILES structure
    DROPFILES *pDropFiles = (DROPFILES *)GlobalLock(hMem);
    pDropFiles->pFiles = sizeof(DROPFILES);
    pDropFiles->fWide = TRUE; // Use Unicode
    wcscpy((wchar_t *)((BYTE *)pDropFiles + sizeof(DROPFILES)), wFilePath.c_str());
    GlobalUnlock(hMem);

    // Create IDataObject
    IDataObject *pDataObject;
    if (FAILED(OleInitialize(nullptr)))
        return; // Initialize COM
    if (FAILED(SHCreateDataObject(nullptr, 1, &hMem, nullptr, IID_IDataObject, (void **)&pDataObject)))
    {
        GlobalFree(hMem);
        return;
    }

    // Start drag-and-drop operation
    FileDropSource dropSource;
    DWORD dwEffect;
    DoDragDrop(pDataObject, &dropSource, DROPEFFECT_COPY, &dwEffect);

    // Clean up
    pDataObject->Release();
    GlobalFree(hMem);
    OleUninitialize();
}

#endif