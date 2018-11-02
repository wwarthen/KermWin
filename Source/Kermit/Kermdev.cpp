/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMDEV.C                                    **
**                                                                            **
**  This module contains the procedures to handle the communications          **
**  device.                                   **
**                                                                            **
*******************************************************************************/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"

/* LOCAL DATA ----------------------------------------------------------------*/

#pragma pack(2)
typedef struct {
    WORD wDevMode;  /* 0=Local, 1=Dial, 2=Answer */
    char szNumber[40];
    char szDialInit[40];
    char szDialCmd[10];
    char szAnsInit[40];
    char szHangup[10];
    char szEscape[10];
    char szConnect[20];
    WORD wDialCnt;
    WORD wDialWait;
    WORD wAnsWait;
} DevBlk;
#pragma pack()

DevBlk      DevSet;

DevBlk      DevDef = {0, "", "ATE1Q0V1", "ATDT", "ATE1Q0V1S0=1",
                      "ATH", "+++", "CONNECT", 10, 60, 45};

#define NUM_DIALCMDS    3

#define DC_NONE         0
#define DC_ABORT        1
#define DC_REDIAL       2

static  int     nDialCmd;
static  BOOL    bHonorCD;
static  char    szCurModule [64] = "";

VOID PUBFUNC PauseMon(DWORD dwPauseTime, WATCHPROC fnWatchProc)
{
    DWORD   dwStartTick;

    DebMsg(DL_INFO, "Entering PauseMon...");

    dwStartTick = GetTickCount();

    while (!(bAbort || bScriptAbort))
    {
        char szWork[2];

        if (ReadCommStr(szWork, 1))
            WriteTermStr(szWork, 1, FALSE);

        if ((fnWatchProc != NULL) && !((*fnWatchProc)((GetTickCount() - dwStartTick), dwPauseTime)))
            break;

        if ((GetTickCount() - dwStartTick) >= dwPauseTime)
            break;

        if (!bCommBusy && (GetQueueStatus(QS_ALLINPUT) == 0))
            WaitMessage();

        MessagePump();
    }

    DebMsg(DL_INFO, "Leaving PauseMon...");

    return;
}

VOID PUBFUNC Pause(DWORD dwPauseTime)
{
    PauseMon(dwPauseTime, NULL);
}

BOOL PUBFUNC WaitMon(PSTR pszStr, DWORD dwWaitTime, WATCHPROC fnWatchProc)
{
    DWORD   dwStartTick;
    char    szWaitStr[80];
    int     nIndex;

    if (pszStr == NULL)
        return TRUE;

    lstrcpy(szWaitStr, pszStr);
    dwStartTick = GetTickCount();
    nIndex = 0;

    while (!(bAbort || bScriptAbort))
    {
        char szWork[2];

        if (ReadCommStr(szWork, 1))
        {
            WriteTermStr(szWork, 1, FALSE);

            if (szWork[0] == szWaitStr[nIndex])
            {
                if (szWaitStr[++nIndex] == '\0')
                    return TRUE;
            }
            else
                nIndex = 0;
        }

        if ((fnWatchProc != NULL) && !((*fnWatchProc)((GetTickCount() - dwStartTick), dwWaitTime)))
            break;

        if ((GetTickCount() - dwStartTick) >= dwWaitTime)
            break;

        if (!bCommBusy && (GetQueueStatus(QS_ALLINPUT) == 0))
            WaitMessage();

        MessagePump();
    }

    return FALSE;
}

BOOL PUBFUNC Wait(PSTR pszStr, DWORD dwTime)
{
    return WaitMon(pszStr, dwTime, NULL);
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC DrawStDevice(VOID)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    if (GetCommInfo(GCI_CONNTYPE) == CT_TCPIP) {
        SetStatus(WID_STDEV, "Local (Network)");
        return;
    }

    switch (DevSet.wDevMode) {
        case 0:
            SetStatus(WID_STDEV, "Local (Serial)");
            break;

        case 1:
            SetStatus(WID_STDEV, "Hayes (Dial)");
            break;

        case 2:
            SetStatus(WID_STDEV, "Hayes (Answer)");
            break;
    }
}

