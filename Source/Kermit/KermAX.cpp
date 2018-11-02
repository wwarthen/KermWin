/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMAX.C                                     **
**                                                                            **
**  This module contains the ActiveX script interface.                        **
**                                                                            **
*******************************************************************************/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include "kermtloc.h"

#include <sys/types.h>
#include <sys/stat.h>

//#include "KermOLE.h"
#include "DKermOA.h"
#include <ActivDbg.h>

/* CONSTANTS -----------------------------------------------------------------*/

#define AXS_CANCEL  FALSE
#define AXS_RUN     1
#define AXS_EDIT    2
#define AXS_BROWSE  3

#define MAX_SCRIPTS 10

/* EMULATION DLL FUNCTION TABLE INDEXES --------------------------------------*/


/* EMULATION DLL FUNCTION DEFINITIONS ----------------------------------------*/


/* EMULATION DLL FUNCTION CALL MACROS ----------------------------------------*/

#if 0

//---------------------------------------------------------------------------
// the CDebugDocument class
//---------------------------------------------------------------------------
class CDebugDocument :
//public IDebugDocumentInfo,
public IDebugDocumentProvider,
//public IDebugDocument,
public IDebugDocumentText,
public IDebugDocumentTextEvents,
//public IDebugDocumentTextAuthor
public IDebugDocumentContext
{

public:
    CDebugDocument(void);
    ~CDebugDocument(void);

    static CDebugDocument * Create(void);

    // *** IUnknown methods ***
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    // *** IDebugDocumentInfo methods ***
    STDMETHOD(GetName)(DOCUMENTNAMETYPE dnt, BSTR __RPC_FAR *pbstrName);
    STDMETHOD(GetDocumentClassId)(CLSID __RPC_FAR *pclsidDocument);

    // *** IDebugDocumentProvider methods ***
    STDMETHOD(GetDocument)(IDebugDocument __RPC_FAR *__RPC_FAR *ppssd);

    // *** IDebugDocument methods ***
//  STDMETHOD(?)(?);

    // *** IDebugDocumentText methods ***
    STDMETHOD(GetDocumentAttributes)(TEXT_DOC_ATTR __RPC_FAR *ptextdocattr);
    STDMETHOD(GetSize)(ULONG __RPC_FAR *pcNumLines, ULONG __RPC_FAR *pcNumChars);
    STDMETHOD(GetPositionOfLine)(ULONG cLineNumber, ULONG __RPC_FAR *pcCharacterPosition);
    STDMETHOD(GetLineOfPosition)(ULONG cCharacterPosition, ULONG __RPC_FAR *pcLineNumber, ULONG __RPC_FAR *pcCharacterOffsetInLine);
    STDMETHOD(GetText)(ULONG cCharacterPosition, WCHAR __RPC_FAR *pcharText, SOURCE_TEXT_ATTR __RPC_FAR *pstaTextAttr, ULONG __RPC_FAR *pcNumChars, ULONG cMaxChars);
    STDMETHOD(GetPositionOfContext)(IDebugDocumentContext __RPC_FAR *psc, ULONG __RPC_FAR *pcCharacterPosition, ULONG __RPC_FAR *cNumChars);
    STDMETHOD(GetContextOfPosition)(ULONG cCharacterPosition, ULONG cNumChars, IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc);

    // *** IDebugDocumentTextEvents methods ***
    STDMETHOD(onDestroy)(void);
    STDMETHOD(onInsertText)(ULONG cCharacterPosition, ULONG cNumToInsert);
    STDMETHOD(onRemoveText)(ULONG cCharacterPosition, ULONG cNumToRemove);
    STDMETHOD(onReplaceText)(ULONG cCharacterPosition, ULONG cNumToReplace);
    STDMETHOD(onUpdateTextAttributes)(ULONG cCharacterPosition, ULONG cNumToUpdate);
    STDMETHOD(onUpdateDocumentAttributes)(TEXT_DOC_ATTR textdocattr);

    // *** IDebugDocumentTextAuthor methods ***
    STDMETHOD(InsertText)(ULONG cCharacterPosition, ULONG cNumToInsert, OLECHAR __RPC_FAR pcharText[]);
    STDMETHOD(RemoveText)(ULONG cCharacterPosition, ULONG cNumToRemove);
    STDMETHOD(ReplaceText)(ULONG cCharacterPosition, ULONG cNumToReplace, OLECHAR __RPC_FAR pcharText[]);

    // *** IDebugDocumentContext methods ***
//  STDMETHOD(GetDocument)(IDebugDocument __RPC_FAR *__RPC_FAR *ppsd);
    STDMETHOD(EnumCodeContexts)(IEnumDebugCodeContexts __RPC_FAR *__RPC_FAR *ppescc);

    // *** Member Variables ***
    UINT                    m_cref;
};

CDebugDocument dd;

#endif

//---------------------------------------------------------------------------
// the CScriptSite class
//---------------------------------------------------------------------------
class CScriptSite : public IActiveScriptSite, public IActiveScriptSiteWindow,
                    public IActiveScriptSiteDebug
{

public:
    CScriptSite(void);
    ~CScriptSite(void);

    static CScriptSite * Create(void);

    BOOL ScriptSiteOpen(void);
    BOOL ScriptSiteClose(void);
    void ScriptLoad(char *, char *);
    void ScriptStop(void);

    // *** IUnknown methods ***
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    // *** IActiveScriptSite methods ***
    STDMETHOD(GetLCID)(LCID *plcid);
    STDMETHOD(GetItemInfo)(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti);
    STDMETHOD(GetDocVersionString)(BSTR *pszVersion);
    STDMETHOD(OnScriptTerminate)(const VARIANT *pvarResult, const EXCEPINFO *pexcepinfo);
    STDMETHOD(OnStateChange)(SCRIPTSTATE ssScriptState);
    STDMETHOD(OnScriptError)(IActiveScriptError *pscripterror);
    STDMETHOD(OnEnterScript)(void);
    STDMETHOD(OnLeaveScript)(void);

    // *** IActiveScriptSiteWindow methods ***
    STDMETHOD(GetWindow)(HWND *phwnd);
    STDMETHOD(EnableModeless)(BOOL fEnable);

    // *** IActiveScriptSiteDebug methods ***
    STDMETHOD(GetDocumentContextFromPosition)(DWORD dwSourceContext, ULONG uCharacterOffset, ULONG uNumChars, IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc);
    STDMETHOD(GetApplication)(IDebugApplication __RPC_FAR *__RPC_FAR *ppda);
    STDMETHOD(GetRootApplicationNode)(IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdanRoot);
    STDMETHOD(OnScriptErrorDebug)(IActiveScriptErrorDebug __RPC_FAR *pErrorDebug, BOOL __RPC_FAR *pfEnterDebugger, BOOL __RPC_FAR *pfCallOnScriptErrorWhenContinuing);

    // *** Member Variables ***
    UINT                    m_cRef;
    IActiveScript           * m_ps;
    IActiveScriptParse      * m_psp;
    CLSID                   m_clsidEngine;
    PATHNAM                 m_szScriptFile [MAX_SCRIPTS];
    IDebugDocumentHelper    * m_pddh [MAX_SCRIPTS];
    DWORD                   dwDocContext [MAX_SCRIPTS];
    DWORD                   m_iddh;
};

//---------------------------------------------------------------------------
// Forward Declares
//---------------------------------------------------------------------------
int DisplayScriptError(HINSTANCE hinst, HWND hwndParent, LPCSTR pszError, int ichError);


/* LOCAL STORAGE -------------------------------------------------------------*/

#define     SC_NONE         0
#define     SC_READY        1
#define     SC_LOADFILE     2
#define     SC_LOADIMMED    3
#define     SC_EXEC         4
#define     SC_QUIT         5

static PATHNAM                  g_szScriptFile = "\0";
static char                     szAXStatus [80] = "Script Status Unknown";
static CScriptSite *            g_pkerm;
static char *                   g_szScriptEngine = "VBScript";
// static char *                g_szScriptEngine = "JScript";
static IProcessDebugManager *   g_ppdm = NULL;
static IDebugApplication *      g_pda = NULL;
static DWORD                    g_dwAppCookie = 0;
static DWORD                    g_dwScriptDepth = 0;
static char                     g_szImmediateCode [1024] = "\0";
static HANDLE                   g_hScriptAbortThread;
static HANDLE                   g_hSiteThreadProc;
static HWND                     g_hScriptAbortWnd;
static char                     g_iSiteCmd = SC_NONE;
static char *                   g_lpszSiteCmdData;
static HANDLE                   g_hEventSite;


