/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMANSI.C                                   **
**                                                                            **
**                      Version 0.50 - Developmental                          **
**                     Requires Microsoft Windows 2.X                         **
**                         Author: Wayne Warthen                              **
**                                                                            **
**  Kermit Terminal Emulation Module -- ANSI                                  **
**                                                                            **
*******************************************************************************/

/* DEFINITIONS ---------------------------------------------------------------*/

#define GLOBAL

#define ANSI_BLACK   0
#define ANSI_RED     1
#define ANSI_GREEN   2
#define ANSI_YELLOW  3
#define ANSI_BLUE    4
#define ANSI_MAGENTA 5
#define ANSI_CYAN    6
#define ANSI_WHITE   7

#define DEFATTR     ((BYTE)(ATTR_FGWHITE | ATTR_BGBLACK))

#define ANSIKEY_UP      1
#define ANSIKEY_DOWN    2
#define ANSIKEY_LEFT    3
#define ANSIKEY_RIGHT   4

/* INCLUDES ------------------------------------------------------------------*/

#include "kermansi.h"
#include <string.h>
#include <stdlib.h>

/* TYPEDEFS ------------------------------------------------------------------*/

typedef struct {
    char * szKeyName;
    UINT   nKeyID;
} ANSIKEYMAP;

/* LOCAL STORAGE -------------------------------------------------------------*/

static RECT Rect;

static BYTE FgAttrMap[] = {ATTR_FGBLACK, ATTR_FGRED, ATTR_FGGREEN, ATTR_FGYELLOW,
                           ATTR_FGBLUE, ATTR_FGMAGENTA, ATTR_FGCYAN, ATTR_FGWHITE};

static BYTE BgAttrMap[] = {ATTR_BGBLACK, ATTR_BGRED, ATTR_BGGREEN, ATTR_BGYELLOW,
                           ATTR_BGBLUE, ATTR_BGMAGENTA, ATTR_BGCYAN, ATTR_BGWHITE};

static KEYMAP km[] = {
                      {FALSE, FALSE, FALSE, VK_UP, ANSIKEY_UP},
                      {FALSE, FALSE, FALSE, VK_DOWN, ANSIKEY_DOWN},
                      {FALSE, FALSE, FALSE, VK_LEFT, ANSIKEY_LEFT},
                      {FALSE, FALSE, FALSE, VK_RIGHT, ANSIKEY_RIGHT},
                      {0, 0, 0, 0, NULL}};

static ANSIKEYMAP akm[] = {
    {"UP",      ANSIKEY_UP},
    {"DOWN",    ANSIKEY_DOWN},
    {"RIGHT",   ANSIKEY_RIGHT},
    {"LEFT",    ANSIKEY_LEFT},
    {NULL, 0}
};

/* LOCAL PROTOTYPES ----------------------------------------------------------*/

    VOID WriteTermCharEsc(char);
    VOID WriteTermCharEscSeq(char);
    VOID WriteTermCharCtl(char);
    VOID WriteTermCharRaw(char);

#ifdef _WIN32

/*----------------------------------------------------------------------------*/
extern "C" int APIENTRY DllMain
/*-----------------------------------------------------------------------------
**
**  Library initialization procedure called when library is loaded.
*/
(HINSTANCE     hInst,         /* Instance handle for library.                 */
 DWORD         X(ul_reason),  /* Handle to the local data segment.            */
 LPVOID        X(lpReserved)) /* Size of local heap.                          */
/*----------------------------------------------------------------------------*/

{
    hLibrary = hInst;
    return 1;
}

#else

/*----------------------------------------------------------------------------*/
int CALLBACK LibMain
/*-----------------------------------------------------------------------------
**
**  Library initialization procedure called when library is loaded.
*/
(HINSTANCE    hInstance,      /* Instance handle for library.                 */
 WORD         X(wDataSeg),    /* Handle to the local data segment.            */
 WORD         cbHeapSize,     /* Size of local heap.                          */
 LPSTR        X(lpszCmdLine)) /* Pointer to command line.                     */