BOOL CALLBACK __export DialDlgProc(HWND hDlg, unsigned message,
                  UINT wParam, LONG /* lParam */ )
{
    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
        case IDD_OK:    /* redial */
                    nDialCmd = DC_REDIAL;
                    EnableWindow(GetDlgItem(hDlg, IDD_OK), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDD_CANCEL), FALSE);
                    break;

        case IDD_CANCEL:    /* abort */
                    nDialCmd = DC_ABORT;
                    EnableWindow(GetDlgItem(hDlg, IDD_OK), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDD_CANCEL), FALSE);
                    break;

                default:
                    return(FALSE);
                    break;
            }
            break;

        default:
            return(FALSE);
    }

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC DeviceDlgInit(HWND hDlg)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    CHKBUTTON(hDlg, IDD_LOCAL, IDD_ANSWER, DevSet.wDevMode);
    SetDlgItemText(hDlg, IDD_NUMBER, DevSet.szNumber);

    SetDlgItemText(hDlg, IDD_DIALINIT, DevSet.szDialInit);
    SetDlgItemText(hDlg, IDD_DIALCMD, DevSet.szDialCmd);
    SetDlgItemText(hDlg, IDD_ANSINIT, DevSet.szAnsInit);
    SetDlgItemText(hDlg, IDD_HANGUP, DevSet.szHangup);
    SetDlgItemText(hDlg, IDD_ESCAPE, DevSet.szEscape);
    SetDlgItemText(hDlg, IDD_CONNECT, DevSet.szConnect);

    SetDlgItemInt(hDlg, IDD_DIALCNT, DevSet.wDialCnt, FALSE);
    SetDlgItemInt(hDlg, IDD_DIALWAIT, DevSet.wDialWait, FALSE);
    SetDlgItemInt(hDlg, IDD_ANSWAIT, DevSet.wAnsWait, FALSE);

    LIMITTEXT(hDlg, IDD_DIALINIT, DevSet.szDialInit);
    LIMITTEXT(hDlg, IDD_DIALCMD, DevSet.szDialCmd);
    LIMITTEXT(hDlg, IDD_ANSINIT, DevSet.szAnsInit);
    LIMITTEXT(hDlg, IDD_HANGUP, DevSet.szHangup);
    LIMITTEXT(hDlg, IDD_ESCAPE, DevSet.szEscape);
    LIMITTEXT(hDlg, IDD_CONNECT, DevSet.szConnect);

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC DeviceDlgTerm(HWND hDlg)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    DevBlk DevWrk;

    memcpy((void *)&DevWrk, (void *)&DevSet, sizeof(DevBlk));

    DevWrk.wDevMode = (WORD)(GetRadioButton(hDlg, IDD_LOCAL, IDD_ANSWER) - IDD_LOCAL);
    GetDlgItemText(hDlg, IDD_NUMBER, DevWrk.szNumber, sizeof(DevWrk.szNumber));

    GetDlgItemText(hDlg, IDD_DIALINIT, DevWrk.szDialInit, sizeof(DevWrk.szDialInit));
    GetDlgItemText(hDlg, IDD_DIALCMD, DevWrk.szDialCmd, sizeof(DevWrk.szDialCmd));
    GetDlgItemText(hDlg, IDD_ANSINIT, DevWrk.szAnsInit, sizeof(DevWrk.szAnsInit));
    GetDlgItemText(hDlg, IDD_HANGUP, DevWrk.szHangup, sizeof(DevWrk.szHangup));
    GetDlgItemText(hDlg, IDD_ESCAPE, DevWrk.szEscape, sizeof(DevWrk.szEscape));
    GetDlgItemText(hDlg, IDD_CONNECT, DevWrk.szConnect, sizeof(DevWrk.szConnect));

    DevWrk.wDialCnt = (WORD)GetDlgItemInt(hDlg, IDD_DIALCNT, NULL, FALSE);
    DevWrk.wDialWait = (WORD)GetDlgItemInt(hDlg, IDD_DIALWAIT, NULL, FALSE);
    DevWrk.wAnsWait = (WORD)GetDlgItemInt(hDlg, IDD_ANSWAIT, NULL, FALSE);

    memcpy(&DevSet, &DevWrk, sizeof(DevBlk));

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export DeviceDlgProc(HWND hDlg, unsigned message,
                    UINT wParam, LONG X(lParam))

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDD_OK:
                    if (!DeviceDlgTerm(hDlg))
                        break;

                case IDD_CANCEL:
                    EndDialog(hDlg, TRUE);
                    break;

                default:
                    return(FALSE);
                    break;
            }
            break;

        case WM_INITDIALOG:
            return(DeviceDlgInit(hDlg));

        default:
            return(FALSE);
    }
    return(TRUE);
}

BOOL DialWatchProc(DWORD dwElapsed, DWORD dwLimit)
{
    char szBuf [40];

    wsprintf(szBuf, "Waiting (%i of %i seconds)...", (int)(dwElapsed / 1000), (int)(dwLimit / 1000));
    SetDlgItemText(hWndStat, IDD_STATUS, szBuf);

    return (nDialCmd == DC_NONE);
}

