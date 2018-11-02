/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMSESS.C                                   **
**                                                                            **
**  This module contains the terminal session procedures including the        **
**  the session window procedure.                                             **
**                                                                            **
*******************************************************************************/

/* DEFINES -------------------------------------------------------------------*/

/* Module Indexes                                 */
#define MODCNT  4    /* Number of module slots                    */

#define EMULMOD 0   /* Terminal Emulation Module                  */
#define PROTMOD 1   /* Transfer Protocol Module                   */
#define CONNMOD 2   /* Communications Module                      */
#define DEVMOD  3   /* Device Module                          */

/* Setup Slot Ids                                 */
#define CONNSLOT    1   /* Communications setup slot                  */
#define EMULSLOT    2   /* Terminal setup slot                    */
#define PROTSLOT    3   /* Protocol setup slot                    */
#define DEVSLOT     4   /* Packets setup slot (temporary, merge with PROTSLOT)*/
#define LOGSLOT     5   /* Logging setup slot                     */
#define TERMSLOT    6   /* Terminal display setup slot                */

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include "commdlg.h"

/* LOCAL DATA ----------------------------------------------------------------*/

#pragma pack(2)
typedef struct {
    WORD    wId;        /* Id of data slot                    */
    WORD    wLength;    /* Number of bytes occupied by the slot           */
} SLOT;
#pragma pack()

#pragma pack(2)
typedef struct {
    char    szProgId [16];        /* Something like "Kermit-MW v1.00"       */
    char    szDesc [64];          /* Users description of the session       */
    FILNAM  szModNames [MODCNT];
    WORD    bConnect;             /* Auto connect on open?              */
    PATHNAM szScript;
    char    szSessNotes [256];    /* Notes...                   */
    LONG    nLeft;
    LONG    nTop;
    LONG    nWidth;
    LONG    nHeight;
} SessData;
#pragma pack()

SessData CurSess;
SessData WrkSess;

BOOL    bSuppressConnChg;   /* Suppresses Conn Type Chg in Sess Dlg Box */

VOID PRVFUNC SetCaption(VOID)
{
    char szCaption[32];

    lstrcpy(szCaption, szAppName);

    if (szSessFileName[0] != '\0') {
        lstrcat(szCaption, " - ");
        lstrcat(szCaption, strrchr(szSessFileName, '\\') + 1);
    }
    else
        lstrcat(szCaption, " - (Untitled)");

    SetWindowText(hAppWnd, szCaption);
}

VOID PUBFUNC NewSess(VOID)
{
    memset(&WrkSess, 0, sizeof(WrkSess));
    lstrcpy(WrkSess.szProgId, "Kermit-MW v0.60");
    lstrcpy(WrkSess.szModNames[CONNMOD], "async");
    lstrcpy(WrkSess.szModNames[EMULMOD], "kermdec");
    lstrcpy(WrkSess.szModNames[PROTMOD], "kermit");

    bSuppressConnChg = FALSE;
    if (!GoDialogBox(hAppInst, "SessDlgBox", hAppWnd, (FARPROC)SessDlgProc))
        return;

//    if (!CloseSess())
//        return;

    memset(szSessFileName, 0, sizeof(szSessFileName));
    memcpy(&CurSess, &WrkSess, sizeof(CurSess));

    SetCaption();
    bChanged = FALSE;

    CMLoad(CurSess.szModNames[CONNMOD], TRUE);
    TMLoad("", TRUE);
    EMLoad(CurSess.szModNames[EMULMOD], TRUE);
    PMLoad(CurSess.szModNames[PROTMOD], TRUE);
    DMLoad(CurSess.szModNames[DEVMOD], TRUE);

    return;
}

BOOL PRVFUNC ReadSlot (int nFileHandle, UINT *pwId, UINT *pwSize)
{
    SLOT Slot;

    if (_lread(nFileHandle, (VOID *)&Slot, sizeof(Slot)) != sizeof(Slot))
        return(FALSE);

    *pwId = Slot.wId;
    *pwSize = Slot.wLength;

    return(Slot.wId > 0);
}