/*----------------------------------------------------------------------------*/

{
    hLibrary = hInstance;

    if (cbHeapSize != 0)
        UnlockData(0);

    return(TRUE);
}
#endif

/*----------------------------------------------------------------------------*/
VOID WriteTermCharEsc
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Handle a character being sent to the terminal.  This function handles     */
/*  control sequences as well as characters to be written.                    */
/*                                                                            */
(char TermChar)     /* Character for terminal.                                */
/*----------------------------------------------------------------------------*/

{
    int i;

    switch (TermChar) {
        case '[':
            npTCB->bEscSeq = TRUE;
            npTCB->nParmCnt = 0;
            memset(npTCB->szEscSeq, '\0', sizeof(npTCB->szEscSeq));
            for (i = 0; i < MAXPARM; i++)
                npTCB->nParm[i] = 0;
            break;
    }

    npTCB->bEscape = FALSE;
}

/*----------------------------------------------------------------------------*/
VOID WriteTermCharEscSeq
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Handle a character being sent to the terminal.  This function handles     */
/*  control sequences as well as characters to be written.                    */
/*                                                                            */
(char TermChar)     /* Character for terminal.                                */
/*----------------------------------------------------------------------------*/

{
    if ((TermChar >= '0' && TermChar <= '9')) {
        npTCB->szEscSeq[strlen(npTCB->szEscSeq)] = TermChar;
        return;
    }

    npTCB->nParm[npTCB->nParmCnt++] = atoi(npTCB->szEscSeq);
    memset(npTCB->szEscSeq, '\0', sizeof(npTCB->szEscSeq));

    if (TermChar == ';')    // parm separator, keep going
        return;

    switch (TermChar) {
        case 'H':           // CURSOR POSITION SET
        case 'f':
            npTCB->nCurRow = min(LASTROW, max(0, npTCB->nParm[0] - 1));
            npTCB->nCurCol = min(LASTCOL, max(0, npTCB->nParm[1] - 1));
            break;

        case 'A':           // CURSOR MOVE UP
            npTCB->nCurRow = max(0, npTCB->nCurRow - max(1, npTCB->nParm[0]));
            break;

        case 'B':           // CURSOR MOVE DOWN
            npTCB->nCurRow = min(LASTROW, npTCB->nCurRow + max(1, npTCB->nParm[0]));
            break;

        case 'C':           // CURSOR MOVE RIGHT
            npTCB->nCurCol = min(LASTCOL, npTCB->nCurCol + max(1, npTCB->nParm[0]));
            break;

        case 'D':           // CURSOR MOVE LEFT
            npTCB->nCurCol = max(0, npTCB->nCurCol - max(1, npTCB->nParm[0]));
            break;

        case 'J':           // ERASE ??? SCREEN
            switch (npTCB->nParm[0]) {
                case 0:     // ... TO END OF ...
                    if (npTCB->nCurRow < LASTROW) {
                        SetRect(&Rect, 0, npTCB->nCurRow + 1,
                                       LASTCOL, LASTROW);
                        FillScrn(&Rect, ' ');
                        FillAttr(&Rect, DEFATTR);
                    }
                    SetRect(&Rect, npTCB->nCurCol, npTCB->nCurRow,
                                   LASTCOL, npTCB->nCurRow);
                    FillScrn(&Rect, ' ');
                    FillAttr(&Rect, DEFATTR);
                    break;

                case 1:     // ... START OF ...
                    if (npTCB->nCurRow > 0) {
                        SetRect(&Rect, 0, 0, LASTCOL, npTCB->nCurRow - 1);
                        FillScrn(&Rect, ' ');
                        FillAttr(&Rect, DEFATTR);
                    }
                    SetRect(&Rect, 0, npTCB->nCurRow, npTCB->nCurCol, npTCB->nCurRow);
                    FillScrn(&Rect, ' ');
                    FillAttr(&Rect, DEFATTR);
                    break;

                case 2:     // ... ENTIRE ...
                    ScrollScrn(0, TERMROWS, NULL);
                    FillScrn(NULL, ' ');
                    FillAttr(NULL, DEFATTR);
                    npTCB->nCurRow = 0;
                    npTCB->nCurCol = 0;
                    break;
            }
            break;

        case 'K':           // ERASE ??? LINE
            switch (npTCB->nParm[0]) {
                case 0:     // ... TO END OF ...
                    SetRect(&Rect, npTCB->nCurCol, npTCB->nCurRow,
                                   LASTCOL, npTCB->nCurRow);
                    FillScrn(&Rect, ' ');
                    FillAttr(&Rect, DEFATTR);
                    break;

                case 1:     // ... FROM START OF ...
                    SetRect(&Rect, 0, npTCB->nCurRow,
                                   npTCB->nCurCol, npTCB->nCurRow);
                    FillScrn(&Rect, ' ');
                    FillAttr(&Rect, DEFATTR);
                    break;

                case 2:     // ... ENTIRE ...
                    SetRect(&Rect, 0, npTCB->nCurRow,
                                   LASTCOL, npTCB->nCurRow);
                    FillScrn(&Rect, ' ');
                    FillAttr(&Rect, DEFATTR);
                    break;

            }
            break;

        case 'n':           // READ ???
            switch (npTCB->nParm[0]) {
                static char szWorkBuf [16];

                case 6:     // CURSOR POSITION
                    wsprintf(szWorkBuf, "\x1B[%d;%dR",
                             npTCB->nCurRow + 1, npTCB->nCurCol + 1);
                    SendTermStr(szWorkBuf, strlen(szWorkBuf));
                    break;
            }
            break;

        case 's':           // Save Cursor Position
            npTCB->nSavRow = npTCB->nCurRow;
            npTCB->nSavCol = npTCB->nCurCol;
            break;

        case 'u':           // Restore Cursor Position
            npTCB->nCurRow = npTCB->nSavRow;
            npTCB->nCurCol = npTCB->nSavCol;
            break;

        case 'm':
            {
                int i;
                BYTE cAttr;

                for (i = 0; i < npTCB->nParmCnt; i++) {
                    if (npTCB->nParm[i] == 0) { /* Normal */
                        npTCB->fg = ANSI_WHITE;  /* white */
                        npTCB->bg = ANSI_BLACK;  /* on black */
                        npTCB->ul = FALSE;
                        npTCB->bold = FALSE;
                        npTCB->inv = FALSE;
                    }
                    else if (npTCB->nParm[i] == 1) /* Bold (simulate with yellow) */
                        npTCB->fg = ANSI_YELLOW;
                    else if (npTCB->nParm[i] == 4) /* Underline (set underline flag) */
                        npTCB->ul = TRUE;
                    else if (npTCB->nParm[i] == 5) /* Blink (simulate with red) */
                        npTCB->fg = ANSI_RED;
                    else if (npTCB->nParm[i] == 7) /* Inverse (set inverted flag) */
                        npTCB->inv = TRUE;
                    else if (npTCB->nParm[i] == 8) { /* Invisible (black on black) */
                        npTCB->fg = ANSI_BLACK;
                        npTCB->bg = ANSI_BLACK;
                    }
                    else if (npTCB->nParm[i] >= 30 && npTCB->nParm[i] <= 37)
                        npTCB->fg = (BYTE)(npTCB->nParm[i] - 30);
                    else if (npTCB->nParm[i] >= 40 && npTCB->nParm[i] <= 47)
                        npTCB->bg = (BYTE)(npTCB->nParm[i] - 40);
                }

                cAttr = 0;
                if (npTCB->inv) {
                    cAttr |= FgAttrMap[npTCB->bg];
                    cAttr |= BgAttrMap[npTCB->fg];
                }
                else {
                    cAttr |= FgAttrMap[npTCB->fg];
                    cAttr |= BgAttrMap[npTCB->bg];
                }

                if (npTCB->bold)
                    cAttr |= ATTR_BOLD;

                if (npTCB->ul)
                    cAttr |= ATTR_UNDERLINE;

                SetAttr(cAttr);
            }
            break;
    }

    npTCB->bEscSeq = FALSE;
}

