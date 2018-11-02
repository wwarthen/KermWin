/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMINIT.C                                   **
**                                                                            **
**  This module contains initialization functions required at application     **
**  startup time.                                                             **
**                                                                            **
*******************************************************************************/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include "ole2ver.h"
#include "kermc3d.h"
#include "DKermOA.h"

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC MakeClass(HINSTANCE hInstance)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    WNDCLASS WndClass;

    DebMsg(DL_INFO, "Initializing component managers...");

    memset(&WndClass, 0, sizeof(WndClass));

    if (!(TextInit(hInstance)))
        return(FALSE);

    if (!(MessageInit(hInstance)))
        return(FALSE);

    if (!(TerminalInit(hInstance)))
    return(FALSE);

    if (!(StatusInit(hInstance)))
    return(FALSE);

    if (!(CommInit(hInstance)))
    return(FALSE);

    if (!(InitKermitClass(hInstance)))
    return(FALSE);

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC KermitInit(HINSTANCE hInstance, HINSTANCE hPrevInstance,
            LPSTR lpszCmdLine, int cmdShow)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    char szWorkStr [80] = "";
    char szTargetFile [80] = "";
    int nWorkStr = 0;
    int i = 0;
    BOOL bConnect = FALSE;
    BOOL bAutomation = FALSE;
    bAbort = FALSE;
    bScript = FALSE;
    bScriptAbort = FALSE;

    DebMsg(DL_INFO, "Initializing application...");

    dwBaseThreadId = GetCurrentThreadId();
    DebMsg(DL_INFO, "Application base thread id = %lu", dwBaseThreadId);

#ifdef _WIN32
    {
    #pragma warning(push)
    #pragma warning(disable: 4996)

        OSVERSIONINFO osvi;

        osvi.dwOSVersionInfoSize = sizeof(osvi);
        if (GetVersionEx(&osvi))
        {
            OSVer.dwMajor = osvi.dwMajorVersion;
            OSVer.dwMinor = osvi.dwMinorVersion;
            OSVer.dwBuild = LOWORD(osvi.dwBuildNumber);
        }
    #pragma warning(pop)
    }
#else
    OSVer.dwMajor = LOBYTE(LOWORD(GetVersion()));
    OSVer.dwMinor = HIBYTE(LOWORD(GetVersion()));
    OSVer.dwBuild = HIWORD(GetVersion()) & 0x7FFF;
#endif

    DebMsg(DL_INFO, "OS Version: %lu.%lu.%lu", OSVer.dwMajor, OSVer.dwMinor, OSVer.dwBuild);

    if (!hPrevInstance)
    {
        if (!MakeClass(hInstance))
            return FALSE;
    }

    hAppInst = hInstance;

    DebMsg(DL_INFO, "Creating application window...");

    hAppWnd = CreateWindow(szAppName, szAppName,
               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
               CW_USEDEFAULT, CW_USEDEFAULT,
               CW_USEDEFAULT, CW_USEDEFAULT,
               NULL, NULL, hInstance, NULL);

    ASSERT(hAppWnd != NULL)

    DebugAddMenu();

    hMenuAccel = LoadAccelerators(hInstance, "MenuAccel");
    ASSERT(hMenuAccel != NULL)

    hXferAccel = LoadAccelerators(hInstance, "XferAccel");
    ASSERT(hXferAccel != NULL)

    GetModuleFileName(hAppInst, szHelpFileName, sizeof(szHelpFileName));
    *(strrchr(szHelpFileName, '\\') + 1) = '\0';
    lstrcat(szHelpFileName, "Kermit.hlp");

    DebMsg(DL_INFO, "Initializing 3D control...");

    C3DInit();

    C3DRegister(hInstance);
    C3DAutoSubclass(hInstance);

    DebMsg(DL_INFO, "Performing component initialization...");

    CMInit();
    TMInit();
    EMInit();
    PMInit();
    DMInit();
    SMInit();

    OLEVer.dwMajor = HIWORD(CoBuildVersion());
    OLEVer.dwMinor = LOWORD(CoBuildVersion());
    OLEVer.dwMajorReq = rmm;
    OLEVer.dwMinorReq = rup;

    DebMsg(DL_INFO, "OLE Version: %lu.%lu", OLEVer.dwMajor, OLEVer.dwMinor);
    DebMsg(DL_INFO, "OLE Version required: %lu.%lu", OLEVer.dwMajorReq, OLEVer.dwMinorReq);

    // Initialize OLE
    if (OLEVer.dwMajor >= OLEVer.dwMajorReq)
    {
        bOleInit = SUCCEEDED(CoInitialize(NULL));
        if (!bOleInit)
            KermitFmtMsgBox(MB_OK, "CoInitialize() Failed!");
    }
    else
        bOleInit = FALSE;

    // Create and initialize automation object

    DebMsg(DL_INFO, "Initializing OLE automation...");

    if (bOleInit) {
        if (LoadKermTypeLib())
        {
            if (CKermOA::Create() == NULL)
                KermitFmtMsgBox(MB_OK, "CKermOA::Create() Failed!");
        }
        else
            KermitFmtMsgBox(MB_OK, "LoadKermTypeLib() Failed!");

    }

    if (!SetTimer(hAppWnd, 1, 250, NULL)) {
        KermitFmtMsgBox(MB_OK | MB_ICONASTERISK,
                        "No Windows timer resources available!\n\n"
                        "This application requires at least one timer.");
        DestroyWindow(hAppWnd);
        return(FALSE);
    }

