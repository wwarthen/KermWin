/*
 * KermOA.CPP
 */


#include "kermit.h"

#include "DKermOA.h"

#ifdef AXSCRIPT
extern BOOL bScriptAbort;      /* TRUE when we only want to abort script      */
#endif

typedef struct tagInvokeParmBlock
{
    void FAR *          _this;
    ITypeInfo FAR *     ptinfo;
    DISPID              dispidMember;
    unsigned short      wFlags;
    DISPPARAMS FAR *    pparms;
    VARIANT FAR *       pvarResult;
    EXCEPINFO FAR *     pexcepinfo;
    unsigned int FAR *  puArgErr;
    HRESULT             hr;
} InvokeParmBlock;

static InvokeParmBlock  ipb;
static BOOL bDispInvokePending = FALSE;
static HANDLE hEventInvokeDone = NULL;

PUBFUNC void DispInvokeHandler(void)
{
    DebMsg(DL_INFO, "Entering DispInvokeHandler...");
    if (bDispInvokePending)
    {
        ipb.hr = DispInvoke(ipb._this, ipb.ptinfo, ipb.dispidMember,
            ipb.wFlags, ipb.pparms, ipb.pvarResult, ipb.pexcepinfo, ipb.puArgErr);
        bDispInvokePending = FALSE;
        SetEvent(hEventInvokeDone);
    }
    DebMsg(DL_INFO, "Returning from DispInvokeHandler...");
}

CKermOA * CKermOA::Create(void)
{
    CKermOA * pKermOA;

    DebMsg(DL_INFO, "Creating CKermOA...");

    if ((pKermOA = new CKermOA()) == NULL)
        return NULL;

    if ((pKermOA->m_pITypeInfo = GetKermTypeInfo(IID_IKermOA)) == NULL)
    {
        delete pKermOA;
        return NULL;
    }

    /* Should error check */
    pKermOA->m_pProvideClassInfo = CProvideClassInfo::Create(pKermOA, pKermOA->m_pITypeInfo);

    /* Should error check */
    pKermOA->m_pSupportErrorInfo = CSupportErrorInfo::Create(pKermOA, IID_IKermOA);

    return pKermOA;
}

CKermOA::CKermOA(void)
{
    DebMsg(DL_INFO, "CKermOA creation...");

    m_cRef = 0;
    m_lSound = 0;

    m_pITypeInfo = NULL;

    m_pSupportErrorInfo = NULL;
    m_pProvideClassInfo = NULL;

    g_pKermOA = this;

    {
        SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

        hEventInvokeDone = CreateEvent(&sa, FALSE, FALSE, NULL);
    }

    return;
}

CKermOA::~CKermOA(void)
{
    DebMsg(DL_INFO, "CKermOA destruction...");

    if (hEventInvokeDone != NULL)
    {
        if (!CloseHandle(hEventInvokeDone))
            DebMsg(DL_INFO, "EventInvokeDone handle did not close properly!");
        hEventInvokeDone = NULL;
    }

    if (m_pSupportErrorInfo != NULL)
        DeleteInterfaceImp(m_pSupportErrorInfo);

    if (m_pSupportErrorInfo != NULL)
        DeleteInterfaceImp(m_pProvideClassInfo);

    ReleaseInterface(m_pITypeInfo);

    g_pKermOA = NULL;

    return;
}







STDMETHODIMP CKermOA::QueryInterface(REFIID riid, void FAR * FAR * ppv)
{
    *ppv=NULL;

    if (riid == IID_IUnknown || riid == IID_IDispatch || riid == IID_IKermOA)
    {
        *ppv=this;
        AddRef();
    }
    else if (riid == IID_ISupportErrorInfo && (m_pSupportErrorInfo != NULL))
    {
        *ppv = m_pSupportErrorInfo;
        m_pSupportErrorInfo->AddRef();
    }
    else if (riid == IID_IProvideClassInfo && (m_pProvideClassInfo != NULL))
    {
        *ppv = m_pProvideClassInfo;
        m_pProvideClassInfo->AddRef();
    }
    else
        return ResultFromScode(E_NOINTERFACE);

//    ((IUnknown *)(*ppv))->AddRef();

    return NOERROR;
}