/*----------------------------------------------------------------------------*/
VOID WriteTermCharCtl
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Handle a character being sent to the terminal.  This function handles     */
/*  control sequences as well as characters to be written.                    */
/*                                                                            */
(char TermChar)     /* Character for terminal.                                */
/*----------------------------------------------------------------------------*/

{
    switch(TermChar) {
        case 7:                     /* bell */
            MessageBeep(0);
            break;

        case 8:                     /* backspace */
            if (npTCB->nCurCol > 0)
                (npTCB->nCurCol)--;
            break;

        case 9:                     /* tab */
            npTCB->nCurCol = min(LASTCOL,
                                 npTCB->nCurCol + 8 - (npTCB->nCurCol % 8));
            break;

        case 10:                    /* line feed */
            if (npTCB->nCurRow + 1 < TERMROWS)
                (npTCB->nCurRow)++;
            else
                ScrollScrn(0, 1, NULL);
            break;

        case 12:                    /* form feed */
            ScrollScrn(0, TERMROWS, NULL);
            FillScrn(NULL, ' ');
            FillAttr(NULL, DEFATTR);
            npTCB->nCurRow = 0;
            npTCB->nCurCol = 0;
            break;

        case 13:                    /* carriage return */
            npTCB->nCurCol = 0;
            break;

        case 27:                    /* escape */
            npTCB->bEscape = TRUE;
            break;

        default:                    /* all other characters (eat them up!!!) */
//            WriteTermCharRaw(TermChar);
            break;
    }
}

