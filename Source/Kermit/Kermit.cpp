/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                                KERMIT.C                                    **
**                                                                            **
**                      Version 0.50 - Developmental                          **
**                     Requires Microsoft Windows 2.X                         **
**                         Author: Wayne Warthen                              **
**                                                                            **
**  Primary application source file containing the WinMain() and the client   **
**  Window message handling procedure and support functions.                  **
**                                                                            **
*******************************************************************************/

/* TODO ------------------------------------------------------------------------

------------------------------------------------------------------------------*/

/* DEFINITIONS ---------------------------------------------------------------*/

#define GLOBAL

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include "kermtloc.h"
#include "kermprot.h"
#include "kermc3d.h"
#include "KermOLE.h"

/* DEFINITIONS ---------------------------------------------------------------*/

/* LOCAL DATA ----------------------------------------------------------------*/

static BOOL bQuit = FALSE;
static BOOL bDispInvokePending = FALSE;

/*==================================================================*/
int PASCAL WinMain
/*==================================================================*/
/*                                                                  */
/*  Top level application function called by MS Windows             */
/*  when Kermit is executed.  Returns DOS return code.              */
/*                                                                  */
(HINSTANCE hInstance,      /* Handle to this instance.              */
 HINSTANCE hPrevInstance,  /* Handle to previous instance (if any). */
 LPSTR     lpszCmdLine,    /* Command line parms.                   */
 int       nCmdShow)       /* Window type requested.                */
/*==================================================================*/

{
#ifndef _WIN32
    int cMsg = 96;

    while ((!SetMessageQueue(cMsg)) && (cMsg > 0))
        cMsg -= 8;

#endif

    DebugInit(hInstance, hPrevInstance, lpszCmdLine, nCmdShow);

    bQuit = FALSE;
    bAbortConfirm = FALSE;
    nCloseReq = CAR_NONE;
    bDispInvokePending = TRUE;

    if (!KermitInit(hInstance, hPrevInstance, lpszCmdLine, nCmdShow))
        return(FALSE);


    while (!bQuit)
    {
        while (KermitPoll() && !bAbort)
        {
            MessagePump();
            if (bDispInvokePending)
            {
                DispInvokeHandler();
                bDispInvokePending = FALSE;
            }
        }

        if (bQuit)
            break;

            /* Actually, the following check for bAbort should be redundant
            since that is really the only way we can get out of the above
        loop (KermitPoll never returns FALSE). */

        if (bAbort)
        {
            BOOL bLocalAbortConfirm;
            int nLocalCloseReq;

            bAbort = FALSE;
            bLocalAbortConfirm = bAbortConfirm;
            bAbortConfirm = FALSE;
            nLocalCloseReq = nCloseReq;
            nCloseReq = CAR_NONE;

            if (nLocalCloseReq == CAR_DISCONNECT)
            {
                DMDisconnect(CD_FORCE);
                continue;
            }

            if (nLocalCloseReq == CAR_DISCONNECTED)
            {
                DMDisconnect(CD_ABORT);
                continue;
            }

            if (bLocalAbortConfirm && !ConfirmClose())
                continue;

            if (!CloseSess() && (nLocalCloseReq != CAR_QUIT))
                continue;

            switch (nLocalCloseReq) {
            case CAR_QUIT:
                DestroyWindow(hAppWnd);
                break;

            case CAR_RESET:
                NewSess();
                break;

            case CAR_OPEN:
                OpenSess(NULL);
                break;
            }
        }
    }

    if (dwRegisterActiveObject != 0)
    {
        HRESULT hr;

        hr = RevokeActiveObject(dwRegisterActiveObject, NULL);
        dwRegisterActiveObject = 0;

        DebMsg(DL_INFO, "RevokeActiveObject() returned %lu", hr);
    }

    if (dwRegisterCF != 0)
    {
        HRESULT hr;

        hr = CoRevokeClassObject(dwRegisterCF);
        dwRegisterCF = 0;

        DebMsg(DL_INFO, "CoRevokeClassObject() returned %lu", hr);
    }

    ASSERT(g_pKermOAF == NULL);

#ifdef AXSCRIPT
    DebMsg(DL_INFO, "ActiveX Scripting Termination...");
    AXTerm();
#endif

    if (g_pKermOA != NULL)
    {
        HRESULT hr;

        hr = CoDisconnectObject((IUnknown *)g_pKermOA, 0);

        DebMsg(DL_INFO, "CoDisconnectObject() returned %lu", hr);
    }

    // Allow automation object to release if originally locked due to window visible
    if (bAppVisible && (g_pKermOA != NULL))
    {
        DebMsg(DL_INFO, "Releasing KermOA to release window visible lock...");
        ((IUnknown *)g_pKermOA)->Release();
    }

    ASSERT(g_pKermOA == NULL);

    if (g_pKermTypeLib != NULL)
    {
        ReleaseInterface(g_pKermTypeLib);
        ASSERT(g_pKermTypeLib == NULL);
    }

    if (bOleInit)
    {
        DebMsg(DL_INFO, "CoUninitialize...");
        CoUninitialize();
        bOleInit = FALSE;
    }

    DebMsg(DL_INFO, "3D control termination...");
    C3DTerm();

    DebugTerm();

    return 0;
}