VOID PRVFUNC ReadSlotData (int nFileHandle, VOID * pData,
           UINT wSlotSize, UINT wDataSize)
{
    int nRead;

    if (wSlotSize > wDataSize) {
        nRead = _lread(nFileHandle, pData, wDataSize);
        _llseek(nFileHandle, (long)(wSlotSize - wDataSize), SEEK_CUR);
    }
    else {
        nRead = _lread(nFileHandle, pData, wSlotSize);
        memset((BYTE *)pData + wSlotSize, '\0', wDataSize - wSlotSize);
    }
}

BOOL PRVFUNC ReadSess(int FileHandle)
{
    int      nRead;
    UINT     wId, wSize;
    char     ConfigData[512];

    nRead = _lread(FileHandle, (VOID *)&CurSess, sizeof(CurSess));

    if (CurSess.szModNames[CONNMOD][0] == '\0')
        lstrcpy(CurSess.szModNames[CONNMOD], "async");

    if (CurSess.szModNames[EMULMOD][0] == '\0')
        lstrcpy(CurSess.szModNames[EMULMOD], "kermdec");

    if (CurSess.szModNames[PROTMOD][0] == '\0')
        lstrcpy(CurSess.szModNames[PROTMOD], "kermit");

    CMLoad(CurSess.szModNames[CONNMOD], TRUE);
    DMLoad(CurSess.szModNames[DEVMOD], TRUE);
    TMLoad("", TRUE);
    EMLoad(CurSess.szModNames[EMULMOD], TRUE);
    PMLoad(CurSess.szModNames[PROTMOD], TRUE);

    while (ReadSlot(FileHandle, &wId, &wSize))
        switch (wId) {
            case CONNSLOT:
                ReadSlotData(FileHandle, ConfigData, wSize, sizeof(ConfigData));
                CMSetConfig(wSize, ConfigData);
                break;

            case TERMSLOT:
                ReadSlotData(FileHandle, ConfigData, wSize, sizeof(ConfigData));
                TMSetConfig(wSize, ConfigData);
                break;

            case EMULSLOT:
                ReadSlotData(FileHandle, ConfigData, wSize, sizeof(ConfigData));
                EMSetConfig(wSize, ConfigData);
                break;

            case PROTSLOT:
                ReadSlotData(FileHandle, ConfigData, wSize, sizeof(ConfigData));
                PMSetConfig(wSize, ConfigData);
                break;

            case DEVSLOT:
                ReadSlotData(FileHandle, ConfigData, wSize, sizeof(ConfigData));
                DMSetConfig(wSize, ConfigData);
                break;

            case LOGSLOT:
                ReadSlotData(FileHandle, &LogSet, wSize, sizeof(LogSet));
                break;

            default:
                ReadSlotData(FileHandle, NULL, wSize, 0);
                break;
        }

        if (!IsZoomed(hAppWnd) && CurSess.nWidth > 0 && CurSess.nHeight > 0)
            MoveWindow(hAppWnd, (int)CurSess.nLeft, (int)CurSess.nTop,
                                (int)CurSess.nWidth, (int)CurSess.nHeight, TRUE);

    return(TRUE);
}

BOOL PUBFUNC OpenSess(LPSTR lpszFileName)
{
    int      FileHandle;
    OFSTRUCT ReopenBuff;

    if (lpszFileName != NULL && lpszFileName[0] != '\0') {
        if (OpenFile(lpszFileName, &ReopenBuff, OF_PARSE) == -1) {
            MessageBeep(0);
            KermitFmtMsgBox(MB_ICONEXCLAMATION | MB_OK,
                            "Invalid filename '%s'", (LPSTR)lpszFileName);
            return(FALSE);
        }

        lstrcpy(szWorkFileName, (PSTR)ReopenBuff.szPathName);
    }
    else {
    OPENFILENAME ofn;

    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hAppWnd;
    ofn.lpstrFilter = "Kermit Files(*.KRM)\0*.krm\0All Files(*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szWorkFileName;
    ofn.nMaxFile = sizeof(szWorkFileName);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "KRM";

    if (!GetOpenFileName(&ofn))
        return(FALSE);
    }

    if (!strchr(strrchr(szWorkFileName, '\\') + 1, '.'))
        lstrcat(szWorkFileName, ".KRM");

    FileHandle = ErrOpenFile(szWorkFileName, &ReopenBuff, OF_READ);
    if (FileHandle == -1) {
        MessageBeep(0);
        KermitFmtMsgBox(MB_ICONEXCLAMATION | MB_OK,
                        "Can't open '%s'", (LPSTR)szWorkFileName);
        return(FALSE);
    }

//    if (!CloseSess())
//        return(FALSE);

    lstrcpy(szSessFileName, szWorkFileName);

    memset(&LogSet, 0, sizeof(LogSet));

    bChanged = FALSE;
    SetCaption();

    if (!ReadSess(FileHandle))
        return(FALSE);

    _lclose(FileHandle);

    if (CurSess.bConnect)
        DMConnect();

    return TRUE;
}