/*----------------------------------------------------------------------------*/
VOID WriteTermCharRaw
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Handle a character being sent to the terminal.  This function handles     */
/*  control sequences as well as characters to be written.                    */
/*                                                                            */
(char TermChar)     /* Character for terminal.                                */
/*----------------------------------------------------------------------------*/

{
    if (npTCB->nCurCol >= TERMCOLS) {
        if (npTCB->TermSet.LineWrap) {
            WriteTermCharCtl('\r');
            WriteTermCharCtl('\n');
        }
        else
            npTCB->nCurCol = LASTCOL;
    }

    WriteScrnChar((npTCB->nCurCol)++, npTCB->nCurRow, TermChar);

    if (!npTCB->TermSet.LineWrap && (npTCB->nCurCol >= TERMCOLS))
        npTCB->nCurCol = LASTCOL;
}

/*----------------------------------------------------------------------------*/
VOID WriteTermChar
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Handle a character being sent to the terminal.  This function handles     */
/*  control sequences as well as characters to be written.                    */
/*                                                                            */
(char TermChar,     /* Character for terminal.                                */
 BOOL Raw)          /* Raw mode flag (suppress formatting when TRUE).         */
/*----------------------------------------------------------------------------*/

{
    if (TermChar >= '\0' && TermChar < ' ') {
        if (Raw)
            WriteTermCharRaw(TermChar);
        else
            WriteTermCharCtl(TermChar);
    }
    else if (npTCB->bEscape)
        WriteTermCharEsc(TermChar);
    else if (npTCB->bEscSeq)
        WriteTermCharEscSeq(TermChar);
    else
        WriteTermCharRaw(TermChar);
}

/*----------------------------------------------------------------------------*/
extern "C" VOID DLLEXPORT CALLBACK WriteTerm
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Process string of characters for the terminal.                            */
/*                                                                            */
(HANDLE  hEmul,     /* Handle to callers terminal window.                     */
 LPSTR   TermStr,   /* Character string to be written.                        */
 int     StrLen,    /* Number of chars to write (string may contain nulls).   */
 BOOL    Raw)       /* Raw mode flag (suppress formatting when TRUE).         */
/*----------------------------------------------------------------------------*/