//---------------------------------------------------------------------------
// Local support functions
//---------------------------------------------------------------------------

BOOL PRVFUNC CreateScriptSite(void)
{
    g_pkerm = CScriptSite::Create();

    if (g_pkerm == NULL)
        return FALSE;

    if (!g_pkerm->ScriptSiteOpen())
    {
        g_pkerm->ScriptSiteClose();
        ReleaseInterface(g_pkerm);
    }

    return TRUE;
}

BOOL PRVFUNC DestroyScriptSite(void)
{
    ASSERT(g_pkerm != NULL);

    if (g_pkerm != NULL)
    {
        g_pkerm->ScriptSiteClose();
        ReleaseInterface(g_pkerm)
    }

    ASSERT(g_pkerm == NULL);

    return TRUE;
}

BOOL PRVFUNC FindScriptFile(PSTR pszScriptFile, PSTR pszScriptFileName)
{
    char * pszFilePart;
    int nResult;

    /* Look for script file in same directory as session file (if possible) */
    nResult = 0;
    if (szSessFileName[0] != '\0')
    {
        PATHNAM szSessPath;

        lstrcpy(szSessPath, szSessFileName);
        *(strrchr(szSessFileName, '\\') + 1) = '\0';
        nResult = SearchPath(szSessPath, pszScriptFile, ".ksc", sizeof(PATHNAM),
                             pszScriptFileName, &pszFilePart);
    }

    /* Look for script file in standard places, exe dir and system dirs */
    if (nResult == 0)
        nResult = SearchPath(NULL, pszScriptFile, ".ksc", sizeof(PATHNAM),
                             pszScriptFileName, &pszFilePart);

    if (nResult == 0)
        return FALSE;

    return TRUE;
}


BOOL PRVFUNC LoadScriptCode(LPSTR pszStatement)
{
    if (g_pkerm == NULL)
    {
        KermitFmtMsgBox(MB_OK, "Can not execute script statements!\n\n"
            "ActiveX scripting engine %s is not installed!", (LPSTR)g_szScriptEngine);

        return FALSE;
    }

    g_pkerm->ScriptLoad(pszStatement, NULL);

    return TRUE;
}

BOOL PRVFUNC LoadScriptFile(LPSTR pszScriptFile)
{
    BOOL        bResult;
    char *      pszScript;
    PATHNAM     szScriptFileName;
    int         nFileLen, nBytesRead;
    FILE *      pFile;

    bResult = TRUE;

    DebMsg(DL_INFO, "LoadScriptFile: '%s'", (LPSTR)pszScriptFile);

    if (g_pkerm == NULL)
    {
        DebMsg(DL_INFO, "Scripting support not available");

        KermitFmtMsgBox(MB_OK, "Can not execute script '%s'!\n\n"
            "ActiveX scripting engine %s is not installed!",
            (LPSTR)pszScriptFile, (LPSTR)g_szScriptEngine);

        return FALSE;
    }

    if (!FindScriptFile(pszScriptFile, szScriptFileName))
    {
        DebMsg(DL_INFO, "Unable to locate script file");

        KermitFmtMsgBox(MB_OK, "Can't locate script file %s!", (LPSTR)pszScriptFile);

        return FALSE;
    }

    DebMsg(DL_INFO, "Expanded file name is '%s'", (LPSTR)szScriptFileName);

    if (fopen_s(&pFile, szScriptFileName, "rb") != 0)
    {
        DebMsg(DL_INFO, "Error opening script file, errno = %i", errno);

        KermitFmtMsgBox(MB_OK, "Can't open script file: %s", (LPSTR)szScriptFileName);

        return FALSE;
    }

    nFileLen = _filelength(_fileno(pFile));

    if (nFileLen != -1)
    {
        pszScript = new char[nFileLen + 1];

        if (pszScript != NULL)
        {
            memset(pszScript, 0, nFileLen + 1);

            nBytesRead = fread(pszScript, 1, nFileLen, pFile);

            DebMsg(DL_INFO, "Script file size is %ul, read %ul bytes", nFileLen, nBytesRead);

            if (nBytesRead == nFileLen)
            {
                g_pkerm->ScriptLoad(pszScript, szScriptFileName);
            }
            else
            {
                DebMsg(DL_INFO, "Script file bytes read not equal to bytes requested");
                bResult = FALSE;
            }

            delete [] pszScript;
        }
        else
        {
            DebMsg(DL_INFO, "Error allocating buffer to read script file contents");
            bResult = FALSE;
        }

    }
    else
    {
        DebMsg(DL_INFO, "Error getting script file length");
        bResult = FALSE;
    }

    fclose(pFile);

    return TRUE;
}

BOOL PRVFUNC ExecScriptFunc(LPSTR lpszFuncName)
{
    HRESULT hr;
    IDispatch * pIScriptDispatch;

    hr = g_pkerm->m_ps->GetScriptDispatch(NULL, &pIScriptDispatch);

    if (SUCCEEDED(hr))
    {
        DISPID DispID;

        OLECHAR * pwszFuncName = new OLECHAR [lstrlen(lpszFuncName) + 1];
        ANSITOOLE(lpszFuncName, pwszFuncName, lstrlen(lpszFuncName) + 1);

        hr = pIScriptDispatch->GetIDsOfNames(IID_NULL, &pwszFuncName, 1,
            LOCALE_SYSTEM_DEFAULT, &DispID);

        if (SUCCEEDED(hr))
        {
            DISPPARAMS dp;
            VARIANTARG va;
            EXCEPINFO exInfo;
            UINT uErr;

            dp.cArgs = 0;
            dp.rgvarg = 0;
            dp.cNamedArgs = 0;
            dp.rgdispidNamedArgs = NULL;

            hr = pIScriptDispatch->Invoke(DispID, IID_NULL, LOCALE_SYSTEM_DEFAULT,
                DISPATCH_METHOD, &dp, &va, &exInfo, &uErr);

            if (FAILED(hr))
                DebMsg(DL_INFO, "ExecScriptFunc:Invoke '%s' failed, result=%lX", (LPSTR)lpszFuncName, hr);
        }
        else
            DebMsg(DL_INFO, "ExecScriptFunc:GetIDsOfNames '%s' failed, result=%lX", (LPSTR)lpszFuncName, hr);

        delete [] pwszFuncName;

        pIScriptDispatch->Release();
    }
    else
        DebMsg(DL_INFO, "ExecScriptFunc:GetScriptDispatch '%s' failed, result=%lX", (LPSTR)lpszFuncName, hr);

    return SUCCEEDED(hr);
}

DWORD WINAPI SiteThreadProc(LPVOID dwParentThreadId)
{
    HRESULT hr;

    DebMsg(DL_INFO, "SiteThreadProc() running, ThreadId=%lu...", GetCurrentThreadId());

    hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        if (!AttachThreadInput(GetCurrentThreadId(), (DWORD)dwParentThreadId, TRUE))
            DebMsg(DL_INFO, "Thread AttachThreadInput() failed!");

        if (CreateScriptSite())
        {
            g_iSiteCmd = SC_READY;

            do
            {
                if (!WaitForSingleObject(g_hEventSite, INFINITE) == WAIT_OBJECT_0)
                    DebMsg(DL_INFO, "Wait for EventSite failed!");

                switch (g_iSiteCmd)
                {
                case SC_LOADFILE:
                    DebMsg(DL_INFO, "Script thread procesing load script command...");
                    LoadScriptFile(g_lpszSiteCmdData);
                    g_iSiteCmd = SC_READY;
                    break;

                case SC_LOADIMMED:
                    DebMsg(DL_INFO, "Script thread procesing load immediate command...");
                    LoadScriptCode(g_lpszSiteCmdData);
                    g_iSiteCmd = SC_READY;
                    break;

                case SC_EXEC:
                    DebMsg(DL_INFO, "Script thread procesing execute function command...");
                    ExecScriptFunc(g_lpszSiteCmdData);
                    g_iSiteCmd = SC_READY;
                    break;

                case SC_QUIT:
                    DebMsg(DL_INFO, "Script thread procesing quit command...");
                    break;

                default:
                    break;
                }
            }
            while (g_iSiteCmd != SC_QUIT);

            DestroyScriptSite();
        }
        else
            DebMsg(DL_INFO, "SiteThreadProc CreateScriptSite() failed");
    }
    else
        DebMsg(DL_INFO, "SiteThreadProc:CoInitialize() failed, result = %lX, closing thread...", hr);

    CoUninitialize();

    DebMsg(DL_INFO, "SiteThreadProc() terminating, ThreadId=%lu...", GetCurrentThreadId());

    return 0;
}