#ifdef AXSCRIPT

    DebMsg(DL_INFO, "Initializing ActiveX scripting...");

    if (bOleInit)
        AXInit();

#endif

    for (i = 0; ; i++)
    {
        if (lpszCmdLine[i] == ' ' || lpszCmdLine[i] == '\t' || lpszCmdLine[i] == '\0')
        {
            szWorkStr[nWorkStr] = '\0';

            if (szWorkStr[0] == '/' || szWorkStr[0] == '-')
            {
                if (lstrcmpi(szWorkStr + 1, "Automation") == 0)
                    bAutomation = TRUE;
                else if (lstrcmpi(szWorkStr + 1, "Connect") == 0)
                    bConnect = TRUE;
            }
            else
                lstrcpy(szTargetFile, szWorkStr);

            nWorkStr = 0;
        }
        else
            szWorkStr[nWorkStr++] = lpszCmdLine[i];

        if (lpszCmdLine[i] == '\0')
            break;
    }

    if (bAutomation)
    {
        DebMsg(DL_INFO, "Creating automation class factory...");

        if (g_pKermOA == NULL)
            KermitFmtMsgBox(MB_OK, "Can't start automation -- CKermOA creation failed!");
        else
        {
            g_pKermOAF = CKermOAFactory::Create(g_pKermOA);
            if (g_pKermOAF == NULL)
            {
                KermitFmtMsgBox(MB_OK, "CKermOAF::Create() Failed!");
//              g_pKermOA->Release();
//              g_pKermOA = NULL;

                /* Should do something to close app at this point because
                   there is nothing it can do without a class factory. */
            }
        }

        bAppVisible = FALSE;
    }
    else
    {
        DebMsg(DL_INFO, "Showing and updating application window...");

        ShowWindow(hAppWnd, cmdShow);
        UpdateWindow(hAppWnd);

        bAppVisible = TRUE;
        ((IUnknown *)g_pKermOA)->AddRef();  // Keep automation object alive while visible
    }

#if defined(AXSCRIPT)

    if (szTargetFile[0] != '\0')
    {
        if (lstrcmpi(_fstrrchr(szTargetFile, '.'), ".krm") == 0)
        {
            DebMsg(DL_INFO, "Opening session file %s...", (LPSTR)szTargetFile);
            OpenSess(szTargetFile);
        }
        else
        {
            DebMsg(DL_INFO, "Running script file %s...", (LPSTR)szTargetFile);
            AXScriptLoadFile(szTargetFile);
        }
    }

#else

    if (szTargetFile[0] != '\0')
    {
        DebMsg(DL_INFO, "Opening session file %s...", (LPSTR)szTargetFile);
        OpenSess(szTargetFile);
    }

#endif

    if (bConnect)
    {
        DebMsg(DL_INFO, "Autoconnect...");
        DMConnect();
    }

    return TRUE;
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC GetSysValues
/*                                        */
/* Get various system metrics for later use.                      */
/*                                        */
/*----------------------------------------------------------------------------*/
(HWND hWnd) /* Window to use for retrieving the text metrics.         */
/*----------------------------------------------------------------------------*/

{
    HDC        hDC;
    TEXTMETRIC tm;

    hDC = GetDC(hWnd);
    GetTextMetrics(hDC, &tm);

#ifdef _WIN32
    xSysChar = (int)tm.tmAveCharWidth;
    ySysChar = (int)tm.tmHeight;
#else
    xSysChar = tm.tmAveCharWidth;
    ySysChar = tm.tmHeight;
#endif

    ReleaseDC(hWnd, hDC);

    xBord = GetSystemMetrics(SM_CXBORDER);
    yBord = GetSystemMetrics(SM_CYBORDER);
    yMenu = GetSystemMetrics(SM_CYMENU);
}