{
    int i;

    npTCB = (TCB *)LocalLock(hEmul);

    for (i = 0; i < StrLen; i++)
        WriteTermChar(TermStr[i], Raw);

    FlushScrnBuf();
    SetCurPos(npTCB->nCurCol, npTCB->nCurRow, FALSE);

    LocalUnlock(hEmul);
}

/*----------------------------------------------------------------------------*/
extern "C" VOID DLLEXPORT CALLBACK ProcessTermChar
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Process a character entered at the terminal console.  Function sends      */
/*  the character down the comm line, then echoes the character to the        */
/*  terminal is echoing is enabled.                                           */
/*                                                                            */
(HANDLE hEmul,   /* Handle to callers terminal window.                        */
 char   cChar)   /* Character entered.                                        */
/*----------------------------------------------------------------------------*/

{
    npTCB = (TCB *)LocalLock(hEmul);

    SendTermChar(cChar);
    if (npTCB->TermSet.LocalEcho)
        WriteTerm(hEmul, &cChar, 1, FALSE);

    if (cChar == '\r' && npTCB->TermSet.NewLine) {
        cChar = '\n';
        SendTermChar(cChar);
        if (npTCB->TermSet.LocalEcho)
            WriteTerm(hEmul, &cChar, 1, FALSE);
    }

    LocalUnlock(hEmul);
}

/*----------------------------------------------------------------------------*/
extern "C" VOID DLLEXPORT CALLBACK ProcessTermKey
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Process a key entered at the terminal console.  Function handles          */
/*  function keys, etc.  Echoes if appropriate.                               */
/*                                                                            */
(HANDLE hEmul,  /* Handle to callers terminal window.                         */
 UINT   cKey)   /* Key pressed.                                               */
/*----------------------------------------------------------------------------*/

{
    static char szWorkStr[16];

    npTCB = (TCB *)LocalLock(hEmul);

    switch (cKey) {
        case ANSIKEY_RIGHT:
            lstrcpy(szWorkStr, " [C");
            break;

        case ANSIKEY_UP:
            lstrcpy(szWorkStr, " [A");
            break;

        case ANSIKEY_LEFT:
            lstrcpy(szWorkStr, " [D");
            break;

        case ANSIKEY_DOWN:
            lstrcpy(szWorkStr, " [B");
            break;

        default:
            LocalUnlock(hEmul);
            return;
    }

    szWorkStr[0] = 27;

    SendTermStr(szWorkStr, strlen(szWorkStr));
    if (npTCB->TermSet.LocalEcho)
        WriteTerm(hEmul, szWorkStr, strlen(szWorkStr), FALSE);

    LocalUnlock(hEmul);
}

extern "C" BOOL DLLEXPORT CALLBACK DoKeyMap(HANDLE X(hEmul), KEYMAPENTRY FAR * lpkme)
{
    int    i;
    KEYMAP km2;

    for (i = 0; akm[i].szKeyName != NULL; i++) {
        if (_fstricmp(akm[i].szKeyName, lpkme->szKeyName) == 0) {
            km2.bShift = lpkme->bShift;
            km2.bControl = lpkme->bShift;
            km2.bAlternate = lpkme->bAlternate;
            km2.vk = lpkme->vk;
            km2.tk = akm[i].nKeyID;
            KeyMapAdd(&km2);
            break;
        }
    }

    if (akm[i].szKeyName == NULL) {
        char szMessage[80];

        wsprintf(szMessage, "Unknown ANSI key neumonic %s!",
                 (LPSTR)lpkme->szKeyName);
        MessageBox(NULL, szMessage, "Kermit Key Parse Error",
                   MB_OK | MB_ICONEXCLAMATION);

        return FALSE;
    }

    return TRUE;
}