//---------------------------------------------------------------------------
// Exposed scripting interface functions
//---------------------------------------------------------------------------

PSTR PUBFUNC AXStatus(void)
{
    return szAXStatus;
}

BOOL PUBFUNC AXInit(void)
{
    HRESULT hr;

    // Create process debug manager...

    DebMsg(DL_INFO, "Creating process debug manager (CoCreateInstance)...");

    hr = CoCreateInstance(CLSID_ProcessDebugManager, NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER,
        IID_IProcessDebugManager, (void **)&g_ppdm);

    if (SUCCEEDED(hr))
    {
        char szBuf[128];

        GetCLSIDDesc(CLSID_ProcessDebugManager, szBuf);
        DebMsg(DL_INFO, "Process Debug Manager: %s", (LPSTR)szBuf);
    }
    else
    {
        g_ppdm = NULL;
        DebMsg(DL_INFO, "CoCreate of CLSID_ProcessDebugManager failed!");
    }

    // Create debug application...

    if (g_ppdm != NULL)
    {
        DebMsg(DL_INFO, "Creating debug application...");

        hr = g_ppdm->CreateApplication(&g_pda);

        if (FAILED(hr))
        {
            g_pda = NULL;
            DebMsg(DL_INFO, "CreateApplication failed!");
        }
    }

    // Set name of application being debugged...

    if (g_pda != NULL)
    {
        DebMsg(DL_INFO, "SetName for debug application...");

        hr = g_pda->SetName(OLIT("Kermit for Windows"));

        if (FAILED(hr))
            DebMsg(DL_INFO, "SetName failed!");
    }

    // Add application to debugger...

    if ((g_ppdm != NULL) && (g_pda != NULL))
    {
        DebMsg(DL_INFO, "AddApplication...");

        hr = g_ppdm->AddApplication(g_pda, &g_dwAppCookie);

        if (FAILED(hr))
            DebMsg(DL_INFO, "AddApplication failed!");
    }

    // Create an event used to signal when site script thread should wake up

    {
        SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

        g_hEventSite = CreateEvent(&sa, FALSE, FALSE, NULL);
    }

    // Start the script site running as a new thread
    DWORD dwThreadId;

    g_iSiteCmd = SC_NONE;
    g_hSiteThreadProc = CreateThread(NULL, 0, &SiteThreadProc, (LPVOID)GetCurrentThreadId(), 0, &dwThreadId);
    if (g_hSiteThreadProc == NULL)
        DebMsg(DL_INFO, "Error starting new Script Site thread!");
    else
        DebMsg(DL_INFO, "Site Script thread started, id=%lu", dwThreadId);

    while (g_iSiteCmd != SC_READY)
    {
        MessagePump();
        WaitMessage();
    }

    return TRUE;
}

BOOL PUBFUNC AXTerm(void)
{
    DebMsg(DL_INFO, "Terminating Script Site thread...");

    if (bScript)
    {
        bScriptAbort = TRUE;
        AXScriptAbort();
    }

    while (g_iSiteCmd != SC_READY)
        Sleep(100);

    g_iSiteCmd = SC_QUIT;
    SetEvent(g_hEventSite);

    if (g_hSiteThreadProc != NULL)
    {
        if (WaitForSingleObject(g_hSiteThreadProc, 1000) != WAIT_OBJECT_0)
            DebMsg(DL_INFO, "SiteThreadProc did not terminate!");
        CloseHandle(g_hSiteThreadProc);
    }

    if (g_hEventSite != NULL)
    {
        if (!CloseHandle(g_hEventSite))
            DebMsg(DL_INFO, "EventSite handle did not close properly!");
        g_hEventSite = NULL;
    }

    DebMsg(DL_INFO, "Removing debug application...");

    if (g_ppdm != NULL)
    {
        g_ppdm->RemoveApplication(g_dwAppCookie);
        g_dwAppCookie = 0;
    }

    DebMsg(DL_INFO, "Releasing debug application interface...");

    ASSERT(g_pda != NULL);

    if (g_pda != NULL)
        ReleaseInterface(g_pda);

    ASSERT(g_pda == NULL);

    DebMsg(DL_INFO, "Releasing debug document manager interface...");

    ASSERT(g_ppdm != NULL);

    if (g_ppdm != NULL)
        ReleaseInterface(g_ppdm);

    ASSERT(g_ppdm == NULL);

    return TRUE;
}

BOOL PUBFUNC AXScriptLoadFile(LPSTR lpszScriptFileName)
{
    if (g_iSiteCmd != SC_READY)
    {
        DebMsg(DL_INFO, "AXScriptLoadFile failed (script site not in ready state)");
        return FALSE;
    }

    g_lpszSiteCmdData = lpszScriptFileName;
    g_iSiteCmd = SC_LOADFILE;
    SetEvent(g_hEventSite);

    return TRUE;
}

BOOL PUBFUNC AXScriptLoadImmed(LPSTR lpszScriptCode)
{
    if (g_iSiteCmd != SC_READY)
    {
        DebMsg(DL_INFO, "AXScriptLoadImmed failed (script site not in ready state)");
        return FALSE;
    }

    g_lpszSiteCmdData = lpszScriptCode;
    g_iSiteCmd = SC_LOADIMMED;
    SetEvent(g_hEventSite);

    return TRUE;
}

BOOL PUBFUNC AXScriptExec(LPSTR lpszScriptFunc)
{
    if (g_iSiteCmd != SC_READY)
    {
        DebMsg(DL_INFO, "AXScriptExec failed (script site not in ready state)");
        return FALSE;
    }

    g_lpszSiteCmdData = lpszScriptFunc;
    g_iSiteCmd = SC_EXEC;
    SetEvent(g_hEventSite);

    return TRUE;
}

VOID PUBFUNC AXScriptAbort(VOID)
{
    if (g_pkerm != NULL)
        g_pkerm->ScriptStop();
}

BOOL PUBFUNC AXScriptDebugStart(void)
{
    HRESULT hr;

    if (g_pda == NULL)
        return FALSE;

    DebMsg(DL_INFO, "StartDebugSession()...");

    hr = g_pda->StartDebugSession();
    if (FAILED(hr))
        KermitFmtMsgBox(MB_OK, "StartDebugSession failed!");

    return TRUE;
}

BOOL PUBFUNC AXScriptDebugBreak(void)
{
    HRESULT hr;

    if (g_pda == NULL)
        return FALSE;

    DebMsg(DL_INFO, "CauseBreak()...");

    hr = g_pda->CauseBreak();
    if (FAILED(hr))
        KermitFmtMsgBox(MB_OK, "CauseBreak() failed!");

    return TRUE;
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export ScriptDlgProc(HWND hDlg, unsigned message,
                                     UINT wParam, LONG X(lParam))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    switch (message)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDD_RUN:
            Edit_GetText(GetDlgItem(hDlg, IDD_PARMS), g_szScriptFile, sizeof(g_szScriptFile));
            EndDialog(hDlg, AXS_RUN);
            break;

        case IDD_EDIT:
            Edit_GetText(GetDlgItem(hDlg, IDD_PARMS), g_szScriptFile, sizeof(g_szScriptFile));
            EndDialog(hDlg, AXS_EDIT);
            break;

        case IDD_CANCEL:
            EndDialog(hDlg, AXS_CANCEL);
            break;

        case IDD_BROWSE:
            Edit_GetText(GetDlgItem(hDlg, IDD_PARMS), g_szScriptFile, sizeof(g_szScriptFile));
            EndDialog(hDlg, AXS_BROWSE);
            break;

        default:
            return(FALSE);
            break;
        }
        break;

        case WM_INITDIALOG:
            Edit_LimitText(GetDlgItem(hDlg, IDD_PARMS), sizeof(g_szScriptFile));
            Edit_SetText(GetDlgItem(hDlg, IDD_PARMS), g_szScriptFile);
            break;

        default:
            return(FALSE);
    }

    return(TRUE);
}

