/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMWND.C                                    **
**                                                                            **
**  This module contains the routines  to handle the MS Windows housekeeping  **
**  that is required during a Kermit protocol.  Includes a status window,     **
**  popup windows for messages, etc.                                          **
**                                                                            **
*******************************************************************************/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include "kermprot.h"

/* LOCAL DATA ----------------------------------------------------------------*/

PktBlk  PktDef  = {{0, 0, 1, 13, 35}, {0, 0, 1, 13, 35}};
ProtBlk ProtDef = {90, 90, 5, 7, 5, 0, 0, 0, 0, 1, 1, 0, 1};

static char    szCurModule [64] = "";

/*----------------------------------------------------------------------------*/
VOID PRVFUNC DrawStProt(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    SetStatus(WID_STPROT, "Kermit Protocol");
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC ShowRetries(int Retries)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    SetDlgItemInt(hWndStat, IDD_RETRIES, Retries, FALSE);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC ShowPackets(int Packets)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    SetDlgItemInt(hWndStat, IDD_PACKETS, Packets, FALSE);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC ShowBytes(long Bytes)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    char ByteStr[20];

    if (filsiz > 0)
        wsprintf(ByteStr, "%li, %li%%", Bytes, (Bytes * 100) / filsiz);
    else
        wsprintf(ByteStr, "%li", Bytes);

    SetDlgItemText(hWndStat, IDD_BYTES, ByteStr);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC ShowTypeIn(char TypeIn)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    static char TypeStr[] = "\0\0";

    TypeStr[0] = TypeIn;
    SetDlgItemText(hWndStat, IDD_TYPEIN, TypeStr);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC ShowTypeOut(char TypeOut)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    static char TypeStr [] = "\0\0";

    TypeStr[0] = TypeOut;
    SetDlgItemText(hWndStat, IDD_TYPEOUT, TypeStr);
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export StatusDlgProc(HWND X(hDlg), UINT X(message),
                                     WPARAM X(wParam), LPARAM X(lParam))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    return (FALSE);
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC SendDlgUpdate(HWND hDlg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int  nFileCnt;
    char szFileCnt[32];

    nFileCnt = ListBox_GetCount(GetDlgItem(hDlg, IDD_SENDLIST));

    wsprintf(szFileCnt, "%i Files", nFileCnt);
    SetDlgItemText(hDlg, IDD_SENDCNT, szFileCnt);

    EnableWindow(GetDlgItem(hDlg, IDD_SEND), nFileCnt > 0);
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC SendDlgAdd(HWND hDlg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    char     szFileSpec[1024];
    char     szFileName[80];
    PSTR     pszFileName;
    OFSTRUCT OpenBuf;
        char *   pszTok = NULL;

    GetDlgItemText(hDlg, IDD_FILE, szFileSpec, sizeof(szFileSpec) - 1);

    pszFileName = strtok_s(szFileSpec, " ", &pszTok);

    while (pszFileName != NULL) {
        if (OpenFile(pszFileName, &OpenBuf, OF_PARSE) == -1) {
            pszFileName = strtok_s(NULL, " ", &pszTok);
            continue;
        }

        lstrcpy(szFileName, (PSTR)OpenBuf.szPathName);

        if (strchr(szFileName, '*') || strchr(szFileName, '?')) {
            if (ErrDlgDirList(hDlg, szFileName, NULL, NULL, 0))
                ListBox_InsertString(GetDlgItem(hDlg, IDD_SENDLIST),
                                     -1, OpenBuf.szPathName);
        }
        else {
            if (OpenFile(szFileName, &OpenBuf, OF_EXIST | OF_READ) != -1)
                ListBox_InsertString(GetDlgItem(hDlg, IDD_SENDLIST),
                                     -1, OpenBuf.szPathName);
        }

        pszFileName = strtok_s(NULL, " ", &pszTok);
    }

    SendDlgUpdate(hDlg);
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC SendDlgList(HWND hDlg, PSTR pszFileSpec)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int  nEditLen;
    char szFileName [80];

    GetDlgItemText(hDlg, IDD_FILE, szFileName, sizeof(szFileName) - 1);

    nEditLen = strlen(szFileName);
    while (nEditLen > 0 && szFileName[nEditLen] <= ' ')
        nEditLen--;

    if (nEditLen == 0 || szFileName[0] == ' ') {
        MessageBeep(0);
        return;
    }

    if (szFileName[nEditLen] == '\\' || szFileName[nEditLen] == ':')
        lstrcat(szFileName, pszFileSpec);

    if (!ErrDlgDirList(hDlg, szFileName, IDD_FILELIST, NULL, 0)) {
        SetFocus(GetDlgItem(hDlg, IDD_FILE));
        MessageBeep(0);
        return;
    }

    ErrDlgDirList(hDlg, szFileName, IDD_DIRLIST, IDD_CURDIR, 0xC010);
    SetDlgItemText(hDlg, IDD_FILE, pszFileSpec);
    SetFocus(GetDlgItem(hDlg, IDD_FILE));
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC SendDlgDelete(HWND hDlg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int nCount;
    int nItem;
    int nSel;

    nCount = ListBox_GetCount(GetDlgItem(hDlg, IDD_SENDLIST));

    if (nCount == LB_ERR)
        return;

    for (nItem = nCount - 1; nItem >= 0; nItem--) {
        nSel = ListBox_GetSel(GetDlgItem(hDlg, IDD_SENDLIST), nItem);

        if (nSel == LB_ERR)
            break;

        if (nSel)
            ListBox_DeleteString(GetDlgItem(hDlg, IDD_SENDLIST), nItem);
    }

    SendDlgUpdate(hDlg);
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC SendDlgSelChg(HWND hDlg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int  nCount;
    int  nItem;
    int  nSel;
    char szFileName [80];
    char szFileList [1024];

    SetDlgItemText(hDlg, IDD_FILE, "");

    nCount = ListBox_GetCount(GetDlgItem(hDlg, IDD_FILELIST));

    if (nCount == LB_ERR)
        return;

    szFileList[0] = '\0';

    for (nItem = 0; nItem < nCount; nItem++) {
        nSel = ListBox_GetSel(GetDlgItem(hDlg, IDD_FILELIST), nItem);

        if (nSel == LB_ERR)
            break;

        if (nSel) {
            ListBox_GetText(GetDlgItem(hDlg, IDD_FILELIST),
                            nItem, szFileName);
            if (strlen(szFileList) + strlen(szFileName) + 2 < sizeof(szFileList)) {
                lstrcat(szFileList, szFileName);
                lstrcat(szFileList, " ");
            }
        }
    }

    SetDlgItemText(hDlg, IDD_FILE, szFileList);
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC SendDlgSend(HWND hDlg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int  nCount;
    int  nItem;
    char szFileSpec[80];

    nCount = ListBox_GetCount(GetDlgItem(hDlg, IDD_SENDLIST));

    if (nCount == LB_ERR || nCount == 0) {
        MessageBeep(0);
        return(FALSE);
    }

    hSendList = CreateWindow("LISTBOX", "SendList", WS_CHILDWINDOW,
                             0, 0, 0, 0, hAppWnd, NULL, hAppInst, NULL);

    if (!hSendList) {
        MessageBeep(0);
        return(FALSE);
    }

    for (nItem = 0; nItem < nCount; nItem++) {
        if (ListBox_GetText(GetDlgItem(hDlg, IDD_SENDLIST),
                            nItem, szFileSpec) == LB_ERR)
            break;
        ListBox_AddString(hSendList, szFileSpec);
    }

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export SendDlgProc(HWND hDlg, UINT message,
                                  WPARAM wParam, LPARAM lParam)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    static char szFileSpec[80];

    switch (message) {
        case WM_INITDIALOG:
            lstrcpy(szFileSpec, "*.*");
            DlgDirList(hDlg, szFileSpec, IDD_DIRLIST, IDD_CURDIR, 0xC010);
            DlgDirList(hDlg, szFileSpec, IDD_FILELIST, NULL, 0);
            SetDlgItemText(hDlg, IDD_FILE, szFileSpec);
            SendDlgUpdate(hDlg);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDD_DIRLIST:
#ifdef _WIN32
                    switch(HIWORD(wParam)) {
#else
                    switch(HIWORD(lParam)) {
#endif
                        char szWork[80];

                        case LBN_DBLCLK:
                            SendMessage(hDlg, WM_COMMAND, IDD_LIST, 0L);
                            break;

                        case LBN_SELCHANGE:
                            if (DlgDirSelectEx(hDlg, szWork, sizeof(szWork), IDD_DIRLIST)) {
                                lstrcat(szWork, szFileSpec);
                                SetDlgItemText(hDlg, IDD_FILE, szWork);
                            }
                            break;
                    }
                    break;

                case IDD_FILELIST:
#ifdef _WIN32
                    switch(HIWORD(wParam)) {
#else
                    switch(HIWORD(lParam)) {
#endif
                        case LBN_DBLCLK:
                            SendMessage(hDlg, WM_COMMAND, IDD_ADD, 0L);
                            break;

                        case LBN_SELCHANGE:
                            SendDlgSelChg(hDlg);
                            break;
                    }
                    break;

                case IDD_SENDLIST:
#ifdef _WIN32
                    lParam = 0;
                    switch(HIWORD(wParam)) {
#else
                    switch(HIWORD(lParam)) {
#endif
                        case LBN_DBLCLK:
                            SendMessage(hDlg, WM_COMMAND, IDD_DELETE, 0L);
                            break;

                        case LBN_SELCHANGE:
                            break;
                    }
                    break;

                case IDD_ADD:
                    SendDlgAdd(hDlg);
                    break;

                case IDD_LIST:
                    SendDlgList(hDlg, szFileSpec);
                    break;

                case IDD_DELETE:
                    SendDlgDelete(hDlg);
                    break;

                case IDD_SEND:
                    if (SendDlgSend(hDlg))
                        EndDialog(hDlg, (int)hSendList);
                    break;

                case IDD_CANCEL:
                    EndDialog(hDlg, NULL);
                    break;

                default:
                    return(FALSE);
            }

        default:
           return(FALSE);
    }
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export GetDlgProc(HWND hDlg, unsigned message,
                                 UINT wParam, LONG X(lParam))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDD_OK:
                    GetDlgItemText(hDlg, IDD_PARMS, cmarg, 80);
                    EndDialog(hDlg, TRUE);
                    break;

                case IDD_CANCEL:
                    EndDialog(hDlg, FALSE);
                    break;

                default:
                    return(FALSE);
                    break;
            }
            break;

        case WM_INITDIALOG:
            LIMITTEXT(hDlg, IDD_PARMS, cmarg);
            break;

        default:
            return(FALSE);
    }
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export HostDlgProc(HWND hDlg, unsigned message,
                                  UINT wParam, LONG X(lParam))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDD_OK:
                    GetDlgItemText(hDlg, IDD_PARMS, cmarg, 80);
                    EndDialog(hDlg, TRUE);
                    break;

                case IDD_CANCEL:
                    EndDialog(hDlg, FALSE);
                    break;

                default:
                    return(FALSE);
                    break;
            }
            break;

        case WM_INITDIALOG:
            LIMITTEXT(hDlg, IDD_PARMS, cmarg);
            break;

        default:
            return(FALSE);
    }
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export GenericDlgProc(HWND hDlg, unsigned message,
                                     UINT wParam, LONG X(lParam))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    static char CommandTable[] = "ICLFDUETRKWMHQPJV";
    static char CommandCode;
    char Parm1[80], Parm2[80], Parm3[80];

    switch (message) {
        case WM_COMMAND:
            if (LOWORD(wParam) >= IDD_GCMDI && LOWORD(wParam) <= IDD_GCMDV) {
                CommandCode = CommandTable[LOWORD(wParam) - IDD_GCMDI];
                CheckRadioButton(hDlg, IDD_GCMDI, IDD_GCMDV, LOWORD(wParam));
                return(TRUE);
            }

            switch (LOWORD(wParam)) {
                case IDD_OK:
                    GetDlgItemText(hDlg, IDD_PARM1, Parm1, 80);
                    GetDlgItemText(hDlg, IDD_PARM2, Parm2, 80);
                    GetDlgItemText(hDlg, IDD_PARM3, Parm3, 80);
                    memset(cmarg, '\0', sizeof(cmarg));
                    cmarg[0] = CommandCode;
                    if (lstrlen(Parm1) > 0 &&
                        lstrlen(cmarg) + lstrlen(Parm1) + 1 < sizeof(cmarg)) {
                        cmarg[lstrlen(cmarg)] = tochar(lstrlen(Parm1));
                        lstrcat(cmarg, Parm1);
                    }
                    if (lstrlen(Parm2) > 0 &&
                        lstrlen(cmarg) + lstrlen(Parm2) + 1 < sizeof(cmarg)) {
                        cmarg[lstrlen(cmarg)] = tochar(lstrlen(Parm2));
                        lstrcat(cmarg, Parm2);
                    }
                    if (lstrlen(Parm3) > 0 &&
                        lstrlen(cmarg) + lstrlen(Parm3) + 1 < sizeof(cmarg)) {
                        cmarg[lstrlen(cmarg)] = tochar(lstrlen(Parm3));
                        lstrcat(cmarg, Parm3);
                    }
                    EndDialog(hDlg, TRUE);
                    break;

                case IDD_CANCEL:
                    EndDialog(hDlg, FALSE);
                    break;

                default:
                    return(FALSE);
                    break;
            }
            break;

        case WM_INITDIALOG:
            CHKBUTTON(hDlg, IDD_GCMDI, IDD_GCMDV, 0);
            LIMITTEXT(hDlg, IDD_PARM1, Parm1);
            LIMITTEXT(hDlg, IDD_PARM2, Parm2);
            LIMITTEXT(hDlg, IDD_PARM3, Parm3);
            break;

        default:
            return(FALSE);
    }
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC KermitCheck
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Poll appropriate poll(ing) functions based on status                       */
/*                                                                            */
(VOID)                 /* No parameters.                                      */
/*----------------------------------------------------------------------------*/

{
    DoKermit();

    if (bEndKermit)
        return FALSE;

    if (bAbort && bAbortConfirm) {
        MessageBeep(0);
        if (MessageBox(hAppWnd, "Abort Kermit Protocol in Progress",
                       szAppName, MB_OKCANCEL | MB_ICONQUESTION) != IDD_OK)
            bAbort = FALSE;
    }

    if (!bCommBusy && !GetQueueStatus(QS_ALLINPUT))
        WaitMessage();

    return TRUE;
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC StartKermit(int Command)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    lpXfrDlgProc = MakeProcInstance((FARPROC)StatusDlgProc, hAppInst);
    hWndStat = CreateDialog(hAppInst, "StatusDlgBox", hAppWnd,
                           (DLGPROC)lpXfrDlgProc);
    ShowWindow(hWndStat, SW_SHOWNA);

    SetKermitMenu("StopMenu");

//    tmsg("Starting Kermit (Command: %c)", Command);

    tinit();
    start = Command;
    bKermit = TRUE;
    SetMessage(ML_PROTOCOL, "Kermit Protocol Active...");

//    DoKermit();

    bEndKermit = FALSE;

    while (KermitCheck() && !bAbort)
        MessagePump();

    tterm();

    SetKermitMenu("StartMenu");

    if (hSendList) {
        DestroyWindow(hSendList);
        hSendList = NULL;
    }

    DestroyWindow(hWndStat);
    hWndStat = NULL;
    X32(FreeProcInstance(lpXfrDlgProc);)
    lpXfrDlgProc = NULL;
    bKermit = FALSE;
    SetMessage(ML_PROTOCOL, NULL);

//    tmsg("Kermit Finished (Result: %i), Ribbit...\n\r", Result);

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC EndKermit(int X(Result))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    bEndKermit = TRUE;
    return TRUE;

//    tterm();

//    SetKermitMenu("StartMenu");

//    if (hSendList) {
//        DestroyWindow(hSendList);
//        hSendList = NULL;
//    }

//    DestroyWindow(hWndStat);
//    hWndStat = NULL;
//    X32(FreeProcInstance(lpXfrDlgProc);)
//    lpXfrDlgProc = NULL;
//    bKermit = FALSE;
//    SetMessage(ML_PROTOCOL, NULL);

//    tmsg("Kermit Finished (Result: %i), Ribbit...\n\r", Result);

//    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC AbortXfer(BOOL bVerify)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (bVerify) {
        MessageBeep(0);
        if (MessageBox(hAppWnd, "Abort Kermit Protocol in Progress",
                       szAppName, MB_OKCANCEL | MB_ICONQUESTION) != IDD_OK)
            return(FALSE);
    }

    EndKermit(-1);
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC KermitUserInt(UINT IntCmd)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    switch (IntCmd) {
        case IDM_CANFILE:
            tmsg("File interrupt");
            cx = TRUE;
            break;

        case IDM_CANBATCH:
            tmsg("File group interrupt");
            cz = TRUE;
            break;

        case IDM_RETRY:
            cr = TRUE;
            break;

        case IDM_STOP:
            tmsg("User exit");
            ce = TRUE;
            break;

        case IDM_ABORT:
            tmsg("User abort");
            cc = TRUE;
            break;
    }
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC KermitSend()

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    BOOL Result;

    if (!GoDialogBox(hAppInst, "SendDlgBox", hAppWnd, (FARPROC)SendDlgProc))
        return(FALSE);

    nfils = 0;
    Result = StartKermit(KERMITSEND);

    return(Result);
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC KermitReceive()

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    return(StartKermit(KERMITRECEIVE));
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC KermitServer()

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    return(StartKermit(KERMITSERVER));
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC KermitGet()

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (GoDialogBox(hAppInst, "GetDlgBox", hAppWnd, (FARPROC)GetDlgProc))
        return(StartKermit(KERMITGET));
    else
        return(FALSE);
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC KermitHost()

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (GoDialogBox(hAppInst, "HostDlgBox", hAppWnd, (FARPROC)HostDlgProc))
        return(StartKermit(KERMITHOST));
    else
        return(FALSE);
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC KermitGeneric()

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (GoDialogBox(hAppInst, "GenericDlgBox", hAppWnd, (FARPROC)GenericDlgProc))
        return(StartKermit(KERMITGENERIC));
    else
        return(FALSE);
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC PacketsDlgInit(HWND hDlg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    SetDlgItemInt(hDlg, IDD_SPADCHR, PktSet.Send.PadChar, FALSE);
    SetDlgItemInt(hDlg, IDD_RPADCHR, PktSet.Recv.PadChar, FALSE);
    SetDlgItemInt(hDlg, IDD_SPADCNT, PktSet.Send.PadCount, FALSE);
    SetDlgItemInt(hDlg, IDD_RPADCNT, PktSet.Recv.PadCount, FALSE);
    SetDlgItemInt(hDlg, IDD_SPKTSOP, PktSet.Send.StartChar, FALSE);
    SetDlgItemInt(hDlg, IDD_RPKTSOP, PktSet.Recv.StartChar, FALSE);
    SetDlgItemInt(hDlg, IDD_SPKTEOL, PktSet.Send.EndChar, FALSE);
    SetDlgItemInt(hDlg, IDD_RPKTEOL, PktSet.Recv.EndChar, FALSE);
    SetDlgItemInt(hDlg, IDD_SCTLPFX, PktSet.Send.CtlPrefix, FALSE);
    SetDlgItemInt(hDlg, IDD_RCTLPFX, PktSet.Recv.CtlPrefix, FALSE);

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC PacketsDlgTerm(HWND hDlg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    PktBlk PktWrk;

    memcpy(&PktWrk, &PktSet, sizeof(PktBlk));

    if (!GetValidDlgItemInt(hDlg, IDD_SPADCHR, FALSE,
                            (UINT *)&PktWrk.Send.PadChar, 0, 31))
        return(FALSE);
    if (!GetValidDlgItemInt(hDlg, IDD_RPADCHR, FALSE,
                            (UINT *)&PktWrk.Recv.PadChar, 0, 31))
        return(FALSE);

    if (!GetValidDlgItemInt(hDlg, IDD_SPADCNT, FALSE,
                            (UINT *)&PktWrk.Send.PadCount, 0, 99))
        return(FALSE);
    if (!GetValidDlgItemInt(hDlg, IDD_RPADCNT, FALSE,
                            (UINT *)&PktWrk.Recv.PadCount, 0, 99))
        return(FALSE);

    if (!GetValidDlgItemInt(hDlg, IDD_SPKTSOP, FALSE,
                            (UINT *)&PktWrk.Send.StartChar, 0, 31))
        return(FALSE);
    if (!GetValidDlgItemInt(hDlg, IDD_RPKTSOP, FALSE,
                            (UINT *)&PktWrk.Recv.StartChar, 0, 31))
        return(FALSE);

    if (!GetValidDlgItemInt(hDlg, IDD_SPKTEOL, FALSE,
                            (UINT *)&PktWrk.Send.EndChar, 0, 31))
        return(FALSE);
    if (!GetValidDlgItemInt(hDlg, IDD_RPKTEOL, FALSE,
                            (UINT *)&PktWrk.Recv.EndChar, 0, 31))
        return(FALSE);

    if (!GetValidDlgItemInt(hDlg, IDD_SCTLPFX, FALSE,
                            (UINT *)&PktWrk.Send.CtlPrefix, 33, 126))
        return(FALSE);
    if (!GetValidDlgItemInt(hDlg, IDD_RCTLPFX, FALSE,
                            (UINT *)&PktWrk.Recv.CtlPrefix, 33, 126))
        return(FALSE);

    memcpy(&PktSet, &PktWrk, sizeof(PktBlk));

    bChanged = TRUE;
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export PacketsDlgProc(HWND hDlg, unsigned message,
                                     UINT wParam, LONG X(lParam))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDD_OK:
                    if (!PacketsDlgTerm(hDlg))
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
            return(PacketsDlgInit(hDlg));

        default:
            return(FALSE);
    }
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC ProtocolDlgInit(HWND hDlg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    SetDlgItemInt(hDlg, IDD_SPKTSIZE, ProtSet.SendPktSize, FALSE);
    SetDlgItemInt(hDlg, IDD_RPKTSIZE, ProtSet.RecvPktSize, FALSE);
    SetDlgItemInt(hDlg, IDD_STIMEOUT, ProtSet.SendTimeout, FALSE);
    SetDlgItemInt(hDlg, IDD_RTIMEOUT, ProtSet.RecvTimeout, FALSE);
    SetDlgItemInt(hDlg, IDD_SLIMIT, ProtSet.RetryLimit, FALSE);
    CheckRadioButton(hDlg, IDD_BCHK1, IDD_BCHK3,
                     (ProtSet.BlockCheck + IDD_BCHK1));
    CheckDlgButton(hDlg, IDD_PKTDBG, ProtSet.DebugPacket);
    CheckDlgButton(hDlg, IDD_STADBG, ProtSet.DebugState);
    CheckDlgButton(hDlg, IDD_OTHDBG, ProtSet.DebugOther);
    CheckDlgButton(hDlg, IDD_ATRCAP, ProtSet.Attributes);
    SetDlgItemInt(hDlg, IDD_WNDSIZE, ProtSet.WndSize, FALSE);

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC ProtocolDlgTerm(HWND hDlg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    ProtBlk ProtWrk;

    memcpy(&ProtWrk, &ProtSet, sizeof(ProtBlk));

    if (!GetValidDlgItemInt(hDlg, IDD_SPKTSIZE, FALSE, (UINT *)&ProtWrk.SendPktSize,
                           20, 1000))
        return(FALSE);
    if (!GetValidDlgItemInt(hDlg, IDD_RPKTSIZE, FALSE, (UINT *)&ProtWrk.RecvPktSize,
                           20, 1000))
        return(FALSE);

    if (!GetValidDlgItemInt(hDlg, IDD_STIMEOUT, FALSE,
                            (UINT *)&ProtWrk.SendTimeout, 0, 99))
        return(FALSE);
    if (!GetValidDlgItemInt(hDlg, IDD_RTIMEOUT, FALSE,
                           (UINT *)&ProtWrk.RecvTimeout, 0, 99))
        return(FALSE);

    if (!GetValidDlgItemInt(hDlg, IDD_SLIMIT, FALSE,
                           (UINT *)&ProtWrk.RetryLimit, 0, 99))
        return(FALSE);

    ProtWrk.BlockCheck = (WORD)(GetRadioButton(hDlg, IDD_BCHK1, IDD_BCHK3)
                                - IDD_BCHK1);

    ProtWrk.DebugPacket = (WORD)IsDlgButtonChecked(hDlg, IDD_PKTDBG);
    ProtWrk.DebugState = (WORD)IsDlgButtonChecked(hDlg, IDD_STADBG);
    ProtWrk.DebugOther = (WORD)IsDlgButtonChecked(hDlg, IDD_OTHDBG);

    ProtWrk.Attributes = (WORD)IsDlgButtonChecked(hDlg, IDD_ATRCAP);

    if (!GetValidDlgItemInt(hDlg, IDD_WNDSIZE, FALSE,
                           (UINT *)&ProtWrk.WndSize, 1, 31))
        return(FALSE);

    memcpy(&ProtSet, &ProtWrk, sizeof(ProtBlk));

    bChanged = TRUE;
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export ProtocolDlgProc(HWND hDlg, unsigned message,
                                      UINT wParam, LONG X(lParam))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDD_BCHK1:
                case IDD_BCHK2:
                case IDD_BCHK3:
                    CheckRadioButton(hDlg, IDD_BCHK1, IDD_BCHK3, LOWORD(wParam));
                    break;

                case IDD_PACKETS:
                    GoDialogBox(hAppInst, "PacketsDlgBox",
                                hDlg, (FARPROC)PacketsDlgProc);
                    break;

                case IDD_OK:
                    if (!ProtocolDlgTerm(hDlg))
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
            return(ProtocolDlgInit(hDlg));

        default:
            return(FALSE);
    }
    return(TRUE);
}

BOOL PUBFUNC PMInit(VOID)
{
//    bKermit = FALSE;
//    SetKermitMenu("StartMenu");

    memcpy(&PktSet,  &PktDef,  sizeof(PktSet));
    memcpy(&ProtSet, &ProtDef, sizeof(ProtSet));

//    DrawStProt();

    return(TRUE);
}

BOOL PUBFUNC PMLoad(PSTR pszModule, BOOL bReset)
{
    if (bReset || strcmp(szCurModule, pszModule) != 0) {

        lstrcpy(szCurModule, pszModule);

        memcpy(&PktSet,  &PktDef,  sizeof(PktSet));
        memcpy(&ProtSet, &ProtDef, sizeof(ProtSet));
    }

    bKermit = FALSE;
    SetKermitMenu("StartMenu");

    DrawStProt();

    return(TRUE);
}

BOOL PUBFUNC PMSetConfig(UINT wInfoSize, PSTR pszInfo)
{
    if (bKermit)
        return(FALSE);

    if (wInfoSize > sizeof(ProtSet))
        return(FALSE);

    memcpy(&PktSet,  &PktDef,  sizeof(PktSet));
/*    memcpy(&PktSet, pszInfo, wInfoSize); */ /* we need to imbed Pkt in Prot!!! */

    memcpy(&ProtSet,  &ProtDef,  sizeof(ProtSet));
    memcpy(&ProtSet, pszInfo, wInfoSize);

    DrawStProt();

    return(TRUE);
}

BOOL PUBFUNC PMGetConfig(UINT * pwInfoSize, PSTR pszInfo)
{
    *pwInfoSize = sizeof(ProtSet);
    memcpy(pszInfo, &ProtSet, sizeof(ProtSet));

    return(TRUE);
}

BOOL PUBFUNC PMSetup(HINSTANCE hInst, HWND hWnd)
{
    if (GoDialogBox(hInst, "ProtocolDlgBox", hWnd, (FARPROC)ProtocolDlgProc)) {
        DrawStProt();
        bChanged = TRUE;
    }

    return(TRUE);
}
