/*
 * KermOLE.cpp
 */


#include "kermit.h"

#include "DKermOA.h"

void PUBFUNC GetCLSIDDesc(CLSID clsid, PSTR szDescription)
{
    LPOLESTR pwszCLSID;
    char szCLSID[80];
    char szRegKey [80];
    char szRegValue [80];
    long lRegValue;

    StringFromCLSID(clsid, &pwszCLSID);
    OLETOANSI(pwszCLSID, szCLSID, sizeof(szCLSID));

    CoTaskMemFree(pwszCLSID);

    lRegValue = sizeof(szRegValue);
    wsprintf(szRegKey, "CLSID\\%s\\InprocServer32", szCLSID);

    if (RegQueryValue(HKEY_CLASSES_ROOT, szRegKey, szRegValue, &lRegValue) == ERROR_SUCCESS)
        GetFileVerString(szRegValue, szDescription);
    else
        lstrcpy(szDescription, "No version info available");
}


BOOL LoadKermTypeLib(void)
{
    HRESULT hr;

    DebMsg(DL_INFO, "LoadKermTypeLib()...");

    hr = LoadRegTypeLib(LIBID_KermOATypeLibrary, 1, 0, LANG_NEUTRAL, &g_pKermTypeLib);
    DebMsg(DL_INFO, "LoadRegTypeLib() returned %lX", hr);
    if (FAILED(hr))
    {
        PATHNAM szModuleFileName;
        OLECHAR wszModuleFileName [sizeof(szModuleFileName)];

       /*
        * If LoadRegTypeLib fails, try loading directly with
        * LoadTypeLib and register.
        */

        GetModuleFileName(hAppInst, (LPSTR)szModuleFileName, sizeof(szModuleFileName));
        lstrcat(szModuleFileName, "\\1");

      DebMsg(DL_INFO, "szModuleFileName: %s", szModuleFileName);

        ANSITOOLE(szModuleFileName, wszModuleFileName, sizeof(wszModuleFileName));

        hr = LoadTypeLib(wszModuleFileName, &g_pKermTypeLib);
    DebMsg(DL_INFO, "LoadTypeLib() returned %lX", hr);
        if (FAILED(hr))
            return hr;

        hr = RegisterTypeLib(g_pKermTypeLib, wszModuleFileName, OLIT("HelpFile"));
    DebMsg(DL_INFO, "RegisterTypeLib() returned %lX", hr);
    }

    return SUCCEEDED(hr);
}

ITypeInfo * GetKermTypeInfo(REFGUID guid)
{
    ITypeInfo * pITypeInfo;
    HRESULT hr;

    DebMsg(DL_INFO, "GetKermTypeInfo()...");

    pITypeInfo = NULL;

    if (g_pKermTypeLib != NULL)
    {
        hr = g_pKermTypeLib->GetTypeInfoOfGuid(guid, &pITypeInfo);
        if (FAILED(hr))
        {
            KermitFmtMsgBox(MB_OK, "GetTypeInfoOfGuid Failed!");
            return NULL;
        }
    }

    return pITypeInfo;
}

PCSupportErrorInfo CSupportErrorInfo::Create(LPUNKNOWN pUnkOuter, REFIID riid)
{
    PCSupportErrorInfo pSEI;

    DebMsg(DL_INFO, "Creating CSupportErrorInfo...");

    if ((pSEI = new CSupportErrorInfo()) == NULL)
        return NULL;

    pSEI->m_pUnkOuter = pUnkOuter;
    pSEI->m_iid = riid;

    return pSEI;
}

CSupportErrorInfo::CSupportErrorInfo(void)
{
    DebMsg(DL_INFO, "CSupportErrorInfo creation...");
    m_pUnkOuter = NULL;
//    m_iid = ?;
}

STDMETHODIMP
CSupportErrorInfo::QueryInterface(REFIID iid, void FAR* FAR* ppv)
{
    DebMsg(DL_INFO, "CSupportErrorInfo::QueryInterface()...");
    return m_pUnkOuter->QueryInterface(iid, ppv);
}

STDMETHODIMP_(ULONG)
CSupportErrorInfo::AddRef(void)
{
    DebMsg(DL_INFO, "CSupportErrorInfo::AddRef()...");
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG)
CSupportErrorInfo::Release(void)
{
    DebMsg(DL_INFO, "CSupportErrorInfo::Release()...");
    return m_pUnkOuter->Release();
}

STDMETHODIMP
CSupportErrorInfo::InterfaceSupportsErrorInfo(REFIID riid)
{
    DebMsg(DL_INFO, "CSupportErrorInfo::InterfaceSupportsErrorInfo()...");
    return (riid == m_iid) ? NOERROR : ResultFromScode(S_FALSE);
}



PCProvideClassInfo CProvideClassInfo::Create(LPUNKNOWN pUnkOuter, ITypeInfo * pITI)
{
    PCProvideClassInfo pPCI;

    DebMsg(DL_INFO, "Creating CProvideClassInfo...");

    if ((pPCI = new CProvideClassInfo()) == NULL)
        return NULL;

    pPCI->m_pUnkOuter = pUnkOuter;
    pPCI->m_pITI = pITI;
    pPCI->m_pITI->AddRef();

    return pPCI;
}

CProvideClassInfo::CProvideClassInfo(void)
{
    DebMsg(DL_INFO, "CProvideClassInfo creation...");

    return;
}

CProvideClassInfo::~CProvideClassInfo(void)
{
    DebMsg(DL_INFO, "CProvideClassInfo destruction...");

    ReleaseInterface(m_pITI);

    return;
}

STDMETHODIMP CProvideClassInfo::QueryInterface(REFIID riid, void FAR * FAR * ppv)
{
    DebMsg(DL_INFO, "CProvideClassInfo::QueryInterface()...");
    return m_pUnkOuter->QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG) CProvideClassInfo::AddRef(void)
{
    DebMsg(DL_INFO, "CProvideClassInfo::AddRef()...");
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CProvideClassInfo::Release(void)
{
    DebMsg(DL_INFO, "CProvideClassInfo::Release()...");
    return m_pUnkOuter->Release();
}

STDMETHODIMP CProvideClassInfo::GetClassInfo(ITypeInfo FAR * FAR * ppITI)
{
    DebMsg(DL_INFO, "CProvideClassInfo::GetClassInfo()...");
    if (ppITI == NULL)
        return ResultFromScode(E_POINTER);

    *ppITI = m_pITI;

    if (m_pITI != NULL)
    {
        m_pITI->AddRef();

        return NOERROR;
    }

    return ResultFromScode(E_FAIL);
}