VOID PUBFUNC SessOnConnect(VOID)
{
#ifdef AXSCRIPT
    if (CurSess.szScript[0] != '\0')
        AXScriptLoadFile(CurSess.szScript);
#endif
}

BOOL PRVFUNC WriteSlot(int nFileHandle, UINT wId, UINT wSize, VOID *pData)
{
    SLOT     Slot;

    Slot.wId = (WORD)wId;
    Slot.wLength = (WORD)wSize;
    if (_lwrite(nFileHandle, (PSTR)(VOID *)&Slot, sizeof(Slot)) != sizeof(Slot))
        return(FALSE);

    if (_lwrite(nFileHandle, (PSTR)pData, wSize) != (UINT)wSize)
    return(FALSE);

    return(TRUE);
}

BOOL PUBFUNC SaveSess(BOOL bGetFileName)
{
    int      FileHandle;
    OFSTRUCT ReopenBuff;
    char     ConfigData[512];
    UINT     wSize;
    RECT     rc;

    if (bGetFileName || *szSessFileName == '\0') {
    OPENFILENAME ofn;

    lstrcpy(szWorkFileName, szSessFileName);

    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hAppWnd;
    ofn.lpstrFilter = "Kermit Files(*.KRM)\0*.krm\0All Files(*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szWorkFileName;
    ofn.nMaxFile = sizeof(szWorkFileName);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "KRM";

    if (!GetSaveFileName(&ofn))
        return(FALSE);

    lstrcpy(szSessFileName, szWorkFileName);
        SetCaption();
    }

    FileHandle = OpenFile(szSessFileName, &ReopenBuff, OF_CREATE | OF_WRITE);
    if (FileHandle == -1)
        return(FALSE);

    if (!IsZoomed(hAppWnd)) {
        GetWindowRect(hAppWnd, &rc);
        CurSess.nLeft = rc.left;
        CurSess.nTop = rc.top;
        CurSess.nWidth = rc.right - rc.left;
        CurSess.nHeight = rc.bottom - rc.top;
    }

    /* write the file header */
    _lwrite(FileHandle, (PSTR)(VOID *)&CurSess, sizeof(CurSess));

    /* write the slots */
    CMGetConfig(&wSize, ConfigData);
    WriteSlot(FileHandle, CONNSLOT, wSize, ConfigData);
    TMGetConfig(&wSize, ConfigData);
    WriteSlot(FileHandle, TERMSLOT, wSize, ConfigData);
    EMGetConfig(&wSize, ConfigData);
    WriteSlot(FileHandle, EMULSLOT, wSize, ConfigData);
    PMGetConfig(&wSize, ConfigData);
    WriteSlot(FileHandle, PROTSLOT, wSize, ConfigData);
    DMGetConfig(&wSize, ConfigData);
    WriteSlot(FileHandle, DEVSLOT, wSize, ConfigData);
    WriteSlot(FileHandle, LOGSLOT, sizeof(LogSet), &LogSet);

    _lclose(FileHandle);

    bChanged = FALSE;

    return(TRUE);
}