STDMETHODIMP_(ULONG) CKermOA::AddRef(void)
{
    m_cRef++;

    DebMsg(DL_INFO, "CKermOA::AddRef, ref count now %i", m_cRef);

    return m_cRef;
}

STDMETHODIMP_(ULONG) CKermOA::Release(void)
{
    m_cRef--;

    DebMsg(DL_INFO, "CKermOA::Release, ref count now %i", m_cRef);

    if (m_cRef != 0)
        return m_cRef;

    DebMsg(DL_INFO, "CKermOA requesting application termination");

    bAbort = TRUE;
    bAbortConfirm = FALSE;
    nCloseReq = CAR_QUIT;

    delete this;

    return 0L;
}



STDMETHODIMP CKermOA::GetTypeInfoCount(UINT FAR * pctInfo)
{
    DebMsg(DL_INFO, "CKermOA::GetTypeInfoCount()...");

    *pctInfo = 1;

    return NOERROR;
}

STDMETHODIMP CKermOA::GetTypeInfo(UINT itinfo, LCID X(lcid), ITypeInfo FAR * FAR * pptInfo)
{
    DebMsg(DL_INFO, "CKermOA::GetTypeInfo()...");

    *pptInfo = NULL;

    if(itinfo != 0)
        return ResultFromScode(DISP_E_BADINDEX);

    *pptInfo = m_pITypeInfo;
    (*pptInfo)->AddRef();

    return NOERROR;
}

STDMETHODIMP CKermOA::GetIDsOfNames(REFIID X(riid), OLECHAR FAR * FAR * rgszNames, UINT cNames,
                           LCID X(lcid), DISPID FAR * rgDispID)
{
    DebMsg(DL_INFO, "CKermOA::GetIDsOfNames()...");

    return DispGetIDsOfNames(m_pITypeInfo, rgszNames, cNames, rgDispID);
}

STDMETHODIMP CKermOA::Invoke(DISPID dispID, REFIID X(riid), LCID X(lcid), WORD wFlags,
                    DISPPARAMS FAR * pDispParams , VARIANT FAR * pVarResult,
                    EXCEPINFO FAR * pExcepInfo, UINT FAR * puArgErr)
{
    HRESULT hr;

    DebMsg(DL_INFO, "Entering CKermOA::Invoke()...");

    if (GetCurrentThreadId() == dwBaseThreadId)
    {
        hr = DispInvoke(this, m_pITypeInfo, dispID, wFlags, pDispParams,
            pVarResult, pExcepInfo, puArgErr);
    }
    else
    {
        ipb._this = this;
        ipb.ptinfo = m_pITypeInfo;
        ipb.dispidMember = dispID;
        ipb.wFlags = wFlags;
        ipb.pparms = pDispParams;
        ipb.pvarResult = pVarResult;
        ipb.pexcepinfo = pExcepInfo;
        ipb.puArgErr = puArgErr;
        ipb.hr = 0;
        bDispInvokePending = TRUE;

        /* Post a message to ensure main thread handles request promptly??? */

        PostMessage(hAppWnd, WM_USER_INVOKEPENDING, 0, 0);

        if (!WaitForSingleObject(hEventInvokeDone, INFINITE) == WAIT_OBJECT_0)
            DebMsg(DL_INFO, "Wait for EventInvokeDone failed!");

        if (bDispInvokePending)
            DebMsg(DL_INFO, "Wait for InvokeDone completed, but InvokePending still TRUE!");

        hr = ipb.hr;
    }

    DebMsg(DL_INFO, "Leaving CKermOA::Invoke()...");

    return hr;
}

STDMETHODIMP CKermOA::get_Sound(long FAR * pRet)
{
    *pRet = m_lSound;

    return S_OK;
}


STDMETHODIMP CKermOA::put_Sound(long lSound)
{
    if (MB_OK != lSound && MB_ICONEXCLAMATION != lSound
        && MB_ICONQUESTION != lSound && MB_ICONHAND != lSound
        && MB_ICONASTERISK != lSound)
    {
    /*
     * We cannot return error information to the StdDispatch
     * given the way we've defined the return value of this
     * methods, so we can't raise an exception.
     */
        return S_OK;
    }

    m_lSound=lSound;

    return S_OK;
}