BOOL PRVFUNC ScriptDialogBrowse(PSTR pszScriptFileName)
{
    OPENFILENAME ofn;
    PATHNAM     szWorkFileName2;

    lstrcpy(szWorkFileName2, pszScriptFileName);

    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hAppWnd;
    ofn.lpstrFilter = "Kermit Script Files(*.ksc)\0*.ksc\0"
                      "All Files(*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szWorkFileName2;
    ofn.nMaxFile = sizeof(PATHNAM);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "ksc";
    ofn.lpstrTitle = "Choose Script";

    if (!GetOpenFileName(&ofn))
    {
        if (CommDlgExtendedError() != FNERR_INVALIDFILENAME)
            return FALSE;

        szWorkFileName2[0] = '\0';
        if (!GetOpenFileName(&ofn))
            return FALSE;
    }

    lstrcpy(pszScriptFileName, szWorkFileName2);
    return TRUE;
}

BOOL PUBFUNC AXScriptDialog(VOID)
{
    if (bScript)
    {
        bScriptAbort = TRUE;
        AXScriptAbort();
        return TRUE;
    }

    for (;;)
    {
        switch (GoDialogBox(hAppInst, "ScriptDlgBox", hAppWnd, (FARPROC)ScriptDlgProc))
        {
        case AXS_CANCEL:
            return TRUE;

        case AXS_BROWSE:
            ScriptDialogBrowse(g_szScriptFile);
            break;

        case AXS_EDIT:
            {
                PATHNAM szTempFileName;
                PATHNAM szExecStr;

                lstrcpy(szTempFileName, g_szScriptFile);
                StrCatDefExt(szTempFileName, ".ksc");
                wsprintf(szExecStr, "NotePad %s", (LPSTR)szTempFileName);
                WinExec(szExecStr, SW_SHOWNORMAL);
            }
            break;

        case AXS_RUN:
            return AXScriptLoadFile(g_szScriptFile);
            break;
        }
    }
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export ExecDlgProc(HWND hDlg, unsigned message,
                                   UINT wParam, LONG X(lParam))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    switch (message)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDD_RUN:
            GetDlgItemText(hDlg, IDD_PARMS, g_szImmediateCode,
                sizeof(g_szImmediateCode));
            EndDialog(hDlg, TRUE);
            break;

        case IDD_CANCEL:
            EndDialog(hDlg, FALSE);
            break;

        default:
            return FALSE;
            break;
        }
        break;

        case WM_INITDIALOG:
            Edit_LimitText(GetDlgItem(hDlg, IDD_PARMS), sizeof(g_szImmediateCode));
            Edit_SetText(GetDlgItem(hDlg, IDD_PARMS), g_szImmediateCode);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

BOOL PUBFUNC AXExecDialog(VOID)
{
    if (bScript)
    {
        bScriptAbort = TRUE;
        AXScriptAbort();
        return TRUE;
    }

    if (GoDialogBox(hAppInst, "ExecDlgBox", hAppWnd, (FARPROC)ExecDlgProc))
        return AXScriptLoadImmed(g_szImmediateCode);

    return TRUE;
}