BOOL PUBFUNC ConfirmClose(VOID)
{
    int nResult;
    char szMsg[80];

    szMsg[0] = '\0';

    if (bConnected)
        lstrcpy(szMsg, "Terminate connection?");

    if (bKermit)
        lstrcat(szMsg, "\n\nFile transfer will be aborted!");

    if (szMsg[0] != '\0')
        if (KermitFmtMsgBox(MB_OKCANCEL | MB_ICONQUESTION, szMsg) != IDOK)
            return FALSE;

    if (bChanged) {
        nResult = KermitFmtMsgBox(MB_ICONQUESTION | MB_YESNOCANCEL,
                                  "Save Session Settings?");

        switch (nResult) {
            case IDYES:
                if (SaveSess(FALSE))
                    break;

            case IDCANCEL:
                return FALSE;
        }
    }

    return TRUE;
}

BOOL PUBFUNC CloseSess(void)
{
    TermReview(FALSE);

    if (bKermit && !AbortXfer(FALSE))
        return FALSE;

    if (bConnected && !DMDisconnect(CD_FORCE))
        return FALSE;

/*  DMUnload();                                   */
/*  PMUnload();                                   */
/*  TMUnload();                                   */
    EMUnload();
/*  CMUnload();                                   */

    memset(szSessFileName, 0, sizeof(szSessFileName));
    SetCaption();

    return(TRUE);
}

VOID PRVFUNC ListModules(HWND hDlg, int nCtl, PSTR pszFileExt, PSTR pszModName)
{
    int     nItemCnt, nItem;
    HWND    hWndCtl;
    PATHNAM szFileSpec;

    GetModuleFileName(hAppInst, szFileSpec, sizeof(szFileSpec));
    *(strrchr(szFileSpec, '\\') + 1) = '\0';
    lstrcat(szFileSpec, pszFileExt);
    ErrDlgDirListComboBox(hDlg, szFileSpec, nCtl, NULL, 0);

    hWndCtl = GetDlgItem(hDlg, nCtl);

    nItemCnt = ComboBox_GetCount(hWndCtl);

    for (nItem = 0; nItem < nItemCnt; nItem++) {
        ComboBox_GetLBText(hWndCtl, nItem, szFileSpec);
        *strrchr(szFileSpec, '.') = '\0';
        ComboBox_DeleteString(hWndCtl, nItem);
        ComboBox_InsertString(hWndCtl, nItem, szFileSpec);
    }

    if (pszModName != NULL)
        ComboBox_SelectString(hWndCtl, -1, pszModName);

//    if (ComboBox_GetCurSel(hWndCtl) == LB_ERR)
//        ComboBox_SetCurSel(hWndCtl, 0);
}

BOOL PRVFUNC SessDlgInit(HWND hDlg)
{
    SetDlgItemText(hDlg, IDD_NAME, WrkSess.szDesc);
    SetDlgItemText(hDlg, IDD_FILE, WrkSess.szScript);

    ComboBox_AddString(GetDlgItem(hDlg, IDD_CONNLIST), "async");
    ComboBox_AddString(GetDlgItem(hDlg, IDD_CONNLIST), "tcpip");
    ComboBox_SelectString(GetDlgItem(hDlg, IDD_CONNLIST), -1, WrkSess.szModNames[CONNMOD]);

    ListModules(hDlg, IDD_EMULLIST, "*.TRM", WrkSess.szModNames[EMULMOD]);
/*    ListModules(hDlg, IDD_PROTLIST, "*.PRT", WrkSess.szModNames[PROTMOD]); */
/*    ListModules(hDlg, IDD_DEVLIST,  "*.DEV", WrkSess.szModNames[DEVMOD]);  */
/*    ListModules(hDlg, IDD_COMMLIST, "*.COM", WrkSess.szModNames[CONNMOD]); */

    ComboBox_AddString(GetDlgItem(hDlg, IDD_PROTLIST), "kermit");
    ComboBox_SelectString(GetDlgItem(hDlg, IDD_PROTLIST), -1, WrkSess.szModNames[PROTMOD]);

    CheckDlgButton(hDlg, IDD_CONNECT, WrkSess.bConnect);

    if (bSuppressConnChg)
        EnableWindow(GetDlgItem(hDlg, IDD_CONNLIST), FALSE);

    return(TRUE);
}

