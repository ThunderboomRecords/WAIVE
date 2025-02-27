#ifdef _WIN32

#include "DragSource.h"
#include <windows.h>
#include <shlobj.h>
#include <oleidl.h>
#include <vector>

// Custom IDataObject implementation
class FileDataObject : public IDataObject
{
private:
    LONG refCount;
    STGMEDIUM stgMedium;
    FORMATETC formatEtc;

public:
    FileDataObject(HGLOBAL hGlobal) : refCount(1)
    {
        formatEtc.cfFormat = CF_HDROP;
        formatEtc.ptd = nullptr;
        formatEtc.dwAspect = DVASPECT_CONTENT;
        formatEtc.lindex = -1;
        formatEtc.tymed = TYMED_HGLOBAL;

        stgMedium.tymed = TYMED_HGLOBAL;
        stgMedium.hGlobal = hGlobal;
        stgMedium.pUnkForRelease = nullptr;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) override
    {
        if (riid == IID_IUnknown || riid == IID_IDataObject)
        {
            *ppv = static_cast<IDataObject *>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&refCount); }

    ULONG STDMETHODCALLTYPE Release() override
    {
        LONG count = InterlockedDecrement(&refCount);
        if (count == 0)
        {
            GlobalFree(stgMedium.hGlobal);
            delete this;
        }
        return count;
    }

    HRESULT STDMETHODCALLTYPE GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium) override
    {
        if (pformatetcIn->cfFormat == CF_HDROP && pformatetcIn->tymed == TYMED_HGLOBAL)
        {
            pmedium->tymed = TYMED_HGLOBAL;
            pmedium->hGlobal = GlobalAlloc(GHND, GlobalSize(stgMedium.hGlobal));
            if (!pmedium->hGlobal)
                return E_OUTOFMEMORY;

            void *dest = GlobalLock(pmedium->hGlobal);
            void *src = GlobalLock(stgMedium.hGlobal);
            memcpy(dest, src, GlobalSize(stgMedium.hGlobal));
            GlobalUnlock(stgMedium.hGlobal);
            GlobalUnlock(pmedium->hGlobal);

            pmedium->pUnkForRelease = nullptr;
            return S_OK;
        }
        return DV_E_FORMATETC;
    }

    HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC *, STGMEDIUM *) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC *) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC *, FORMATETC *) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE SetData(FORMATETC *, STGMEDIUM *, BOOL) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD, IEnumFORMATETC **) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC *, DWORD, IAdviseSink *, DWORD *) override { return OLE_E_ADVISENOTSUPPORTED; }
    HRESULT STDMETHODCALLTYPE DUnadvise(DWORD) override { return OLE_E_ADVISENOTSUPPORTED; }
    HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA **) override { return OLE_E_ADVISENOTSUPPORTED; }
};

// Custom IDropSource implementation
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
    std::wstring wFilePath(filePath.begin(), filePath.end());

    // Allocate DROPFILES structure
    size_t memSize = sizeof(DROPFILES) + (wFilePath.size() + 2) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GHND, memSize);
    if (!hMem)
        return;

    // Fill DROPFILES structure
    DROPFILES *pDropFiles = (DROPFILES *)GlobalLock(hMem);
    pDropFiles->pFiles = sizeof(DROPFILES);
    pDropFiles->fWide = TRUE;
    wcscpy((wchar_t *)((BYTE *)pDropFiles + sizeof(DROPFILES)), wFilePath.c_str());
    GlobalUnlock(hMem);

    // Create COM drag objects
    FileDataObject *dataObject = new FileDataObject(hMem);
    FileDropSource dropSource;

    // Start the drag operation
    DWORD dwEffect;
    OleInitialize(nullptr); // Initialize COM
    DoDragDrop(dataObject, &dropSource, DROPEFFECT_COPY, &dwEffect);
    dataObject->Release();
    OleUninitialize();
}

#endif