BOOL PUBFUNC MessagePump(VOID)
{
    MSG Msg;

    while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
    {
        if (Msg.message == WM_QUIT)
        {
            bAbort = TRUE;
            bQuit = TRUE;
            return TRUE;
        }

        if ((hWndStat != NULL) && IsDialogMessage(hWndStat, &Msg))
            continue;

        if (TranslateAccelerator(hAppWnd, hMenuAccel, &Msg))
            continue;

        if (bKermit && TranslateAccelerator(hAppWnd, hXferAccel, &Msg))
            continue;

        bSuppCharMsg = FALSE;

        TranslateMessage(&Msg);
        DispatchMessage(&Msg);

            /* if bSuppressChar has been set during the processing of
               a WM_KEYDOWN or WM_SYSKEYDOWN message, use PeekMessage()
               to discard the corresponding WM_CHAR or WM_SYSCHAR if
               such a message has been posted by Translate Message. */

        if (bSuppCharMsg)
        {
            if (Msg.message == WM_KEYDOWN)
                PeekMessage(&Msg, NULL, WM_CHAR, WM_CHAR, PM_REMOVE);
            else if (Msg.message == WM_SYSKEYDOWN)
                PeekMessage(&Msg, NULL, WM_SYSCHAR, WM_SYSCHAR, PM_REMOVE);
        }
    }

    return FALSE;
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC KermitPoll
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Poll appropriate poll(ing) functions based on status                       */
/*                                                                            */
(VOID)                 /* No parameters.                                      */
/*----------------------------------------------------------------------------*/

{
    if (bScript)
    {
        WaitMessage();
        return TRUE;
    }

    if (bConnected)
        ProcessTermLine();

    if (!bCommBusy && !GetQueueStatus(QS_ALLINPUT))
        WaitMessage();

    return TRUE;
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC Kermit_OnTimer
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle comm event notifications                                            */
/*                                                                            */
(HWND   X(hWnd),       /* Window that received the command.                   */
 UINT   X(id))         /* Timer identifier.                                   */
/*----------------------------------------------------------------------------*/

{
    StatusUpdate();
    MessageUpdate();
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC Kermit_OnSysCommand
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle menu commands.                                                      */
/*                                                                            */
(HWND   hWnd,          /* Window that received the command.                   */
 UINT   cmd,           /* Command                                             */
 int    x,             /* ???                                                 */
 int    y)             /* ???                                                 */
/*----------------------------------------------------------------------------*/

{
    if ((cmd & 0xFFF0) == SC_SIZE)
        bAppSizeActive = TRUE;

    FORWARD_WM_SYSCOMMAND(hWnd, cmd, x, y, DefWindowProc);

    if ((cmd & 0xFFF0) == SC_SIZE)
    {
        bAppSizeActive = FALSE;
        TermSizeEnd();
    }

    return;
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC Kermit_OnCommand
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle menu commands.                                                      */
/*                                                                            */
(HWND   hWnd,          /* Window that received the command.                   */
 int    Id,            /* Command (wParam of WM_COMMAND message).             */
 HWND   X(hWndCtl),    /* Parameter (lParam of WM_COMMAND message).           */
 UINT   X(wNotify))    /* Parameter (lParam of WM_COMMAND message).           */
/*----------------------------------------------------------------------------*/

{
    if (DebugMenuCmd(hWnd, Id))
        return;

    switch (Id)
    {
        case IDM_NEW:
//            NewSess();
            bAbort = TRUE;
            bAbortConfirm = TRUE;
            nCloseReq = CAR_RESET;
            break;

        case IDM_OPEN:
//            OpenSess(NULL);
            bAbort = TRUE;
            bAbortConfirm = TRUE;
            nCloseReq = CAR_OPEN;
            break;

        case IDM_SAVE:
        case IDM_SAVEAS:
            SaveSess(Id == IDM_SAVEAS);
            break;

#ifdef AXSCRIPT
        case IDM_SCRIPT:
            AXScriptDialog();
            break;

        case IDM_EXEC:
            AXExecDialog();
            break;

        case IDM_DBGROPEN:
            AXScriptDebugStart();
            break;

        case IDM_DBGRBREAK:
            AXScriptDebugBreak();
            break;
#endif

        case IDM_EXIT:
            SendMessage(hWnd, WM_CLOSE, 0, 0L);
            break;

        case IDM_COPY:
            TermCBCopy();
            break;

        case IDM_PASTE:
            TermCBPaste();
            break;

        case IDM_REVIEW:
            TermReview(!bReview);
            break;

        case IDM_SESSION:
            SMSetup(hAppInst, hWnd);
            break;

        case IDM_TERMINAL:
            EMSetup(hAppInst, hWnd);
            break;

        case IDM_COMMUNICATIONS:
            CMSetup(hAppInst, hWnd);
            break;

        case IDM_DEVICE:
            DMSetup(hAppInst, hWnd);
            break;

        case IDM_PROTOCOL:
            PMSetup(hAppInst, hWnd);
            break;

        case IDM_LOGGING:
            GoDialogBox(hAppInst, "LoggingDlgBox",
                        hWnd, (FARPROC)LoggingDlgProc);
            break;

        case IDM_FONT:
            TMSetup(hAppInst, hWnd);
            break;

        case IDM_CONNECT:
            if (bConnected) {
                bAbort = TRUE;
                bAbortConfirm = TRUE;
                nCloseReq = CAR_DISCONNECT;
            }
            else
                DMConnect();
            break;

        case IDM_PRINT:
            break;

        case IDM_BREAK:
            SendBreak();
            break;

        case IDM_TRANSMIT:
            break;

        case IDM_CAPTURE:
            break;

        case IDM_SEND:
            KermitSend();
            break;

        case IDM_RECEIVE:
            KermitReceive();
            break;

        case IDM_SERVER:
            KermitServer();
            break;

        case IDM_GET:
            KermitGet();
            break;

        case IDM_HOST:
            KermitHost();
            break;

        case IDM_GENERIC:
            KermitGeneric();
            break;

        case IDM_CANFILE:
        case IDM_CANBATCH:
        case IDM_RETRY:
        case IDM_STOP:
        case IDM_ABORT:
            KermitUserInt((UINT)Id);
            break;

        case IDM_HELPCTX:
            WinHelp(hWnd, szHelpFileName, HELP_CONTENTS, 0L);
            break;

        case IDM_HELPSRCH:
            WinHelp(hWnd, szHelpFileName, HELP_PARTIALKEY, (DWORD)(LPSTR)"");
            break;

        case IDM_HELPHELP:
            WinHelp(hWnd, "WINHELP.HLP", HELP_HELPONHELP, 0L);
            break;

        case IDM_ABOUT:
            GoDialogBox(hAppInst, "AboutDlgBox",
                        hWnd, (FARPROC)AboutDlgProc);
            break;
    }
}

BOOL PasteReady(VOID)
{
    BOOL bReady;

    bReady = FALSE;

    if (OpenClipboard(hAppWnd))
    {
        bReady = IsClipboardFormatAvailable(CF_OEMTEXT);
        CloseClipboard();
    }

    return bReady;
}

VOID EnableSubMenu(HMENU hSubMenu, BOOL bActivate)
{
    WORD wItem;

    for (wItem = 0; (int)wItem < GetMenuItemCount(hSubMenu); wItem++)
        EnableMenuItem(hSubMenu, wItem, (UINT)((bActivate) ? MF_ENABLED : MF_GRAYED) | MF_BYPOSITION);
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC Kermit_OnInitMenu
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Modify application menu just before it is displayed based on current       */
/* application states.                                                        */
/*                                                                            */
(HWND   hWnd,         /* Window containing menu                               */
 HMENU  hMenu)        /* Menu being accessed                                  */
/*----------------------------------------------------------------------------*/

{
#define SUBMENU_FILE        0
#define SUBMENU_EDIT        1
#define SUBMENU_SESSION     2
#define SUBMENU_KERMIT      3
#define SUBMENU_CONFIGURE   4
#define SUBMENU_HELP        5
#define SUBMENU_DEBUG       6

#define CMD_ACT(menu, act) EnableMenuItem(hMenu, menu, (UINT)((act) ? MF_ENABLED : MF_GRAYED))
#define SUBMENU_ACT(submenu, act) EnableSubMenu(GetSubMenu(hMenu, submenu), act)

    if (hMenu != GetMenu(hWnd))
        return;

    CMD_ACT(IDM_NEW, TRUE);
    CMD_ACT(IDM_OPEN, TRUE);

    CMD_ACT(IDM_SAVE, TRUE);
    CMD_ACT(IDM_SAVEAS, TRUE);
#ifdef AXSCRIPT
    CMD_ACT(IDM_SCRIPT, !(bKermit || bReview || bReadLine));
    CMD_ACT(IDM_EXEC, !(bKermit || bReview || bReadLine || bScript));
#endif
    CMD_ACT(IDM_EXIT, TRUE);

    CMD_ACT(IDM_COPY, bSelect && !(bKermit || bScript));
    CMD_ACT(IDM_PASTE, !(bReview || bKermit || bReadLine || bScript) && PasteReady());
    CMD_ACT(IDM_REVIEW, !(bKermit || bReadLine || bScript));

    CMD_ACT(IDM_CONNECT, !(bReadLine || bScript));
    CMD_ACT(IDM_BREAK, bConnected && (GetCommInfo(GCI_CONNTYPE) == CT_ASYNC) &&
                       !(bKermit || bReview || bReadLine || bScript));

    SUBMENU_ACT(SUBMENU_KERMIT, bConnected && !(bReview || bReadLine || bScript));

    SUBMENU_ACT(SUBMENU_CONFIGURE, !bKermit && !(bReview || bReadLine || bScript));
    CMD_ACT(IDM_DEVICE, !(bKermit || bReview || bReadLine || bScript) &&
                        (GetCommInfo(GCI_CONNTYPE) == CT_ASYNC));

    CMD_ACT(IDM_LOGGING, FALSE);
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC Kermit_OnMenuSelect
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Display a line of help text in the message window based on the currently   */
/* highlighted menu bar selection.                                            */
/*                                                                            */
(HWND      X(hWnd),      /*                                                   */
 HMENU     X(hMenu),     /*                                                   */
 int       nItem,        /*                                                   */
 HMENU     hMenuPopup,   /*                                                   */
 UINT      wFlags)       /*                                                   */
/*----------------------------------------------------------------------------*/

{
    char    szText[80];

    if (wFlags == -1)
    {
        SetMessage(ML_MENU, NULL);
        return;
    }

    if (hMenuPopup == NULL)
    {
        if (nItem == 0)
        {
            SetMessage(ML_MENU, " ");
            return;
        }

        if (LoadString(hAppInst, nItem, szText, sizeof(szText)))
        {
            SetMessage(ML_MENU, szText);
            return;
        }

#ifdef DEBUG
        SetMessage(ML_MENU, "Unknown Menu Command???");
#else
        SetMessage(ML_MENU, "");
#endif
        return;
    }
    else
    {
        int     nMenu, nMenuCnt;
        HMENU   hPopupMenu;

        nMenuCnt = GetMenuItemCount(GetMenu(hAppWnd));
        for (nMenu = 0; nMenu < nMenuCnt; nMenu++)
        {
            hPopupMenu = GetSubMenu(GetMenu(hAppWnd), nMenu);
            if (hPopupMenu == hMenuPopup) {
                LoadString(hAppInst, IDM_TOPMENU + nMenu + 1,
                           szText, sizeof(szText));
                SetMessage(ML_MENU, szText);
                return;
            }
        }

#ifdef DEBUG
        SetMessage(ML_MENU, "Unknown Menu???");
#else
        SetMessage(ML_MENU, "");
#endif
        return;
    }
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC Kermit_OnCreate
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle kermit windows creation stuff.                                      */
/*                                                                            */
(HWND               hWnd,              /* Window that received the command.   */
 LPCREATESTRUCT     X(lpCreateStruct)) /* Create structure.                   */
/*----------------------------------------------------------------------------*/

{
    hAppWnd = hWnd;
    GetSysValues(hWnd);

    CrtSubWnd(szClsKermStatus, NULL, hWnd, WID_STATUS, hAppInst);
    CrtSubWnd(szClsKermTerm, WS_HSCROLL | WS_VSCROLL,
              hWnd, WID_TERM, hAppInst);
    CrtSubWnd(szClsKermMsg, NULL, hWnd, WID_INFO, hAppInst);

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
void PRVFUNC Kermit_OnSize
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle kermit window size change.                                          */
/*                                                                            */
(HWND   hWnd,     /* Window that received the command.                        */
 UINT   statew,   /* Window state.                                            */
 int    cxw,      /* New window width.                                        */
 int    cyw)      /* New window height.                                       */
/*----------------------------------------------------------------------------*/

{
    HDWP hdwp;

    if (statew == SIZE_MINIMIZED)
        return;

    hdwp = BeginDeferWindowPos(5);

    hdwp = DefMovSubWnd(hdwp, hWnd, WID_STATUS,
                        0, 0, cxw, yMenu + 4);

    hdwp = DefMovSubWnd(hdwp, hWnd, WID_TERM,
                        0, yMenu + 4, cxw, cyw - ((yMenu + 4) * 2));

    hdwp = DefMovSubWnd(hdwp, hWnd, WID_INFO,
                        0, cyw - yMenu - 4, cxw, yMenu + 4);

    EndDeferWindowPos(hdwp);

    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void PRVFUNC Kermit_OnClose(HWND X(hWnd))
{
    bAbort = TRUE;
    bAbortConfirm = TRUE;
    nCloseReq = CAR_QUIT;
}

BOOL PRVFUNC Kermit_OnQueryEndSession(HWND /* hWnd */ )
{
    if (!ConfirmClose())
        return FALSE;

    bAbort = TRUE;
    bAbortConfirm = FALSE;
    nCloseReq = CAR_QUIT;
    return TRUE;
}

void PRVFUNC Kermit_OnDestroy(HWND hWnd)
{
    KillTimer(hAppWnd, 1);

    WinHelp(hWnd, szHelpFileName, HELP_QUIT, 0L);

    hAppWnd = NULL;

    C3DUnregister(hAppInst);

    PostQuitMessage(0);
}

void PRVFUNC Kermit_OnActivate(HWND hWnd, UINT statew, HWND X(hWndActDeact), BOOL fMinimized)
{
    if (statew != 0 && !fMinimized)
        SetFocus(GetDlgItem(hWnd, WID_TERM));
}

void PRVFUNC Kermit_OnSysColorChange(HWND X(hWnd))
{
    C3DColorChange();
}

BOOL PRVFUNC Kermit_OnSetCursor(HWND hwnd, HWND hwndCursor,
                                UINT codeHitTest, UINT msg)
{
//    if (nAppBusy > 0)
//  {
//        SetCursor(LoadCursor(NULL, IDC_WAIT));
//        return TRUE;
//    }
//    else
        return FORWARD_WM_SETCURSOR(hwnd, hwndCursor, codeHitTest, msg, DefWindowProc);
}

/*---------------------------------------------------------------------------*/
LONG CALLBACK __export KermitWndProc
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Window procedure for the base application window.                         */
/*                                                                           */
(HWND     hWnd,     /* Handle to window receiving message                    */
 UINT     message,  /* Message number                                        */
 WPARAM   wParam,   /* First Message Parameter (UINT)                        */
 LPARAM   lParam)   /* Second Message Parameter (LONG)                       */
/*---------------------------------------------------------------------------*/

{
    if (message == WM_USER_INVOKEPENDING)
    {
        bDispInvokePending = TRUE;
        return 0L;
    }

    switch (message)
    {

        HANDLE_MSG(hWnd, WM_TIMER, Kermit_OnTimer);

        HANDLE_MSG(hWnd, WM_COMMAND, Kermit_OnCommand);

        HANDLE_MSG(hWnd, WM_SYSCOMMAND, Kermit_OnSysCommand);

        HANDLE_MSG(hWnd, WM_INITMENU, Kermit_OnInitMenu);

        HANDLE_MSG(hWnd, WM_MENUSELECT, Kermit_OnMenuSelect);

        HANDLE_MSG(hWnd, WM_CREATE, Kermit_OnCreate);

        HANDLE_MSG(hWnd, WM_SIZE, Kermit_OnSize);

        HANDLE_MSG(hWnd, WM_CLOSE, Kermit_OnClose);

        HANDLE_MSG(hWnd, WM_QUERYENDSESSION, Kermit_OnQueryEndSession);

        HANDLE_MSG(hWnd, WM_DESTROY, Kermit_OnDestroy);

        HANDLE_MSG(hWnd, WM_ACTIVATE, Kermit_OnActivate);

        HANDLE_MSG(hWnd, WM_SYSCOLORCHANGE, Kermit_OnSysColorChange);

        HANDLE_MSG(hWnd, WM_SETCURSOR, Kermit_OnSetCursor);

        default:
            return(DefWindowProc(hWnd, message, wParam, lParam));
    }

    return(0L);
}


/*----------------------------------------------------------------------------*/
BOOL PUBFUNC InitKermitClass(HINSTANCE hInstance)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    WNDCLASS WndClass;

    memset(&WndClass, 0, sizeof(WndClass));

    WndClass.lpszClassName        = szAppName;
    WndClass.lpszMenuName         = "KermitMenu";
    WndClass.hInstance            = hInstance;
    WndClass.lpfnWndProc          = KermitWndProc;
    WndClass.style                = NULL;
    WndClass.hbrBackground        = NULL;
    WndClass.hCursor              = LoadCursor(NULL, IDC_ARROW);
    WndClass.hIcon                = LoadIcon(hInstance, "KermitIcon");

    return(RegisterClass(&WndClass) != NULL);
}