BOOL PRVFUNC GetModule(HWND hDlg, int nCtl, PSTR pszModName)
{
    ComboBox_GetText(GetDlgItem(hDlg, nCtl), pszModName, sizeof(WrkSess.szModNames[EMULMOD]));

    return(TRUE);
}

BOOL PRVFUNC SessDlgTerm(HWND hDlg)
{
    GetDlgItemText(hDlg, IDD_NAME, WrkSess.szDesc, sizeof(WrkSess.szDesc));
    GetDlgItemText(hDlg, IDD_FILE, WrkSess.szScript, sizeof(WrkSess.szScript));

    ComboBox_GetText(GetDlgItem(hDlg, IDD_CONNLIST), WrkSess.szModNames[CONNMOD],
                     sizeof(WrkSess.szModNames[CONNMOD]));
    ComboBox_GetText(GetDlgItem(hDlg, IDD_EMULLIST), WrkSess.szModNames[EMULMOD],
                     sizeof(WrkSess.szModNames[EMULMOD]));
    ComboBox_GetText(GetDlgItem(hDlg, IDD_PROTLIST), WrkSess.szModNames[PROTMOD],
                     sizeof(WrkSess.szModNames[PROTMOD]));

    if (!GetModule(hDlg, IDD_EMULLIST, WrkSess.szModNames[EMULMOD]))
        return FALSE;

/*    if (!GetModule(hDlg, IDD_PROTLIST, WrkSess.szModNames[PROTMOD])) */
/*        return(FALSE);                           */

/*    if (!GetModule(hDlg, IDD_DEVLIST, WrkSess.szModNames[DEVMOD]))   */
/*        return(FALSE);                           */

/*    if (!GetModule(hDlg, IDD_COMMLIST, WrkSess.szModNames[CONNMOD])) */
/*        return(FALSE);                           */

/*    SetMessage();                          */

    WrkSess.bConnect = (WORD)IsDlgButtonChecked(hDlg, IDD_CONNECT);

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export SessDlgProc(HWND hDlg, UINT message,
                  WPARAM wParam, LPARAM X(lParam))

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDD_OK:
                    if (SessDlgTerm(hDlg))
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
            return(SessDlgInit(hDlg));

        default:
            return(FALSE);
    }
    return(TRUE);
}

BOOL PUBFUNC SMInit(VOID)
{
    memset(&WrkSess, 0, sizeof(WrkSess));
    lstrcpy(WrkSess.szProgId, "Kermit-MW v0.60");
    lstrcpy(WrkSess.szModNames[CONNMOD], "async");
    lstrcpy(WrkSess.szModNames[EMULMOD], "kermdec");
    lstrcpy(WrkSess.szModNames[PROTMOD], "kermit");

    memset(szSessFileName, 0, sizeof(szSessFileName));
    memcpy(&CurSess, &WrkSess, sizeof(CurSess));

    SetCaption();

    bChanged = FALSE;

    CMLoad(CurSess.szModNames[CONNMOD], TRUE);
    TMLoad("", TRUE);
    EMLoad(CurSess.szModNames[EMULMOD], TRUE);
    PMLoad(CurSess.szModNames[PROTMOD], TRUE);
    DMLoad(CurSess.szModNames[DEVMOD], TRUE);

    return(TRUE);
}

BOOL PUBFUNC SMSetup(HINSTANCE hInst, HWND hWnd)
{
    memcpy(&WrkSess, &CurSess, sizeof(WrkSess));

    bSuppressConnChg = bConnected;
    if (!GoDialogBox(hInst, "SessDlgBox", hWnd, (FARPROC)SessDlgProc))
        return(TRUE);

    memcpy(&CurSess, &WrkSess, sizeof(WrkSess));

    CMLoad(CurSess.szModNames[CONNMOD], FALSE);
    TMLoad("", FALSE);
    EMLoad(CurSess.szModNames[EMULMOD], FALSE);
    PMLoad(CurSess.szModNames[PROTMOD], FALSE);
    DMLoad(CurSess.szModNames[DEVMOD], FALSE);

    bChanged = TRUE;

    return(TRUE);
}