LRESULT CALLBACK __export ScriptAbortWndProc(HWND hWnd, UINT message,
                                             WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    case WM_CHAR:
        if (wParam == 27)
        {
            if (g_pkerm != NULL)
            {
                DebMsg(DL_INFO, "Invoking ScriptStop()...");
                g_pkerm->ScriptStop();
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return(DefWindowProc(hWnd, message, wParam, lParam));
    }

    return 0L;
}

BOOL CScriptSite::ScriptSiteOpen(void)
{
    HRESULT hr;

    // Look up the CLSID for the desired script engine...

    OLECHAR * pwszScriptEngine = new OLECHAR [lstrlen(g_szScriptEngine) + 1];
    ANSITOOLE(g_szScriptEngine, pwszScriptEngine, lstrlen(g_szScriptEngine) + 1);

    hr = CLSIDFromProgID(pwszScriptEngine, &m_clsidEngine);

    delete [] pwszScriptEngine;

    if FAILED(hr)
    {
        wsprintf(szAXStatus, "AXScript: Script engine '%s' unavailable, %lX", (LPSTR)g_szScriptEngine, hr);
        ScriptSiteClose();
        return FALSE;
    }

    // Create an instance of the engine...

    DebMsg(DL_INFO, "Creating scripting engine (CoCreateInstance)...");

    hr = CoCreateInstance(m_clsidEngine, NULL,
        CLSCTX_INPROC_SERVER, IID_IActiveScript,
        (void **)&m_ps);

    if (SUCCEEDED(hr))
    {
        char szBuf[128];

        GetCLSIDDesc(m_clsidEngine, szBuf);
        DebMsg(DL_INFO, "Script Engine: %s", (LPSTR)szBuf);
    }
    else
    {
        // This is OK, it just means that the user has not installed script engine!
        wsprintf(szAXStatus, "AXScript Init Failed (CoCreateInstance() = %lX)", hr);
        ScriptSiteClose();
        return FALSE;
    }

    // Script Engine must support IActiveScriptParse for us to use it...

    DebMsg(DL_INFO, "Checking for ActiveScriptParse interface...");

    hr = m_ps->QueryInterface(IID_IActiveScriptParse, (void **)&m_psp);
    if FAILED(hr)
    {
        wsprintf(szAXStatus, "AXScript Init Failed (QueryInterface(ASParse) = %lX)", hr);
        ScriptSiteClose();
        return FALSE;
    }

    // Set the script site...

    DebMsg(DL_INFO, "SetScriptSite...");

    hr = m_ps->SetScriptSite(this);
    if FAILED(hr)
    {
        wsprintf(szAXStatus, "AXScript Init Failed (SetScriptSite() = %lX)", hr);
        ScriptSiteClose();
        return FALSE;
    }

    // InitNew the object...

    DebMsg(DL_INFO, "InitNew()...");

    hr = m_psp->InitNew();
    if FAILED(hr)
    {
        wsprintf(szAXStatus, "AXScript Init Failed (InitNew() = %lX)", hr);
        ScriptSiteClose();
        return FALSE;
    }

    // Add named item...

    DebMsg(DL_INFO, "AddNamedItem()...");

    hr = m_ps->AddNamedItem(OLIT("Kermit"), SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE);
    if FAILED(hr)
    {
        wsprintf(szAXStatus, "AXScript Init Failed (AddNamedItem() = %lX)", hr);
        ScriptSiteClose();
        return FALSE;
    }

    GetCLSIDDesc(m_clsidEngine, szAXStatus);

    return TRUE;
}

BOOL CScriptSite::ScriptSiteClose(void)
{
    DebMsg(DL_INFO, "Detaching/Releasing all debug document helpers...");

    while (m_iddh > 0)
    {
        m_iddh--;

        ASSERT(m_pddh[m_iddh] != NULL);

        m_pddh[m_iddh]->Detach();
        ReleaseInterface(m_pddh[m_iddh]);

        ASSERT(m_pddh[m_iddh] == NULL);
    }

    DebMsg(DL_INFO, "Releasing script parse interface...");

    ASSERT(m_psp != NULL);

    if (m_psp != NULL)
        ReleaseInterface(m_psp);

    ASSERT(m_psp == NULL);

    DebMsg(DL_INFO, "Closing/Releasing script engine interface...");

    ASSERT(m_ps != NULL);

    if (m_ps != NULL)
    {
        m_ps->SetScriptState(SCRIPTSTATE_DISCONNECTED);
        m_ps->Close();
        ReleaseInterface(m_ps);
    }

    ASSERT(m_ps == NULL);

    return TRUE;
}

void CScriptSite::ScriptLoad(char * pszScript, char * pszScriptFile)
{
    DWORD dwSourceContext;
    HRESULT hr;
    EXCEPINFO ei;
    OLECHAR * pwszScript;

    // Check for max scripts...

    if (m_iddh >= MAX_SCRIPTS)
    {
        KermitFmtMsgBox(MB_OK, "Maximum scripts already loaded!");
        return;
    }

    // Convert script to Unicode...

    pwszScript = new OLECHAR [lstrlen(pszScript) + 1];
    ANSITOOLE(pszScript, pwszScript, lstrlen(pszScript) + 1);

    // Record script file name...

    if (pszScriptFile == NULL)
        lstrcpy(m_szScriptFile[m_iddh], "Immediate Code");
    else
        lstrcpy(m_szScriptFile[m_iddh], pszScriptFile);

    // Create debug document helper...

    if (g_ppdm != NULL)
    {
        DebMsg(DL_INFO, "CreateDebugDocumentHelper()...");

        hr = g_ppdm->CreateDebugDocumentHelper(NULL, &m_pddh[m_iddh]);
        if (FAILED(hr))
        {
            KermitFmtMsgBox(MB_OK, "CreateDebugDocumentHelper failed!");
            m_pddh[m_iddh] = NULL;
        }
    }

    // Debug document helper init...

    if (m_pddh[m_iddh] != NULL)
    {
        if (pszScriptFile == NULL)
        {
            hr = m_pddh[m_iddh]->Init(g_pda, OLIT("Immediate Code"), OLIT("Immediate Code"),
                TEXT_DOC_ATTR_READONLY | DEBUG_TEXT_ALLOWBREAKPOINTS | DEBUG_TEXT_ALLOWERRORREPORT);
        }
        else
        {
            char * pszShortScriptFileName;

            pszShortScriptFileName = _fstrrchr(pszScriptFile, '\\');
            if (pszShortScriptFileName == NULL)
                pszShortScriptFileName = pszScriptFile;
            else
                pszShortScriptFileName++;

            OLECHAR * pwszShortName = new OLECHAR [lstrlen(pszShortScriptFileName) + 1];
            OLECHAR * pwszLongName = new OLECHAR [lstrlen(pszScriptFile) + 1];

            if (ANSITOOLE(pszShortScriptFileName, pwszShortName, lstrlen(pszShortScriptFileName) + 1) == 0)
                DebMsg(DL_INFO, "Error %lu in ShortName ANSITOOLE", GetLastError());
            if (ANSITOOLE(pszScriptFile, pwszLongName, lstrlen(pszScriptFile) + 1) == 0)
                DebMsg(DL_INFO, "Error %lu in LongName ANSITOOLE", GetLastError());

            DebMsg(DL_INFO, "DebugDocumentHelper Init()...");

            hr = m_pddh[m_iddh]->Init(g_pda, pwszShortName, pwszLongName,
                TEXT_DOC_ATTR_READONLY | DEBUG_TEXT_ALLOWBREAKPOINTS | DEBUG_TEXT_ALLOWERRORREPORT);
            //          TEXT_DOC_ATTR_READONLY);

            delete [] pwszShortName;
            delete [] pwszLongName;

        }

        if (FAILED(hr))
        {
            DebMsg(DL_INFO, "Debug document helper init failed!");
            ReleaseInterface(m_pddh[m_iddh]);
        }
    }

    // Attach debug document...

    if (m_pddh[m_iddh] != NULL)
    {
        DebMsg(DL_INFO, "DebugDocumentHelper Attach()...");

        hr = m_pddh[m_iddh]->Attach(NULL);

        if (FAILED(hr))
        {
            DebMsg(DL_INFO, "DebugDocumentHelper Attach() failed!");
            ReleaseInterface(m_pddh[m_iddh]);
        }
    }

    // Add debug document text...

    if (m_pddh[m_iddh] != NULL)
    {
        DebMsg(DL_INFO, "DebugDocumentHelper AddUnicodeText()...");

        hr = m_pddh[m_iddh]->AddUnicodeText(pwszScript);
        if (FAILED(hr))
        {
            DebMsg(DL_INFO, "AddUnicodeText failed!");
            m_pddh[m_iddh]->Detach();
            ReleaseInterface(m_pddh[m_iddh]);
        }
    }

    // Define the script block from the source text...

    /* Normally, dwSourceContext is used to handle multiple script blocks within
    a single document.  Kermit script documents do not have multiple distinct
    script blocks within a single file/document.  So, we ignore the dwSourceContext
    returned here (assume it is 0), the we use the dwSourceContext field of
    ParseScriptText to pass an our pddh index. */

    dwSourceContext = 0;

    if (m_pddh[m_iddh] != NULL)
    {
        DebMsg(DL_INFO, "DebugDocumentHelper DefineScriptBlock()...");

        hr = m_pddh[m_iddh]->DefineScriptBlock(0, wcslen(pwszScript),
            m_ps, FALSE, &dwSourceContext);
        if (SUCCEEDED(hr))
        {
            DebMsg(DL_INFO, "DebugDocumentHelper assigned SourceContext %lu", dwSourceContext);

            /* We never call DefineScriptBlock for a pddh more than once and we count
            on the fact that dwSourceContext will always be zero because of this
            (in GetDocumentContextFromPosition below).  The ASSERT below will,
            hopefully, clue me in if this ever happens differently. */

//          ASSERT(dwSourceContext == 0);
            dwDocContext[m_iddh] = dwSourceContext;

            m_iddh++;
        }
        else
        {
            DebMsg(DL_INFO, "DefineScriptBlock failed!");
            m_pddh[m_iddh]->Detach();
            ReleaseInterface(m_pddh[m_iddh]);
        }
    }

    DebMsg(DL_INFO, "ParseScriptText()...");

    hr = m_psp->ParseScriptText(
        pwszScript,             //code
        NULL,                   //item name
        NULL,                   //context
        NULL,                   //end delimiter
//      m_iddh,                 //Source Context (see above, this is bastardized)
        dwSourceContext,
        0,                      //Starting line number
        SCRIPTTEXT_ISVISIBLE |  //flags
        SCRIPTTEXT_HOSTMANAGESSOURCE |
        SCRIPTTEXT_ISPERSISTENT,
        NULL,                   //result
        &ei                     //returned exception info
        );

    delete [] pwszScript;

    DebMsg(DL_INFO, "ParseScriptText() returned %lX", hr);

    /* At the moment, no action is taken on an error because an error
       may just mean a runtime error occurred, but that the script is
       still loaded. */

    /* It would be best if all scripts were loaded before doing the CONNECT
       because then an error in ParseScriptText would unambiguously mean a
       compile error and that the text did not make it into the engines
       execution space. */

    // Start the script running...

    DebMsg(DL_INFO, "SetScriptState()...");

    hr = m_ps->SetScriptState(SCRIPTSTATE_CONNECTED);
    if (hr)
    {
        wsprintf(szAXStatus, "AXScript SetScriptState() Failed = %lX)", hr);
        return;
    }
}

void CScriptSite::ScriptStop(void)
{
    HRESULT hr;
    static EXCEPINFO ei;

    DebMsg(DL_INFO, "Entering ScriptStop()");


    ei.bstrDescription = SysAllocString(OLIT("Script aborted by user!"));
    ei.bstrSource = SysAllocString(OLIT("Kermit for Windows Script Aborted!"));
    ei.scode = DISP_E_EXCEPTION;

    hr = m_ps->InterruptScriptThread(SCRIPTTHREADID_BASE, &ei, 0);

    if (FAILED(hr))
        DebMsg(DL_INFO, "InterruptScriptThread() failed, hr=%lX", hr);

    DebMsg(DL_INFO, "Leaving ScriptStop()");
}

//***************************************************************************
// CScriptSite Create/Delete
//***************************************************************************


CScriptSite::CScriptSite(void)
{
    int i;

    m_cRef = 0;
    m_ps = NULL;
    m_psp = NULL;
    memset(&m_clsidEngine, '\0', sizeof(m_clsidEngine));

    m_iddh = 0;
    for (i = 0; i < MAX_SCRIPTS; i++)
    {
        m_pddh[i] = NULL;
        dwDocContext[i] = 0;
        memset(m_szScriptFile[i], '\0', sizeof(m_szScriptFile[0]));
    }

    return;
}


CScriptSite::~CScriptSite(void)
{
    return;
}

CScriptSite * CScriptSite::Create(void)
{
    CScriptSite * pck;

    DebMsg(DL_INFO, "Creating new CScriptSite...");

    pck = new CScriptSite();
    if (pck == NULL)
    {
        wsprintf(szAXStatus, "new CScriptSite() failed");
        return NULL;
    }

    pck->AddRef();

    return pck;
}

//***************************************************************************
// IUnknown Interface
//***************************************************************************

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP CScriptSite::QueryInterface
(
  REFIID  iid,
  LPVOID* ppvObjOut
)
{
    if (!ppvObjOut)
        return E_INVALIDARG;

    *ppvObjOut = NULL;

    if (iid == IID_IUnknown)
        *ppvObjOut = this;
    else if (iid == IID_IActiveScriptSite)
        *ppvObjOut = (IActiveScriptSite *)this;
    else if (iid == IID_IActiveScriptSiteWindow)
        *ppvObjOut = (IActiveScriptSiteWindow *)this;
    else if (iid == IID_IActiveScriptSiteDebug)
        *ppvObjOut = (IActiveScriptSiteDebug *)this;

    if (*ppvObjOut)
    {
        this->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CScriptSite::AddRef
(
  void
)
{
    m_cRef++;

    DebMsg(DL_INFO, "CScriptSite::AddRef, ref count now %i", m_cRef);

    return m_cRef;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CScriptSite::Release
(
 void
 )
{
    m_cRef--;

    DebMsg(DL_INFO, "CScriptSite::Release, ref count now %i", m_cRef);

    if (!m_cRef)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

//***************************************************************************
// IActiveScriptSite Interface
//***************************************************************************

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP CScriptSite::GetLCID
(
  LCID * X(plcid)
)
{
  return E_NOTIMPL;     // Use system settings
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP CScriptSite::GetItemInfo
(
 LPCOLESTR   pstrName,
 DWORD       dwReturnMask,
 IUnknown**  ppunkItemOut,
 ITypeInfo** pptinfoOut
 )
{
    void *  pItem;
    HRESULT hr;

    /* Check for valid args, as appropriate */
    if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
        if (pptinfoOut == NULL)
            return E_INVALIDARG;

    if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
        if (ppunkItemOut == NULL)
            return E_INVALIDARG;

    /* Pick the correct object implementation according to item name */
    if (!_wcsicmp(OLIT("Kermit"), pstrName))
        pItem = g_pKermOA;
    else
        return TYPE_E_ELEMENTNOTFOUND;

    /* Double check we have a reasonable object pointer */
    if (pItem == NULL)
        return E_NOTIMPL;

    /* Fill in ITypeInfo, if requested and available through ProvideClassInfo */
    if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
    {
        IProvideClassInfo * pIPCI;

        hr = ((IUnknown *)pItem)->QueryInterface(IID_IProvideClassInfo, (void **)&pIPCI);
        if (FAILED(hr))
            return hr;

        hr = pIPCI->GetClassInfo(pptinfoOut);

        pIPCI->Release();

        if (FAILED(hr))
            return hr;
    }

    /* Fill in IUnknown, if requested */
    if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
    {
        hr = ((IUnknown *)pItem)->QueryInterface(IID_IUnknown, (void **)ppunkItemOut);

        if (FAILED(hr))
        {
            if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
                (*pptinfoOut)->Release();
            return hr;
        }
    }

    return S_OK;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP CScriptSite::GetDocVersionString
(
  BSTR * X(pbstrVersion)
)
{
  return E_NOTIMPL;   // UNDONE: Implement this method
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP CScriptSite::OnScriptTerminate
(
  const VARIANT   * X(pvarResult),
  const EXCEPINFO * X(pexcepinfo)
)
{
    DebMsg(DL_INFO, "Script Terminated");

  // UNDONE: Put up error dlg here
  return S_OK;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP CScriptSite::OnStateChange
(
  SCRIPTSTATE ssScriptState
)
{
    char * pszState;

    switch (ssScriptState)
    {
    case SCRIPTSTATE_UNINITIALIZED:
        pszState = "Uninitialized";
        break;

    case SCRIPTSTATE_INITIALIZED:
        pszState = "Initialized";
        break;

    case SCRIPTSTATE_STARTED:
        pszState = "Started";
        break;

    case SCRIPTSTATE_CONNECTED:
        pszState = "Connected";
        break;

    case SCRIPTSTATE_DISCONNECTED:
        pszState = "Disconnected";
        break;

    case SCRIPTSTATE_CLOSED:
        pszState = "Closed";
        break;

    default:
        pszState = "<Unknown>";
        break;
    }

    DebMsg(DL_INFO, "Script State Change: %s", (LPSTR)pszState);

  // Don't care about notification
  return S_OK;
}


//---------------------------------------------------------------------------
// Display the error
//---------------------------------------------------------------------------
STDMETHODIMP CScriptSite::OnScriptError
(
  IActiveScriptError *pse
)
{
    char        szErrDesc[1024];
    char        *pszErrDescIdx;
    EXCEPINFO   ei;
    DWORD       dwSrcContext;
    ULONG       ulLine;
    LONG        ichError;
    BSTR        bstrLine = NULL;
    HRESULT     hr;
    char        szSrc[256];
    char        szDesc[256];
    char        szLine[256];

//  if (bAbort)
//      return S_OK;

    hr = pse->GetExceptionInfo(&ei);
    if (FAILED(hr))
    {
        KermitFmtMsgBox(MB_OK | MB_ICONEXCLAMATION, "GetExceptionInfo() Error!");
        return S_OK;
    }

    hr = pse->GetSourcePosition(&dwSrcContext, &ulLine, &ichError);
    if (FAILED(hr))
    {
        KermitFmtMsgBox(MB_OK | MB_ICONEXCLAMATION, "GetSourcePosition() Error!");
        return S_OK;
    }

    OLETOANSI(ei.bstrSource, szSrc, sizeof(szSrc));
    OLETOANSI(ei.bstrDescription, szDesc, sizeof(szDesc));

    pszErrDescIdx = szErrDesc;

    pszErrDescIdx += wsprintf(pszErrDescIdx,  "%s\n", (LPSTR)szSrc);

    pszErrDescIdx += wsprintf(pszErrDescIdx,  "Error %lX: ", (DWORD)ei.scode);

    pszErrDescIdx += SysErrText(ei.scode, pszErrDescIdx, szErrDesc + 1024 - pszErrDescIdx);

    pszErrDescIdx += wsprintf(pszErrDescIdx,  "\n%s", (LPSTR)szDesc);

    pszErrDescIdx += wsprintf(pszErrDescIdx,  "\n\nin %s file %s at line %ld",
                              (LPSTR)g_szScriptEngine, (LPSTR)m_szScriptFile, (long)ulLine);

    if (ichError > 0 && ichError < 255)
        pszErrDescIdx += wsprintf(pszErrDescIdx,  ", character %ld", (long)ichError);

    hr = pse->GetSourceLineText(&bstrLine);

    /* An error in the above just means there is no valid source
       line associated with the error being reported.  This usually
       happens when the user manually aborts the script.  Ignore errors
       in hr and report source line only if bstrLine is non-null. */

    if (bstrLine != NULL)
    {
        OLETOANSI(bstrLine, szLine, sizeof(szLine));

        pszErrDescIdx += wsprintf(pszErrDescIdx,  "\n\n%s", (LPSTR)szLine);

        if (ichError > 0 && ichError < 255)
        {
            char * pszArrow = new char[ichError + 1];
            memset(pszArrow, '-', ichError);
            pszArrow[ichError - 1] = '^';
            pszArrow[ichError] = '\0';
            pszErrDescIdx += wsprintf(pszErrDescIdx,  "\n%s", (LPSTR)pszArrow);
            delete [] pszArrow;
        }

        SysFreeString(bstrLine);
    }

    KermitFmtMsgBox(MB_OK | MB_ICONEXCLAMATION, szErrDesc);

    return S_OK;
    return S_FALSE;
//  return E_FAIL;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP CScriptSite::OnEnterScript
(
  void
)
{
    char szMsg[256];

    g_dwScriptDepth++;

    DebMsg(DL_INFO, "Entering Script - depth now %lu", g_dwScriptDepth);

    wsprintf(szMsg, "Executing script [%lu]...", g_dwScriptDepth);
    SetMessage(ML_SCRIPT, szMsg);

    if (g_dwScriptDepth == 1)
    {
        bScript = TRUE;
        bScriptAbort = FALSE;

        ModifyMenu(GetMenu(hAppWnd), IDM_SCRIPT, MF_BYCOMMAND | MF_STRING,
            IDM_SCRIPT, "A&bort Script");
    }

    return S_OK;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP CScriptSite::OnLeaveScript
(
  void
)
{
    char szMsg[256];

    g_dwScriptDepth--;

    DebMsg(DL_INFO, "Leaving Script - depth now %lu", g_dwScriptDepth);

    wsprintf(szMsg, "Executing script [%lu]...", g_dwScriptDepth);
    SetMessage(ML_SCRIPT, szMsg);

    if (g_dwScriptDepth == 0)
    {
        ModifyMenu(GetMenu(hAppWnd), IDM_SCRIPT, MF_BYCOMMAND | MF_STRING,
            IDM_SCRIPT, "&Manage Scripts...");

        SetMessage(ML_SCRIPT, NULL);

        bScript = FALSE;
    }

    return S_OK;
}

//***************************************************************************
// IActiveScriptSiteWindow Interface
//***************************************************************************

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP CScriptSite::GetWindow
(
  HWND *phwndOut
)
{
    if (!phwndOut)
        return E_INVALIDARG;

    *phwndOut = hAppWnd;

    return S_OK;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP CScriptSite::EnableModeless
(
  BOOL X(fEnable)
)
{
//  if (fEnable)
//    m_pcm->FOnComponentExitState(g_papp->m_idcomp, cmgrstateModal, cmgrcontextAll, 0, NULL);
//  else
//    m_pcm->OnComponentEnterState(g_papp->m_idcomp, cmgrstateModal, cmgrcontextAll, 0, NULL, NULL);

    return S_OK;
}


//***************************************************************************
// IActiveScriptSiteDebug Interface
//***************************************************************************

#if 0
STDMETHODIMP CScriptSite::GetDocumentContextFromPosition
(
 DWORD dwSourceContext,
 ULONG uCharacterOffset,
 ULONG uNumChars,
 IDebugDocumentContext __RPC_FAR *__RPC_FAR * ppddc
 )
{
    DWORD ulStartPos;
    HRESULT hr;

    DebMsg(DL_INFO, "GetDocumentContextFromPosition => dwSourceContext is %lu", dwSourceContext);

    (* ppddc) = pdd;

    return S_OK;
}
#endif

#if 1
STDMETHODIMP CScriptSite::GetDocumentContextFromPosition
(
 DWORD dwSourceContext,
 ULONG uCharacterOffset,
 ULONG uNumChars,
 IDebugDocumentContext __RPC_FAR *__RPC_FAR * ppddc
 )
{
    DWORD ulStartPos;
    HRESULT hr;
    DWORD i;

    DebMsg(DL_INFO, "GetDocumentContextFromPosition => dwSourceContext is %lu", dwSourceContext);

    for (i = 0; i < m_iddh; i++)
        if (dwDocContext[i] == dwSourceContext)
            break;

    if (i >= m_iddh)
    {
        DebMsg(DL_INFO, "Unable to find entry to match SourceContext!");
        return E_INVALIDARG;
    }

    if (m_pddh[i] == NULL)
    {
        DebMsg(DL_INFO, "Invalid document helper pointer for SourceContext!");
        return E_INVALIDARG;
    }

//  DebMsg(DL_INFO, "GetDocumentContextFromPosition => real dwSourceContext is %lu", dwDocContext[dwSourceContext]);

    hr = m_pddh[i]->GetScriptBlockInfo(dwSourceContext, NULL, &ulStartPos, NULL);
//  hr = m_pddh[dwSourceContext]->GetScriptBlockInfo(dwDocContext[dwSourceContext], NULL, &ulStartPos, NULL);
//  hr = m_pddh[dwSourceContext]->GetScriptBlockInfo(0, NULL, &ulStartPos, NULL);
//  hr = m_pddh[0]->GetScriptBlockInfo(dwSourceContext, NULL, &ulStartPos, NULL);

    if (FAILED(hr))
    {
        DebMsg(DL_INFO, "CScriptSite::GetScriptBlockInfo() failed!");
        return hr;
    }

    hr = m_pddh[i]->CreateDebugDocumentContext(ulStartPos + uCharacterOffset, uNumChars, ppddc);
//  hr = m_pddh[dwSourceContext]->CreateDebugDocumentContext(ulStartPos + uCharacterOffset, uNumChars, ppddc);
//  hr = m_pddh[dwSourceContext]->CreateDebugDocumentContext(uCharacterOffset, uNumChars, ppddc);
//  hr = m_pddh[0]->CreateDebugDocumentContext(ulStartPos + uCharacterOffset, uNumChars, ppddc);
    if (FAILED(hr))
        DebMsg(DL_INFO, "CScriptSite::CreateDebugDocumentContext() failed!");

    return hr;
}
#endif

STDMETHODIMP CScriptSite::GetApplication
(
 IDebugApplication __RPC_FAR *__RPC_FAR * ppda
 )
{
    DebMsg(DL_INFO, "CScriptSite::GetApplication()...");

    if (g_pda == NULL)
    {
        DebMsg(DL_INFO, "No valid debug application available!");
        return E_NOTIMPL;
    }

    (*ppda) = g_pda;
    (*ppda)->AddRef();

    return S_OK;
}

STDMETHODIMP CScriptSite::GetRootApplicationNode
(
 IDebugApplicationNode __RPC_FAR *__RPC_FAR * ppdanRoot
 )
{
    HRESULT hr;

    DebMsg(DL_INFO, "CScriptSite::GetRootApplicationNode()...");

    if (g_pda == NULL)
    {
        DebMsg(DL_INFO, "No valid debug application available!");
        return E_NOTIMPL;
    }

    hr = g_pda->GetRootNode(ppdanRoot);
    if (FAILED(hr))
        DebMsg(DL_INFO, "CScriptSite::GetRootApplicationNode() failed!");

    return hr;
}

STDMETHODIMP CScriptSite::OnScriptErrorDebug
(
 IActiveScriptErrorDebug __RPC_FAR * X(pErrorDebug),
 BOOL __RPC_FAR * pfEnterDebugger,
 BOOL __RPC_FAR * pfCallOnScriptErrorWhenContinuing
 )
{
    int nResult;

    DebMsg(DL_INFO, "CScriptSite::OnScriptErrorDebug()...");

    nResult = KermitFmtMsgBox(MB_YESNO | MB_ICONEXCLAMATION, "Script error, enter debugger?");

    *pfEnterDebugger = (nResult == IDYES);

    *pfCallOnScriptErrorWhenContinuing = TRUE;

    return S_OK;
}


#if 0

//***************************************************************************
// CDebugDocument Create/Delete
//***************************************************************************

CDebugDocument::CDebugDocument(void)
{
    return;
}


CDebugDocument::~CDebugDocument(void)
{
    return;
}

CDebugDocument * Create(void)
{
    return NULL;
}

//***************************************************************************
// CDebugDocument IUnknown Interface
//***************************************************************************

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP CDebugDocument::QueryInterface
(
  REFIID  iid,
  LPVOID* ppvObjOut
)
{
    if (!ppvObjOut)
        return E_INVALIDARG;

    *ppvObjOut = NULL;

    if (iid == IID_IUnknown)
        *ppvObjOut = this;
    else if (iid == IID_IDebugDocumentInfo)
    {
        DebMsg(DL_INFO, "Interface IDebugDocumentInfo requested");
        *ppvObjOut = (IDebugDocumentProvider *)this;
    }
    else if (iid == IID_IDebugDocumentProvider)
    {
        DebMsg(DL_INFO, "Interface IDebugDocumentProvider requested");
        *ppvObjOut = (IDebugDocumentProvider *)this;
    }
    else if (iid == IID_IDebugDocument)
    {
        DebMsg(DL_INFO, "Interface IDebugDocument requested");
        *ppvObjOut = (IDebugDocument *)this;
    }
    else if (iid == IID_IDebugDocumentText)
    {
        DebMsg(DL_INFO, "Interface IDebugDocumentText requested");
        *ppvObjOut = (IDebugDocumentText *)this;
    }
    else if (iid == IID_IDebugDocumentTextEvents)
    {
        DebMsg(DL_INFO, "Interface IDebugDocumentTextEvents requested");
        *ppvObjOut = (IDebugDocumentTextEvents *)this;
    }
    else if (iid == IID_IDebugDocumentTextAuthor)
    {
        DebMsg(DL_INFO, "Interface IDebugDocumentTextAuthor requested");
//      *ppvObjOut = (IDebugDocumentTextAuthor *)this;
    }
    else if (iid == IID_IDebugDocumentTextExternalAuthor)
    {
        DebMsg(DL_INFO, "Interface IDebugDocumentTextExternalAuthor requested");
//      *ppvObjOut = (IDebugDocumentTextExternalAuthor *)this;
    }
    else if (iid == IID_IDebugDocumentContext)
    {
        DebMsg(DL_INFO, "Interface IDebugDocumentContext requested");
        *ppvObjOut = (IDebugDocumentContext *)this;
    }
    else if (iid == IID_IDebugDocumentHelper)
    {
        DebMsg(DL_INFO, "Interface IDebugDocumentHelper requested");
    }
    else if (iid == IID_IDebugDocumentHost)
    {
        DebMsg(DL_INFO, "Interface IDebugDocumentHost requested");
    }
    else
    {
        LPOLESTR pstr;
        StringFromCLSID(iid, &pstr);
        DebMsg(DL_INFO, "Unknown DebugDocument interface requested 0x%ls!", pstr);
        CoTaskMemFree(pstr);
    }

    if (*ppvObjOut)
    {
        this->AddRef();
        return S_OK;
    }

    DebMsg(DL_INFO, "No interface pointer returned!");
    return E_NOINTERFACE;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CDebugDocument::AddRef
(
  void
)
{
  return ++m_cref;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CDebugDocument::Release
(
  void
)
{
  m_cref--;

  if (!m_cref)
    {
      delete this;
      return 0;
    }

  return m_cref;
}

//***************************************************************************
// CDebugDocument IDebugDocumentInfo Interface
//***************************************************************************

STDMETHODIMP CDebugDocument::GetName(DOCUMENTNAMETYPE dnt, BSTR __RPC_FAR *pbstrName)
{
    DebMsg(DL_INFO, "CDebugDocument::GetName()...");

    switch (dnt)
    {
    case DOCUMENTNAMETYPE_APPNODE:
        *pbstrName = SysAllocString(L"Script File App Node");
        break;

    case DOCUMENTNAMETYPE_TITLE:
        *pbstrName = SysAllocString(L"Script File Title");
        break;

    case DOCUMENTNAMETYPE_FILE_TAIL:
        *pbstrName = SysAllocString(L"x.ksc");
        break;

    case DOCUMENTNAMETYPE_URL:
        *pbstrName = SysAllocString(L"http://something");
        break;

    default:
        return E_NOTIMPL;
    }

    return S_OK;
}

STDMETHODIMP CDebugDocument::GetDocumentClassId(CLSID __RPC_FAR *pclsidDocument)
{
    DebMsg(DL_INFO, "CDebugDocument::GetDocumentClassId()...");

    return E_NOTIMPL;
}


//***************************************************************************
// CDebugDocument IDebugDocumentProvider Interface
//***************************************************************************

STDMETHODIMP CDebugDocument::GetDocument(IDebugDocument __RPC_FAR *__RPC_FAR *ppssd)
{
    DebMsg(DL_INFO, "CDebugDocument::GetDocument()...");

    *ppssd = (IDebugDocumentText *)this;

    return S_OK;
}


//***************************************************************************
// CDebugDocument IDebugDocument Interface
//***************************************************************************

//***************************************************************************
// CDebugDocument IDebugDocumentText Interface
//***************************************************************************

STDMETHODIMP CDebugDocument::GetDocumentAttributes(TEXT_DOC_ATTR __RPC_FAR *ptextdocattr)
{
    DebMsg(DL_INFO, "CDebugDocument::GetDocumentAttributes()...");

    return E_NOTIMPL;
}

STDMETHODIMP CDebugDocument::GetSize(ULONG __RPC_FAR *pcNumLines, ULONG __RPC_FAR *pcNumChars)
{
    DebMsg(DL_INFO, "CDebugDocument::GetSize()...");

    return E_NOTIMPL;
}

STDMETHODIMP CDebugDocument::GetPositionOfLine(ULONG cLineNumber, ULONG __RPC_FAR *pcCharacterPosition)
{
    DebMsg(DL_INFO, "CDebugDocument::GetPositionOfLine()...");

    return E_NOTIMPL;
}

STDMETHODIMP CDebugDocument::GetLineOfPosition(ULONG cCharacterPosition, ULONG __RPC_FAR *pcLineNumber, ULONG __RPC_FAR *pcCharacterOffsetInLine)
{
    DebMsg(DL_INFO, "CDebugDocument::GetLineOfPosition()...");

    return E_NOTIMPL;
}

STDMETHODIMP CDebugDocument::GetText(ULONG cCharacterPosition, WCHAR __RPC_FAR *pcharText, SOURCE_TEXT_ATTR __RPC_FAR *pstaTextAttr, ULONG __RPC_FAR *pcNumChars, ULONG cMaxChars)
{
    DebMsg(DL_INFO, "CDebugDocument::GetText()...");

    return E_NOTIMPL;
}

STDMETHODIMP CDebugDocument::GetPositionOfContext(IDebugDocumentContext __RPC_FAR *psc, ULONG __RPC_FAR *pcCharacterPosition, ULONG __RPC_FAR *cNumChars)
{
    DebMsg(DL_INFO, "CDebugDocument::GetPositionOfContext()...");

    return E_NOTIMPL;
}

STDMETHODIMP CDebugDocument::GetContextOfPosition(ULONG cCharacterPosition, ULONG cNumChars, IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc)
{
    DebMsg(DL_INFO, "CDebugDocument::GetContextOfPosition()...");

    return E_NOTIMPL;
}


//***************************************************************************
// CDebugDocument IDebugDocumentTextEvents Interface
//***************************************************************************

STDMETHODIMP CDebugDocument::onDestroy(void)
{
    DebMsg(DL_INFO, "CDebugDocument::onDestroy()...");

    return E_NOTIMPL;
}

STDMETHODIMP CDebugDocument::onInsertText(ULONG cCharacterPosition, ULONG cNumToInsert)
{
    DebMsg(DL_INFO, "CDebugDocument::onInsertText()...");

    return E_NOTIMPL;
}

STDMETHODIMP CDebugDocument::onRemoveText(ULONG cCharacterPosition, ULONG cNumToRemove)
{
    DebMsg(DL_INFO, "CDebugDocument::onRemoveText()...");

    return E_NOTIMPL;
}

STDMETHODIMP CDebugDocument::onReplaceText(ULONG cCharacterPosition, ULONG cNumToReplace)
{
    DebMsg(DL_INFO, "CDebugDocument::onReplaceText()...");

    return E_NOTIMPL;
}

STDMETHODIMP CDebugDocument::onUpdateTextAttributes(ULONG cCharacterPosition, ULONG cNumToUpdate)
{
    DebMsg(DL_INFO, "CDebugDocument::onUpdateTextAttributes()...");

    return E_NOTIMPL;
}

STDMETHODIMP CDebugDocument::onUpdateDocumentAttributes(TEXT_DOC_ATTR textdocattr)
{
    DebMsg(DL_INFO, "CDebugDocument::onUpdateDocumentAttributes()...");

    return E_NOTIMPL;
}


//***************************************************************************
// CDebugDocument IDebugDocumentTextAuthor Interface
//***************************************************************************

STDMETHODIMP CDebugDocument::InsertText(ULONG cCharacterPosition, ULONG cNumToInsert, OLECHAR __RPC_FAR pcharText[])
{
    DebMsg(DL_INFO, "CDebugDocument::InsertText()...");

    return E_NOTIMPL;
}

STDMETHODIMP CDebugDocument::RemoveText(ULONG cCharacterPosition, ULONG cNumToRemove)
{
    DebMsg(DL_INFO, "CDebugDocument::RemoveText()...");

    return E_NOTIMPL;
}

STDMETHODIMP CDebugDocument::ReplaceText(ULONG cCharacterPosition, ULONG cNumToReplace, OLECHAR __RPC_FAR pcharText[])
{
    DebMsg(DL_INFO, "CDebugDocument::ReplaceText()...");

    return E_NOTIMPL;
}


//***************************************************************************
// CDebugDocument IDebugDocumentContext Interface
//***************************************************************************

//STDMETHODIMP CDebugDocument::GetDocument(IDebugDocument __RPC_FAR *__RPC_FAR *ppssd)
//{
//  DebMsg(DL_INFO, "CDebugDocument::GetDocument()...");
//
//  *ppssd = (IDebugDocument *)this;
//
//  return S_OK;
//}

STDMETHODIMP CDebugDocument::EnumCodeContexts(IEnumDebugCodeContexts __RPC_FAR *__RPC_FAR *ppescc)
{
    DebMsg(DL_INFO, "CDebugDocument::EnumCodeContexts()...");

    return E_NOTIMPL;
}

#endif