STDMETHODIMP CKermOA::Beep(long FAR * pRet)
{
    MessageBeep((UINT)m_lSound);
    *pRet = m_lSound;

    return S_OK;
}


HRESULT UserScriptAbort(void)
{

//#ifdef _WIN32

    ICreateErrorInfo *pcerrinfo;
    IErrorInfo *perrinfo;

    if (SUCCEEDED(CreateErrorInfo(&pcerrinfo)))
    {
        pcerrinfo->SetSource(OLIT("Kermit Automation"));
        pcerrinfo->SetDescription (OLIT("Script aborted at user request"));

        if (SUCCEEDED(pcerrinfo->QueryInterface(IID_IErrorInfo, (LPVOID FAR*) &perrinfo)))
        {
            SetErrorInfo(0, perrinfo);
            perrinfo->Release();
        }

        pcerrinfo->Release();
    }

    return E_ABORT;

//#else

//  return S_OK;

//#endif
}

STDMETHODIMP CKermOA::Write(BSTR bstrText)
{
    char szText[255];

    OLETOANSI(bstrText, szText, sizeof(szText));

    WriteTermFmt(szText);

    return S_OK;
}


STDMETHODIMP CKermOA::Pause(long lTime)
{
    ::Pause(DWORD(lTime) * 100L);

#ifdef AXSCRIPT

    if (bAbort || bScriptAbort)
        return UserScriptAbort();

#endif

    return S_OK;
}

STDMETHODIMP CKermOA::WaitFor(BSTR bstrText, long lTime, long FAR * plRet)
{
    char szText[255];

    OLETOANSI(bstrText, szText, sizeof(szText));

    *plRet = (long)Wait(szText, DWORD(lTime) * 100L);

#ifdef AXSCRIPT

    if (bAbort || bScriptAbort)
        return UserScriptAbort();

#endif

    return S_OK;
}

STDMETHODIMP CKermOA::Send(BSTR bstrText)
{
    char szText[255];

    OLETOANSI(bstrText, szText, sizeof(szText));

    WriteCommStr(szText, lstrlen(szText));

    return S_OK;
}

//STDMETHODIMP CKermOA::Application(IKermOA FAR * FAR * pRet)
//{
//  *pRet = this;
//  (*pRet)->AddRef();

//  return S_OK;
//}

STDMETHODIMP
CKermOA::get_Application(IKermOA FAR* FAR* ppIKermOA)
{
    *ppIKermOA = NULL;

    return QueryInterface(IID_IDispatch, (void FAR* FAR*)ppIKermOA);
}

STDMETHODIMP
CKermOA::get_FullName(BSTR FAR* pbstr)
{
    *pbstr = SysAllocString(OLIT("Kermit for Windows"));

    return S_OK;
}

STDMETHODIMP
CKermOA::get_Name(BSTR FAR* pbstr)
{
    *pbstr = SysAllocString(OLIT("Kermit"));

    return S_OK;
}

STDMETHODIMP
CKermOA::get_Parent(IKermOA FAR* FAR* ppIKermOA)
{
    *ppIKermOA = NULL;

    return QueryInterface(IID_IDispatch, (void FAR* FAR*)ppIKermOA);
}

STDMETHODIMP
CKermOA::put_Visible(boolean bVisible)
{
    if (bVisible == bAppVisible)
        return S_OK;

    ShowWindow(hAppWnd, bVisible ? SW_SHOW : SW_HIDE);
    bVisible ? AddRef() : Release();
    bAppVisible = bVisible;

    return S_OK;
}

STDMETHODIMP
CKermOA::get_Visible(boolean * pbVisible)
{
    *pbVisible = (boolean)bAppVisible;

    return S_OK;
}

STDMETHODIMP
CKermOA::Quit()
{
    bAbort = TRUE;
    bAbortConfirm = FALSE;
    nCloseReq = CAR_QUIT;

    return NOERROR;
}