/*----------------------------------------------------------------------------*/
int NEAR GetRadioButton
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Scan a set of button controls looking for the first button that is        */
/*  selected.  Return the ID of the first selected button or 0 if none        */
/*  are selected.                                                             */
/*                                                                            */
(HWND hDlg,             /* Handle to dialog containing buttons.               */
 int nIDFirstButton,    /* Control ID of first button in set.                 */
 int nIDLastButton)     /* Control ID of last button in set.                  */
/*----------------------------------------------------------------------------*/

{
    int i;

    for (i = nIDFirstButton; i <= nIDLastButton; i++) {
        if (IsDlgButtonChecked(hDlg, i))
            return(i);
    }

    return(0);
}

/*----------------------------------------------------------------------------*/
BOOL DLLEXPORT CALLBACK AboutDlgProc
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Terminal setup dialog procedure.                                          */
/*                                                                            */
(HWND       hDlg,      /* Handle to termainal setup dialog                    */
 UINT       message,   /* Mesage number.                                      */
 WPARAM     wParam,    /* First paramter.                                     */
 LPARAM     X(lParam)) /* Second parameter.                                   */
/*----------------------------------------------------------------------------*/

{
    char szWork[512];

    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDD_OK:
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
            return(TRUE);

        default:
            return(FALSE);
    }

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL NEAR TermDlgInit
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Initialize the terminal setup dialog by prefilling controls.              */
/*                                                                            */
(HWND hDlg)     /* Handle to terminal setup dialog to initialize.             */
/*----------------------------------------------------------------------------*/