BOOL PUBFUNC DMConnect(VOID)
{
    char    szBuf[80];
    char    szStatusMsg[80];
    BOOL    bSucceed;
    UINT    wDialAttempts;

    bHonorCD = FALSE;

    if ((GetCommInfo(GCI_CONNTYPE) == CT_TCPIP) || (DevSet.wDevMode == 0))
    {
        if (!Connect())
            return(FALSE);
        bConnected = TRUE;

        SetTimer(ST_START);
        CheckMenuItem(GetMenu(hAppWnd), IDM_CONNECT, MF_CHECKED);
        wsprintf(szStatusMsg, "Connected");
        SetMessage(ML_SESSION, szStatusMsg);
        SessOnConnect();
        return(TRUE);
    }

    if (DevSet.wDevMode == 2)
    {
        if (!Connect())
            return(FALSE);
        bConnected = TRUE;

        SetMessage(ML_SCRIPT, "Initializing Modem...");

        SetCommDTRState(TRUE);
        Pause(100);
        SetCommDTRState(FALSE);
        Pause(100);
        SetCommDTRState(TRUE);

        WriteCommStr(DevSet.szAnsInit, 0);
        WriteCommStr("\r", 0);
        Pause(1000);

        SetMessage(ML_SCRIPT, NULL);

        SetTimer(ST_START);
        CheckMenuItem(GetMenu(hAppWnd), IDM_CONNECT, MF_CHECKED);
        wsprintf(szStatusMsg, "Answering %s", (LPSTR)"...");
        SetMessage(ML_SESSION, szStatusMsg);
        SetMessage(ML_SCRIPT, NULL);

        bHonorCD = TRUE;

        SessOnConnect();
        return(TRUE);
    }

    lpXfrDlgProc = MakeProcInstance((FARPROC)DialDlgProc, hAppInst);
    hWndStat = CreateDialog(hAppInst, "DialDlgBox", hAppWnd, (DLGPROC)lpXfrDlgProc);
    ShowWindow(hWndStat, SW_SHOW);
    EnableWindow(hAppWnd, FALSE);
    bSucceed = FALSE;

    for (wDialAttempts = 1; (wDialAttempts <= DevSet.wDialCnt); wDialAttempts++)
    {
        nDialCmd = DC_NONE;
        EnableWindow(GetDlgItem(hWndStat, IDD_OK), TRUE);
        EnableWindow(GetDlgItem(hWndStat, IDD_CANCEL), TRUE);

        wsprintf(szBuf, "%s (%u of %u)", (LPSTR)DevSet.szNumber,
                 wDialAttempts, DevSet.wDialCnt);
        SetDlgItemText(hWndStat, IDD_NUMBER, szBuf);
        SetDlgItemText(hWndStat, IDD_STATUS, "Initializing Modem...");
        UpdateWindow(hWndStat);
        SetMessage(ML_SCRIPT, "Initializing Modem...");

        if (!Connect())
            break;
        bConnected = TRUE;

        SetCommDTRState(TRUE);
        Pause(100);
        SetCommDTRState(FALSE);
        Pause(100);
        SetCommDTRState(TRUE);

        SetDlgItemText(hWndStat, IDD_STATUS, "Dialing Modem...");
        UpdateWindow(hWndStat);
        SetMessage(ML_SCRIPT, "Waiting to Connect...");

        WriteCommStr(DevSet.szDialInit, 0);
        WriteCommStr("\r", 0);
        Pause(1000);
        WriteCommStr(DevSet.szDialCmd, 0);
        WriteCommStr(DevSet.szNumber, 0);
        WriteCommStr("\r", 0);

        bSucceed = WaitMon(DevSet.szConnect, (DevSet.wAnsWait * 1000), DialWatchProc);

        if (bSucceed)
            break;

        DMDisconnect(CD_FORCE);

        {
            static char * szDialCmdMsgs[NUM_DIALCMDS] = {"None", "Aborting...", "Redialing..."};
            if (nDialCmd <= NUM_DIALCMDS)
                SetDlgItemText(hWndStat, IDD_STATUS, szDialCmdMsgs[nDialCmd]);
        }

        Pause(1000);

        if (nDialCmd == DC_ABORT)
            break;
    }

    EnableWindow(hAppWnd, TRUE);
    DestroyWindow(hWndStat);
    hWndStat = NULL;
    X32(FreeProcInstance(lpXfrDlgProc);)
    lpXfrDlgProc = NULL;

    if (bSucceed)
    {
        SetTimer(ST_START);
        CheckMenuItem(GetMenu(hAppWnd), IDM_CONNECT, MF_CHECKED);
        wsprintf(szStatusMsg, "Connected");
        SetMessage(ML_SESSION, szStatusMsg);
        SetDlgItemText(hWndStat, IDD_STATUS, NULL);
        SetMessage(ML_SCRIPT, NULL);
        bHonorCD = TRUE;
        SessOnConnect();
        return TRUE;
    }

    return FALSE;
}

