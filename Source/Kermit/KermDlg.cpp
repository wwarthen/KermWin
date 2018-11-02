/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                                KERMDLG.C                                   **
**                                                                            **
**  This module contains the common dialog procedures.  It also contains      **
**  miscellaneous helper routines.                                            **
**                                                                            **
*******************************************************************************/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include <windowsx.h>
#include "kermc3d.h"

/*----------------------------------------------------------------------------*/
int PUBFUNC GetRadioButton(HWND hDlg, int nIDFirstButton, int nIDLastButton)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int i;

    for (i = nIDFirstButton; i <= nIDLastButton; i++) {
        if (IsDlgButtonChecked(hDlg, i))
            return(i);
    }

    return(0);
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC GetValidDlgItemInt(HWND hDlg, UINT wItem, BOOL Signed,
                                UINT *Parm, UINT LoVal, UINT HiVal)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    UINT x;
    BOOL b;

    x = (UINT)GetDlgItemInt(hDlg, wItem, &b, Signed);
    if (b && (x >= LoVal && x <= HiVal)) {
        *Parm = x;
        return(TRUE);
    }
    else {
        SetFocus(GetDlgItem(hDlg, wItem));
        MessageBeep(0);
        return(FALSE);
    }
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export AboutDlgProc(HWND hDlg, UINT message,
                                    WPARAM wParam, LPARAM X(lParam))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    char szWork[512];

    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDD_OK:
                case IDD_CANCEL:
                    EndDialog(hDlg, TRUE);
                    break;

                default:
                    return(FALSE);
            }
            break;

        case WM_INITDIALOG:
            wsprintf(szWork, "%s\n\n%s\n%s", (LPSTR)szAppTitle,
                     (LPSTR)szVersion, (LPSTR)szBuild);
            SetDlgItemText(hDlg, IDD_VERSION, szWork);

            wsprintf(szWork, "%s\n\n%s", (LPSTR)szCopyright,
                     (LPSTR)szContact);
            SetDlgItemText(hDlg, IDD_MESSAGE, szWork);

            wsprintf(szWork, "OS Version %lu.%lu, build %lu\n"
                             "OLE Version %lu.%lu %s (%lu.%lu required)\n"
                             "%s\n"
                             "%s",
                OSVer.dwMajor, OSVer.dwMinor, OSVer.dwBuild,
                OLEVer.dwMajor, OLEVer.dwMinor,
                bOleInit ? (LPSTR)"loaded" : (LPSTR)"not loaded",
                OLEVer.dwMajorReq, OLEVer.dwMinorReq,
                (LPSTR)C3DStatus(),
#ifdef AXSCRIPT
                (LPSTR)AXStatus()
#else
                (LPSTR)"Scripting Unavailable"
#endif
                );
            SetDlgItemText(hDlg, IDD_INFO, szWork);

            return(TRUE);

        default:
            return(FALSE);
    }
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC LoggingDlgInit(HWND hDlg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    CheckDlgButton(hDlg, IDD_LOGSESFLG, LogSet.SessionLogFlag);
    SetDlgItemText(hDlg, IDD_LOGSESFIL, (LPCSTR)LogSet.SessionLogName);
    CheckDlgButton(hDlg, IDD_LOGPKTFLG, LogSet.PacketLogFlag);
    SetDlgItemText(hDlg, IDD_LOGPKTFIL, (LPCSTR)LogSet.PacketLogName);
    CheckDlgButton(hDlg, IDD_LOGTRNFLG, LogSet.TransLogFlag);
    SetDlgItemText(hDlg, IDD_LOGTRNFIL, (LPCSTR)LogSet.TransLogName);

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC LoggingDlgTerm(HWND hDlg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    LogBlk LogWrk;

    memcpy(&LogWrk, &LogSet, sizeof(LogBlk));

    LogWrk.SessionLogFlag = (WORD)IsDlgButtonChecked(hDlg, IDD_LOGSESFLG);
    LogWrk.PacketLogFlag = (WORD)IsDlgButtonChecked(hDlg, IDD_LOGPKTFLG);
    LogWrk.TransLogFlag = (WORD)IsDlgButtonChecked(hDlg, IDD_LOGTRNFLG);

    GetDlgItemText(hDlg, IDD_LOGSESFIL, LogWrk.SessionLogName,
                   sizeof(LogWrk.SessionLogName));
    GetDlgItemText(hDlg, IDD_LOGPKTFIL, LogWrk.PacketLogName,
                   sizeof(LogWrk.PacketLogName));
    GetDlgItemText(hDlg, IDD_LOGTRNFIL, LogWrk.TransLogName,
                   sizeof(LogWrk.TransLogName));

    memcpy(&LogSet, &LogWrk, sizeof(LogBlk));

    bChanged = TRUE;
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export LoggingDlgProc(HWND hDlg, UINT message,
                                     WPARAM wParam, LPARAM X(lParam))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDD_OK:
                    if (!LoggingDlgTerm(hDlg))
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
            return(LoggingDlgInit(hDlg));

        default:
            return(FALSE);
    }
    return(TRUE);
}