{
    HANDLE hEmul;

    hEmul = GetProp(hDlg, "hEmul");
    npTCB = (TCB *)LocalLock(hEmul);
    if (!npTCB)
        return(FALSE);

    CheckDlgButton(hDlg, IDD_NEWLINE, npTCB->TermSet.NewLine);
    CheckDlgButton(hDlg, IDD_LOCALECHO, npTCB->TermSet.LocalEcho);
    CheckDlgButton(hDlg, IDD_LINEWRAP, npTCB->TermSet.LineWrap);

    LocalUnlock(hEmul);
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL NEAR TermDlgTerm
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Terminate the terminal setup dialog by validating and saving the          */
/*  values in the controls.  Return TRUE only if all controls contain         */
/*  valid values.                                                             */
/*                                                                            */
(HWND hDlg)     /* Handle to terminal setup dialog to terminate.              */
/*----------------------------------------------------------------------------*/

{
    HANDLE hEmul;

    hEmul = GetProp(hDlg, "hEmul");
    npTCB = (TCB *)LocalLock(hEmul);
    if (!npTCB)
        return(FALSE);

    npTCB->TermSet.NewLine = (WORD)IsDlgButtonChecked(hDlg, IDD_NEWLINE);
    npTCB->TermSet.LocalEcho = (WORD)IsDlgButtonChecked(hDlg, IDD_LOCALECHO);
    npTCB->TermSet.LineWrap = (WORD)IsDlgButtonChecked(hDlg, IDD_LINEWRAP);

    LocalUnlock(hEmul);
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL DLLEXPORT CALLBACK TermDlgProc
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Terminal setup dialog procedure.                                          */
/*                                                                            */
(HWND       hDlg,      /* Handle to termainal setup dialog                    */
 UINT       message,   /* Mesage number.                                      */
 WPARAM     wParam,    /* First paramter.                                     */
 LPARAM     lParam)    /* Second parameter.                                   */
/*----------------------------------------------------------------------------*/

{
    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDD_OK:
                    if (!TermDlgTerm(hDlg))
                        break;

                case IDD_CANCEL:
                    RemoveProp(hDlg, "hEmul");
                    EndDialog(hDlg, TRUE);
                    break;

                case IDD_ABOUT:
                    DialogBoxParam(hLibrary, "AboutDlgBox", hDlg,
                                   AboutDlgProc, NULL);
                    break;

                default:
                    return(FALSE);
                    break;
            }
            break;

    case WM_INITDIALOG:
#ifdef _WIN32
        SetProp(hDlg, "hEmul", (HANDLE)lParam);
#else
        SetProp(hDlg, "hEmul", (HANDLE)LOWORD(lParam));
#endif
            return(TermDlgInit(hDlg));

        default:
            return(FALSE);
    }
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
extern "C" int DLLEXPORT CALLBACK SetupTerm
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Invokes the terminal setup dialog.                                        */
/*                                                                            */
(HANDLE hEmul)  /* Handle to callers terminal window.                         */
/*----------------------------------------------------------------------------*/

{
    int nResult;

    npTCB = (TCB *)LocalLock(hEmul);

#ifdef _WIN32
    nResult = DialogBoxParam(hLibrary, "TermDlgBox", npTCB->hAppWnd,
                 TermDlgProc, (LPARAM)hEmul);
#else
    nResult = DialogBoxParam(hLibrary, "TermDlgBox", npTCB->hAppWnd,
                 TermDlgProc, MAKELONG(hEmul,0));
#endif


    LocalUnlock(hEmul);

    return(nResult);
}

/*----------------------------------------------------------------------------*/
extern "C" BOOL DLLEXPORT CALLBACK GetTermConfig
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Retrieve the current terminal configuration and copy it to the            */
/*  requested memory area.  Return TRUE if successful.                        */
/*                                                                            */
(HANDLE hEmul,      /* Handle to callers terminal window.                     */
 LPSTR  lpConfig)   /* Pointer to memory area to fill with terminal config.   */
/*----------------------------------------------------------------------------*/

{
    if (lpConfig == NULL)
        return(FALSE);

    npTCB = (TCB *)LocalLock(hEmul);
    if (!npTCB)
        return(FALSE);

    _fmemcpy(lpConfig, &npTCB->TermSet, sizeof(TermBlk));

    LocalUnlock(hEmul);

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
extern "C" HANDLE DLLEXPORT CALLBACK OpenTerm
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Perform all terminal window intialization required.  Save the             */
/*  configuration information passed, save the callback proc addresses,       */
/*  and initialize local work variables.                                      */
/*                                                                            */
(HWND  hAppWnd,             /* Pointer to terminal config.                    */
 LPSTR lpConfig,            /* Pointer to terminal config.                    */
 FARPROC FAR * APIProcs)    /* Array of callback proc addresses.              */
/*----------------------------------------------------------------------------*/

{
    HANDLE hEmul;
    int    i;

    hEmul = LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(TCB));
    npTCB = (TCB *)LocalLock(hEmul);

    _fmemcpy(&npTCB->TermSet, &TermDef, sizeof(TermBlk));

    if (lpConfig != NULL)
        _fmemcpy(&npTCB->TermSet, lpConfig, sizeof(TermBlk));

    for (i = 0; i < APIPROCCNT; i++)
        npTCB->APIProcs[i] = APIProcs[i];

    npTCB->hAppWnd = hAppWnd;
    npTCB->nCurRow = 0; npTCB->nCurCol = 0;
    npTCB->nSavRow = 0; npTCB->nSavCol = 0;
    npTCB->fg = ANSI_WHITE;
    npTCB->bg = ANSI_BLACK;
    npTCB->bold = FALSE;
    npTCB->ul = FALSE;
    npTCB->inv = FALSE;
    npTCB->bEscape = FALSE;
    npTCB->bEscSeq = FALSE;

    LocalUnlock(hEmul);

    DrawEmul("ANSI (IBM-PC) Emulation");

    KeyMapClear();
    if (!KeyMapParse("KERMANSI.KEY"))
        for (i = 0; km[i].vk != 0; i++)
            KeyMapAdd(&(km[i]));

    return(hEmul);
}

/*----------------------------------------------------------------------------*/
extern "C" VOID DLLEXPORT CALLBACK CloseTerm
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Free all resources allocated to the callers terminal window in            */
/*  preparation for the windows destruction.                                  */
/*                                                                            */
(HANDLE hEmul)  /* Handle to callers terminal window.                         */
/*----------------------------------------------------------------------------*/

{
    LocalFree(hEmul);
}