BOOL PUBFUNC DMDisconnect(int bStrategy)
{
    int     i;

    if (bKermit && !AbortXfer(bStrategy == CD_VERIFY))
        return(FALSE);

    if (bStrategy == CD_VERIFY) {
        MessageBeep(0);
        if (KermitFmtMsgBox(MB_OKCANCEL | MB_ICONQUESTION,
                            "Break Kermit Connection") != IDD_OK)
            return(FALSE);
    }

    TermReview(FALSE);

    bHonorCD = FALSE;

    if ((bStrategy == CD_ABORT) || (GetCommInfo(GCI_CONNTYPE) == CT_TCPIP) ||
        (DevSet.wDevMode == 0)) {
        Disconnect();
        bConnected = FALSE;
        SetTimer(ST_STOP);
        SetMessage(ML_SESSION, NULL);
        CheckMenuItem(GetMenu(hAppWnd), IDM_CONNECT, MF_UNCHECKED);
        return(TRUE);
    }

    SetMessage(ML_SCRIPT, "Hanging Up...");

    FlushCommQueue(FCQ_ALLQUEUES);

    Pause(1000);
    for (i = 0; DevSet.szEscape[i] != '\0'; i++) {
        Pause(100);
        WriteCommStr(DevSet.szEscape + i, 1);
    }

    Pause(2000);
    WriteCommStr(DevSet.szHangup, 0);
    WriteCommStr("\r", 0);
    Pause(1000);

    SetCommDTRState(FALSE);
    Pause(100);

    Disconnect();
    bConnected = FALSE;

    SetMessage(ML_SCRIPT, NULL);

    SetTimer(ST_STOP);
    SetMessage(ML_SESSION, NULL);
    CheckMenuItem(GetMenu(hAppWnd), IDM_CONNECT, MF_UNCHECKED);

    return(TRUE);
}

VOID PUBFUNC DMCommEvent(DWORD dwEvent)
{
    switch (dwEvent) {
        case CE_CLOSED:
            if (bHonorCD || (GetCommInfo(GCI_CONNTYPE) == CT_TCPIP)) {
                MessageBeep(0);
                KermitFmtMsgBox(MB_OK | MB_ICONEXCLAMATION, "Host Disconnected!");
                nCloseReq = CAR_DISCONNECTED;
                bAbortConfirm = FALSE;
                bAbort = TRUE;
            }
            break;
    }
}

BOOL PUBFUNC DMInit(VOID)
{
    memcpy(&DevSet, &DevDef, sizeof(DevSet));

//    DrawStDevice();

//    SetTimer(ST_RESET);

    return(TRUE);
}

BOOL PUBFUNC DMLoad(PSTR pszModule, BOOL bReset)
{
    if (bReset || strcmp(szCurModule, pszModule) != 0) {
        lstrcpy(szCurModule, pszModule);
        memcpy(&DevSet, &DevDef, sizeof(DevSet));
    }

    SetTimer(ST_RESET);

    DrawStDevice();

    return(TRUE);
}

BOOL PUBFUNC DMSetConfig(UINT wInfoSize, PSTR pszInfo)
{
    if (wInfoSize > sizeof(DevSet))
        return(FALSE);

    memcpy(&DevSet, &DevDef, sizeof(DevSet));
    memcpy(&DevSet, pszInfo, wInfoSize);

    DrawStDevice();

    return(TRUE);
}

BOOL PUBFUNC DMGetConfig(UINT * pwInfoSize, PSTR pszInfo)
{
    *pwInfoSize = sizeof(DevSet);
    memcpy(pszInfo, &DevSet, sizeof(DevSet));

    return(TRUE);
}

BOOL PUBFUNC DMSetup(HINSTANCE hInst, HWND hWnd)
{
    if (GoDialogBox(hInst, "DeviceDlgBox", hWnd, (FARPROC)DeviceDlgProc)) {
        bChanged = TRUE;
        DrawStDevice();
    }

    return(TRUE);
}