/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMDEC.C                                    **
**                                                                            **
**                      Version 0.50 - Developmental                          **
**                     Requires Microsoft Windows 2.X                         **
**                         Author: Wayne Warthen                              **
**                                                                            **
**  Kermit Terminal Emulation Module -- DEC                                   **
**                                                                            **
*******************************************************************************/

/* TODO ------------------------------------------------------------------------

1. Implement vt320 mode???

2. Emulate DEC terminal LED's somehow

------------------------------------------------------------------------------*/


/* DEFINITIONS ---------------------------------------------------------------*/

#define GLOBAL

#define DEC_BLACK   0
#define DEC_RED     1
#define DEC_GREEN   2
#define DEC_YELLOW  3
#define DEC_BLUE    4
#define DEC_MAGENTA 5
#define DEC_CYAN    6
#define DEC_WHITE   7

#define DEFATTR     ((BYTE)(ATTR_FGWHITE | ATTR_BGBLACK))

#define DECKEY_UPARR    1
#define DECKEY_DNARR    2
#define DECKEY_RTARR    3
#define DECKEY_LFARR    4
#define DECKEY_PF1      5
#define DECKEY_PF2      6
#define DECKEY_PF3      7
#define DECKEY_PF4      8
#define DECKEY_KP0      9
#define DECKEY_KP1      10
#define DECKEY_KP2      11
#define DECKEY_KP3      12
#define DECKEY_KP4      13
#define DECKEY_KP5      14
#define DECKEY_KP6      15
#define DECKEY_KP7      16
#define DECKEY_KP8      17
#define DECKEY_KP9      18
#define DECKEY_KPCOMA   19
#define DECKEY_KPMINUS  20
#define DECKEY_KPDOT    21
#define DECKEY_KPENTER  22

/* INCLUDES ------------------------------------------------------------------*/

#include "kermdec.h"
#include <string.h>
#include <stdlib.h>

/* TYPEDEFS ------------------------------------------------------------------*/

typedef struct {
    char * szKeyName;
    UINT   nKeyID;
} DECKEYMAP;

/* LOCAL STORAGE -------------------------------------------------------------*/

static RECT Rect;

static BYTE FgAttrMap[] = {ATTR_FGBLACK, ATTR_FGRED, ATTR_FGGREEN, ATTR_FGYELLOW,
                           ATTR_FGBLUE, ATTR_FGMAGENTA, ATTR_FGCYAN, ATTR_FGWHITE};

static BYTE BgAttrMap[] = {ATTR_BGBLACK, ATTR_BGRED, ATTR_BGGREEN, ATTR_BGYELLOW,
                           ATTR_BGBLUE, ATTR_BGMAGENTA, ATTR_BGCYAN, ATTR_BGWHITE};

static DECKEYMAP dkm[] = {
    {"UPARR", DECKEY_UPARR},
    {"DNARR", DECKEY_DNARR},
    {"RTARR", DECKEY_RTARR},
    {"LFARR", DECKEY_LFARR},
    {"PF1", DECKEY_PF1},
    {"PF2", DECKEY_PF2},
    {"PF3", DECKEY_PF3},
    {"PF4", DECKEY_PF4},
    {"KP0", DECKEY_KP0},
    {"KP1", DECKEY_KP1},
    {"KP2", DECKEY_KP2},
    {"KP3", DECKEY_KP3},
    {"KP4", DECKEY_KP4},
    {"KP5", DECKEY_KP5},
    {"KP6", DECKEY_KP6},
    {"KP7", DECKEY_KP7},
    {"KP8", DECKEY_KP8},
    {"KP9", DECKEY_KP9},
    {"KPCOMA", DECKEY_KPCOMA},
    {"KPMINUS", DECKEY_KPMINUS},
    {"KPDOT", DECKEY_KPDOT},
    {"KPENTER", DECKEY_KPENTER},
    {NULL, 0}
};

static KEYMAP km[] = {
                      {FALSE, FALSE, FALSE, VK_UP, DECKEY_UPARR},
                      {FALSE, FALSE, FALSE, VK_DOWN, DECKEY_DNARR},
                      {FALSE, FALSE, FALSE, VK_RIGHT, DECKEY_RTARR},
                      {FALSE, FALSE, FALSE, VK_LEFT, DECKEY_LFARR},

                      {FALSE, FALSE, FALSE, VK_F1, DECKEY_PF1},
                      {FALSE, FALSE, FALSE, VK_F2, DECKEY_PF2},
                      {FALSE, FALSE, FALSE, VK_F3, DECKEY_PF3},
                      {FALSE, FALSE, FALSE, VK_F4, DECKEY_PF4},
                      {FALSE, FALSE, FALSE, VK_NUMPAD0, DECKEY_KP0},
                      {FALSE, FALSE, FALSE, VK_NUMPAD1, DECKEY_KP1},
                      {FALSE, FALSE, FALSE, VK_NUMPAD2, DECKEY_KP2},
                      {FALSE, FALSE, FALSE, VK_NUMPAD3, DECKEY_KP3},
                      {FALSE, FALSE, FALSE, VK_NUMPAD4, DECKEY_KP4},
                      {FALSE, FALSE, FALSE, VK_NUMPAD5, DECKEY_KP5},
                      {FALSE, FALSE, FALSE, VK_NUMPAD6, DECKEY_KP6},
                      {FALSE, FALSE, FALSE, VK_NUMPAD7, DECKEY_KP7},
                      {FALSE, FALSE, FALSE, VK_NUMPAD8, DECKEY_KP8},
                      {FALSE, FALSE, FALSE, VK_NUMPAD9, DECKEY_KP9},
                      {FALSE, FALSE, FALSE, VK_MULTIPLY, DECKEY_KPCOMA},
                      {FALSE, FALSE, FALSE, VK_SEPARATOR, DECKEY_KPCOMA},
                      {FALSE, FALSE, FALSE, VK_SUBTRACT, DECKEY_KPMINUS},
                      {FALSE, FALSE, FALSE, VK_DECIMAL, DECKEY_KPDOT},
                      {FALSE, FALSE, FALSE, VK_ADD, DECKEY_KPENTER},
                      {0, 0, 0, 0, NULL}};

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

VOID DrawTermDesc(VOID)
{
    static char szDesc[64];

    if (npTCB->bVT52)
        lstrcpy(szDesc, "DEC VT52");
    else
        lstrcpy(szDesc, "DEC VT100");

    if (npTCB->bCursor)
        lstrcat(szDesc, " CUR");

    if (npTCB->bApplication)
        lstrcat(szDesc, " APP");

    DrawEmul(szDesc);
}

VOID ResetTerm(VOID)
{
    int i;

    npTCB->nCurRow = 0; npTCB->nCurCol = 0;
    npTCB->nSavRow = 0; npTCB->nSavCol = 0;
    npTCB->nTermRows = 24; npTCB->nTermCols = 80;
    npTCB->nTop = -1; npTCB->nBottom = 0;
    npTCB->fg = npTCB->fgSav = DEC_WHITE;
    npTCB->bg = npTCB->bgSav = DEC_BLACK;
    npTCB->bold = npTCB->boldSav = FALSE;
    npTCB->ul = npTCB->ulSav = FALSE;
    npTCB->inv = npTCB->invSav = FALSE;
    npTCB->bEscape = FALSE;
    npTCB->bEscSeq = FALSE;
    npTCB->bIgnore = FALSE;
    npTCB->bCursor = FALSE;
    npTCB->bApplication = FALSE;
    npTCB->bRelative = FALSE;
    npTCB->bInsert = FALSE;
    npTCB->bQFlag = FALSE;
    npTCB->bCharSelSeq = FALSE;
    npTCB->bPoundSeq = FALSE;
    npTCB->bDirectCursorSeq = FALSE;
    npTCB->nDirectCursorSeq = 0;
    npTCB->nCharSelSet = npTCB->nCharSelSetSav = 0;
    npTCB->pCharSetG0 = npTCB->pCharSetG0 = sUSASCII;
    npTCB->pCharSetG1 = npTCB->pCharSetG1 = sUSASCII;
    npTCB->nCurCharSet = npTCB->nCurCharSetSav = 0;
    _fmemset(&npTCB->bTab, FALSE, sizeof(npTCB->bTab));
    for (i = 8; i < 132; i += 8)
        npTCB->bTab[i] = TRUE;

    SetTermSize(TERMROWS, TERMCOLS);
    SetVideoMode(VM_NORMAL);
    SetAttr((BYTE)(FgAttrMap[npTCB->fg] | BgAttrMap[npTCB->bg]));

    DrawTermDesc();
}

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
        case 'c':   // RESET INITIAL STATE
            ResetTerm();
            break;

        case '7':   // CURSOR SAVE
            npTCB->nSavRow = npTCB->nCurRow;
            npTCB->nSavCol = npTCB->nCurCol;
            npTCB->fgSav = npTCB->fg;
            npTCB->bgSav = npTCB->bg;
            npTCB->boldSav = npTCB->bold;
            npTCB->ulSav = npTCB->ul;
            npTCB->invSav = npTCB->inv;
            npTCB->nCurCharSetSav = npTCB->nCurCharSet;
            npTCB->nCharSelSetSav = npTCB->nCharSelSet;
            npTCB->pCharSetG0Sav = npTCB->pCharSetG0;
            npTCB->pCharSetG1Sav = npTCB->pCharSetG1;
            break;

        case '8':   // CURSOR RESTORE
            {
                BYTE cAttr;

                npTCB->nCurRow = npTCB->nSavRow;
                npTCB->nCurCol = npTCB->nSavCol;
                npTCB->fg = npTCB->fgSav;
                npTCB->bg = npTCB->bgSav;
                npTCB->bold = npTCB->boldSav;
                npTCB->ul = npTCB->ulSav;
                npTCB->inv = npTCB->invSav;
                npTCB->nCurCharSet = npTCB->nCurCharSetSav;
                npTCB->nCharSelSet = npTCB->nCharSelSetSav;
                npTCB->pCharSetG0 = npTCB->pCharSetG0Sav;
                npTCB->pCharSetG1 = npTCB->pCharSetG1Sav;

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

        case 'Z':   // IDENTIFY
            SendTermStr(szIdentStr, strlen(szIdentStr));
            break;

        case 'D':   // INDEX
            if (npTCB->nTop != -1) { /* scrolling region active */
                if (npTCB->nCurRow >= npTCB->nTop &&
                    npTCB->nCurRow <= npTCB->nBottom) { /* cursor is in scrolling region */
                    if (npTCB->nCurRow + 1 <= npTCB->nBottom)
                        (npTCB->nCurRow)++;
                    else {
                        SetRect(&Rect, 0, npTCB->nTop, LASTCOL, npTCB->nBottom);
                        ScrollScrn(0, 1, &Rect);
                    }
                }
                else /* cursor is not in active scrolling region */
                    if (npTCB->nCurRow + 1 < TERMROWS)
                        (npTCB->nCurRow)++;
            }
            else { /* no active scrolling region */
                if (npTCB->nCurRow + 1 < TERMROWS)
                    (npTCB->nCurRow)++;
                else
                    ScrollScrn(0, 1, NULL);
            }

            if ((GetLineAttr(npTCB->nCurRow) != LA_NORMAL) && (npTCB->nCurCol >= (TERMCOLS / 2)))
                npTCB->nCurCol = ((TERMCOLS / 2) - 1);

            break;

        case 'M':   // REVERSE INDEX
            if (npTCB->nTop != -1) { /* scrolling region active */
                if (npTCB->nCurRow >= npTCB->nTop &&
                    npTCB->nCurRow <= npTCB->nBottom) { /* cursor is in scrolling region */
                    if (npTCB->nCurRow - 1 >= npTCB->nTop)
                        (npTCB->nCurRow)--;
                    else {
                        SetRect(&Rect, 0, npTCB->nTop, LASTCOL, npTCB->nBottom);
                        ScrollScrn(0, -1, &Rect);
                    }
                }
                else /* cursor is not in active scrolling region */
                    if (npTCB->nCurRow - 1 >= 0)
                        (npTCB->nCurRow)--;
            }
            else { /* no active scrolling region */
                if (npTCB->nCurRow - 1 >= 0)
                    (npTCB->nCurRow)--;
                else
                    ScrollScrn(0, -1, NULL);
            }

            if ((GetLineAttr(npTCB->nCurRow) != LA_NORMAL) && (npTCB->nCurCol >= (TERMCOLS / 2)))
                npTCB->nCurCol = ((TERMCOLS / 2) - 1);
            break;

        case 'E':   // NEXT LINE
            npTCB->nCurCol = 0;
            if (npTCB->nTop != -1) { /* scrolling region active */
                if (npTCB->nCurRow >= npTCB->nTop &&
                    npTCB->nCurRow <= npTCB->nBottom) { /* cursor is in scrolling region */
                    if (npTCB->nCurRow + 1 <= npTCB->nBottom)
                        (npTCB->nCurRow)++;
                    else {
                        SetRect(&Rect, 0, npTCB->nTop, LASTCOL, npTCB->nBottom);
                        ScrollScrn(0, 1, &Rect);
                    }
                }
                else { /* cursor is not in active scrolling region */
                    if (npTCB->nCurRow + 1 < TERMROWS)
                        (npTCB->nCurRow)++;
                }
            }
            else { /* no active scrolling region */
                if (npTCB->nCurRow + 1 < TERMROWS)
                    (npTCB->nCurRow)++;
                else
                    ScrollScrn(0, 1, NULL);
            }

            break;

        case 'H':   // TAB SET
            if (npTCB->nCurCol >= 0 && npTCB->nCurCol < 132)
                npTCB->bTab[npTCB->nCurCol] = TRUE;
            break;

        case '=':
            npTCB->bApplication = TRUE;
            DrawTermDesc();
            break;

        case '>':
            npTCB->bApplication = FALSE;
            DrawTermDesc();
            break;

        case '[':
            npTCB->bEscSeq = TRUE;
            npTCB->nParmCnt = 0;
            npTCB->bQFlag = FALSE;
            memset(npTCB->szEscSeq, '\0', sizeof(npTCB->szEscSeq));
            for (i = 0; i < MAXPARM; i++)
                npTCB->nParm[i] = 0;
            break;

        case '#':
            npTCB->bPoundSeq = TRUE;
            break;

        case '(':
        case ')':
            npTCB->bCharSelSeq = TRUE;
            npTCB->nCharSelSet = TermChar == '(' ? 0 : 1;
            break;
    }

    npTCB->bEscape = FALSE;
}

/*----------------------------------------------------------------------------*/
VOID WriteTermCharEscVT52
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Handle a character being sent to the terminal.  This function handles     */
/*  control sequences as well as characters to be written.                    */
/*                                                                            */
(char TermChar)     /* Character for terminal.                                */
/*----------------------------------------------------------------------------*/

{
    switch (TermChar) {
        case 'A':           // CURSOR MOVE UP
            if (npTCB->nCurRow > 0)
                npTCB->nCurRow--;
            break;

        case 'B':           // CURSOR MOVE DOWN
            if (npTCB->nCurRow < LASTROW)
                npTCB->nCurRow++;
            break;

        case 'C':           // CURSOR MOVE RIGHT
            if (npTCB->nCurCol < LASTCOL)
                npTCB->nCurCol++;
            break;

        case 'D':           // CURSOR MOVE LEFT
            if (npTCB->nCurCol > 0)
                npTCB->nCurCol--;
            break;

        case 'E':           // CLEAR DISPLAY
            ScrollScrn(0, TERMROWS, NULL);
            FillScrn(NULL, ' ');
            FillAttr(NULL, DEFATTR);
            npTCB->nCurRow = 0;
            npTCB->nCurCol = 0;
            break;

        case 'F':       // ENTER GRAPHICS CHAR SET
            npTCB->pCharSetG0 = sLinedraw;
            break;

        case 'G':       // EXIT GRAPHICS CHAR SET
            npTCB->pCharSetG0 = sUKASCII;
            break;

        case 'H':           // CURSOR HOME
            npTCB->nCurRow = 0;
            npTCB->nCurCol = 0;
            break;

        case 'I':           // REVERSE INDEX
            if (npTCB->nCurRow  > 0)
                (npTCB->nCurRow)--;
            else
                ScrollScrn(0, -1, NULL);
            break;

        case 'J':           // ERASE TO END OF SCREEN
            if (npTCB->nCurRow < LASTROW) {
                SetRect(&Rect, 0, npTCB->nCurRow + 1, LASTCOL, LASTROW);
                FillScrn(&Rect, ' ');
                FillAttr(&Rect, DEFATTR);
            }
            SetRect(&Rect, npTCB->nCurCol, npTCB->nCurRow, LASTCOL, npTCB->nCurRow);
            FillScrn(&Rect, ' ');
            FillAttr(&Rect, DEFATTR);
            break;

        case 'K':           // ERASE TO END OF LINE
            SetRect(&Rect, npTCB->nCurCol, npTCB->nCurRow,
                    LASTCOL, npTCB->nCurRow);
            FillScrn(&Rect, ' ');
            FillAttr(&Rect, DEFATTR);
            break;

        case 'Y':
            npTCB->bDirectCursorSeq = TRUE;
            npTCB->nDirectCursorSeq = 0;
            break;

        case 'Z':   // IDENTIFY
            SendTermStr(szIdentStrVT52, strlen(szIdentStrVT52));
            break;

        case '=':
            npTCB->bApplication = TRUE;
            DrawTermDesc();
            break;

        case '>':
            npTCB->bApplication = FALSE;
            DrawTermDesc();
            break;

        case '<':
            npTCB->bVT52 = FALSE;
            ResetTerm();
            break;
    }

    npTCB->bEscape = FALSE;
}

VOID WriteDirectCursorSeq(char TermChar)
{
    static char cRow;

    if (npTCB->nDirectCursorSeq == 0) {
        npTCB->nDirectCursorSeq++;
        cRow = TermChar;
        return;
    }

    npTCB->bDirectCursorSeq = FALSE;
    npTCB->nDirectCursorSeq = 0;
    npTCB->nCurRow = (cRow - ' ');
    npTCB->nCurCol = (TermChar - ' ');
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

    if (TermChar == '?') {  // '?' flag, just note it and keep going
        npTCB->bQFlag = TRUE;
        return;
    }

    npTCB->nParm[npTCB->nParmCnt++] = atoi(npTCB->szEscSeq);
    memset(npTCB->szEscSeq, '\0', sizeof(npTCB->szEscSeq));

    if (TermChar == ';')    // parm separator, keep going
        return;

    switch (TermChar) {
        case 'H':           // CURSOR POSITION SET
        case 'f':
                /* scrolling region active & relative cursor positioning */
            if (npTCB->nTop != -1 && npTCB->bRelative) {
                int nTemp;
                nTemp = max(0, npTCB->nParm[0] - 1) + npTCB->nTop;
                npTCB->nCurRow = min(npTCB->nBottom, nTemp);
            }
            else
                npTCB->nCurRow = min(LASTROW, max(0, npTCB->nParm[0] - 1));
            npTCB->nCurCol = min(LASTCOL, max(0, npTCB->nParm[1] - 1));

            if ((GetLineAttr(npTCB->nCurRow) != LA_NORMAL) && (npTCB->nCurCol >= (TERMCOLS / 2)))
                npTCB->nCurCol = ((TERMCOLS / 2) - 1);
            break;

        case 'A':           // CURSOR MOVE UP
            {
                int i;

                if (npTCB->nParm[0] == 0)
                    npTCB->nParm[0] = 1;

                for (i = 0; i < npTCB->nParm[0]; i++) {
                    if (npTCB->nTop != -1 /* Scrolling Region Active */
                        && (npTCB->nCurRow >= npTCB->nTop &&
                            npTCB->nCurRow <= npTCB->nBottom)) { /* cursor is in scrolling region */
                        if (npTCB->nCurRow > npTCB->nTop)
                            npTCB->nCurRow--;
                    }
                    else {
                        if (npTCB->nCurRow > 0)
                            npTCB->nCurRow--;
                    }
                }
            }

            if ((GetLineAttr(npTCB->nCurRow) != LA_NORMAL) && (npTCB->nCurCol >= (TERMCOLS / 2)))
                npTCB->nCurCol = ((TERMCOLS / 2) - 1);
            break;

        case 'B':           // CURSOR MOVE DOWN
            {
                int i;

                if (npTCB->nParm[0] == 0)
                    npTCB->nParm[0] = 1;

                for (i = 0; i < npTCB->nParm[0]; i++) {
                    if (npTCB->nTop != -1 /* Scrolling Region Active */
                        && (npTCB->nCurRow >= npTCB->nTop &&
                            npTCB->nCurRow <= npTCB->nBottom)) { /* cursor is in scrolling region */
                        if (npTCB->nCurRow < npTCB->nBottom)
                            npTCB->nCurRow++;
                    }
                    else {
                        if (npTCB->nCurRow < LASTROW)
                            npTCB->nCurRow++;
                    }
                }
            }

            if ((GetLineAttr(npTCB->nCurRow) != LA_NORMAL) && (npTCB->nCurCol >= (TERMCOLS / 2)))
                npTCB->nCurCol = ((TERMCOLS / 2) - 1);
            break;

        case 'C':           // CURSOR MOVE RIGHT
            npTCB->nCurCol = min(LASTCOL, npTCB->nCurCol + max(1, npTCB->nParm[0]));

            if ((GetLineAttr(npTCB->nCurRow) != LA_NORMAL) && (npTCB->nCurCol >= (TERMCOLS / 2)))
                npTCB->nCurCol = ((TERMCOLS / 2) - 1);
            break;

        case 'D':           // CURSOR MOVE LEFT
            npTCB->nCurCol = max(0, npTCB->nCurCol - max(1, npTCB->nParm[0]));
            break;

        case 'J':           // ERASE ??? SCREEN
            {
                int i;

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
                        for (i = npTCB->nCurRow + 1; i < TERMROWS; i++)
                            SetLineAttr(i, LA_NORMAL);
                        if (npTCB->nCurCol == 0)
                            SetLineAttr(npTCB->nCurRow, LA_NORMAL);
                        break;

                    case 1:     // ... TO START OF ...
                        if (npTCB->nCurRow > 0) {
                            SetRect(&Rect, 0, 0, LASTCOL, npTCB->nCurRow - 1);
                            FillScrn(&Rect, ' ');
                            FillAttr(&Rect, DEFATTR);
                        }
                        SetRect(&Rect, 0, npTCB->nCurRow, npTCB->nCurCol, npTCB->nCurRow);
                        FillScrn(&Rect, ' ');
                        FillAttr(&Rect, DEFATTR);
                        for (i = npTCB->nCurRow; i >= 0; i--)
                            SetLineAttr(i, LA_NORMAL);
                        break;

                    case 2:     // ... ENTIRE ...
                        ScrollScrn(0, TERMROWS, NULL);
                        FillScrn(NULL, ' ');
                        FillAttr(NULL, DEFATTR);
                        npTCB->nCurRow = 0;
                        npTCB->nCurCol = 0;
                        for (i = 0; i < TERMROWS; i++)
                            SetLineAttr(i, LA_NORMAL);
                        break;
                }
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

        case 'P':           // CHARACTER DELETE
            SetRect(&Rect, npTCB->nCurCol, npTCB->nCurRow,
                           LASTCOL, npTCB->nCurRow);
            ScrollScrn(max(npTCB->nParm[0], 1), 0, &Rect);
            break;

        case '@':           // CHARACTER INSERT
            SetRect(&Rect, npTCB->nCurCol, npTCB->nCurRow,
                           LASTCOL, npTCB->nCurRow);
            ScrollScrn(-max(npTCB->nParm[0], 1), 0, &Rect);
            break;

        case 'M':           // LINE DELETE
            if (npTCB->nTop != -1) { /* scrolling region active */
                if (npTCB->nCurRow >= npTCB->nTop &&
                    npTCB->nCurRow <= npTCB->nBottom) { /* cursor is in scrolling region */
                    SetRect(&Rect, 0, npTCB->nCurRow, LASTCOL, npTCB->nBottom);
                    ScrollScrn(0, max(npTCB->nParm[0], 1), &Rect);
                }
                else { /* cursor is not in scrolling region */
                    /* do nothing ! */
                }
            }
            else { /* no active scrolling region */
                SetRect(&Rect, 0, npTCB->nCurRow, LASTCOL, LASTROW);
                ScrollScrn(0, max(npTCB->nParm[0], 1), &Rect);
            }

            if ((GetLineAttr(npTCB->nCurRow) != LA_NORMAL) && (npTCB->nCurCol >= (TERMCOLS / 2)))
                npTCB->nCurCol = ((TERMCOLS / 2) - 1);
            break;

        case 'L':           // LINE INSERT
            if (npTCB->nTop != -1) { /* scrolling region active */
                if (npTCB->nCurRow >= npTCB->nTop &&
                    npTCB->nCurRow <= npTCB->nBottom) { /* cursor is in scrolling region */
                    SetRect(&Rect, 0, npTCB->nCurRow, LASTCOL, npTCB->nBottom);
                    ScrollScrn(0, -max(npTCB->nParm[0], 1), &Rect);
                }
                else { /* cursor is not in scrolling region */
                    /* do nothing ! */
                }
            }
            else { /* no active scrolling region */
                SetRect(&Rect, 0, npTCB->nCurRow, LASTCOL, LASTROW);
                ScrollScrn(0, -max(npTCB->nParm[0], 1), &Rect);
            }

            if ((GetLineAttr(npTCB->nCurRow) != LA_NORMAL) && (npTCB->nCurCol >= (TERMCOLS / 2)))
                npTCB->nCurCol = ((TERMCOLS / 2) - 1);
            break;

        case 'n':           // READ ???
            switch (npTCB->nParm[0]) {
                static char szWorkBuf [16];

                case 6:     // CURSOR POSITION
                    wsprintf(szWorkBuf, "\x1B[%d;%dR",
                             npTCB->nCurRow + 1, npTCB->nCurCol + 1);
                    SendTermStr(szWorkBuf, strlen(szWorkBuf));
                    break;

                case 5:     // STATUS
                    wsprintf(szWorkBuf, "\x1B[0n");
                    SendTermStr(szWorkBuf, strlen(szWorkBuf));
                    break;
            }
            break;

        case 'c':           // TRANSMIT DEVICE ATTRIBUTES
            SendTermStr(szIdentStr, strlen(szIdentStr));
            break;

        case 'g':
            if (npTCB->nParm[0] == 0) {
                if (npTCB->nCurCol >= 0 && npTCB->nCurCol < 132)
                    npTCB->bTab[npTCB->nCurCol] = FALSE;
            }
            else if (npTCB->nParm[0] == 3)
                _fmemset(&npTCB->bTab, FALSE, sizeof(npTCB->bTab));
            break;

#define CONSTRAIN(src, dest, low, high) if ((src) < (low)) (dest) = (low); \
                                        else if ((src) > (high)) (dest) = (high); \
                                        else (dest) = (src);
        case 'r':
            CONSTRAIN(npTCB->nParm[0], npTCB->nParm[0], 1, TERMROWS)
            CONSTRAIN(npTCB->nParm[1], npTCB->nParm[1], 1, TERMROWS)
            if (npTCB->nParm[0] >= npTCB->nParm[1]) {
                npTCB->nTop = npTCB->nBottom = -1;
                npTCB->nCurCol = npTCB->nCurRow = 0;
            }
            else {
                npTCB->nTop = npTCB->nParm[0] - 1;
                npTCB->nBottom = npTCB->nParm[1] - 1;
                npTCB->nCurCol = 0;
                npTCB->nCurRow = npTCB->bRelative ? npTCB->nTop : 0;
            }
            break;

        case 'h':
        case 'l':
            if (npTCB->bQFlag) {
                int i;

                for (i = 0; i < npTCB->nParmCnt; i++) {
                    switch (npTCB->nParm[i]) {
                        case 1:
                            npTCB->bCursor = (TermChar == 'h');
                            DrawTermDesc();
                            break;

                        case 2:
                            if (TermChar == 'h')
                                npTCB->bVT52 = FALSE;
                            else
                                npTCB->bVT52 = TRUE;
                            ResetTerm();
                            break;

                        case 3:
                            if (TermChar == 'h')
                                TERMCOLS = 132;
                            else
                                TERMCOLS = 80;

                            SetTermSize(TERMROWS, TERMCOLS);
                            FillAttr(NULL, DEFATTR);
                            npTCB->nCurCol = npTCB->nCurRow = 0;
                            break;

                        case 5:
                            SetVideoMode(TermChar == 'h' ? VM_REVERSE : VM_NORMAL);
                            break;

                        case 6:
                            npTCB->bRelative = (TermChar == 'h');
                            break;

                        case 7:
                            npTCB->TermSet.LineWrap = (WORD)(TermChar == 'h');
                            break;
                    }
                }
            }
            else {
                switch (npTCB->nParm[0]) {
                    case 4:
                        npTCB->bInsert = (TermChar == 'h');
                        break;

                    case 20:
                        npTCB->TermSet.NewLine = (WORD)(TermChar == 'h');
                        break;
                }
            }
            break;

        case 'm':
            {
                int i;
                BYTE cAttr;

                for (i = 0; i < npTCB->nParmCnt; i++) {
                    if (npTCB->nParm[i] == 0) { /* Normal */
                        npTCB->fg = DEC_WHITE;  /* white */
                        npTCB->bg = DEC_BLACK;  /* on black */
                        npTCB->ul = FALSE;
                        npTCB->bold = FALSE;
                        npTCB->inv = FALSE;
                    }
                    else if (npTCB->nParm[i] == 1) /* Bold (simulate with yellow) */
                        npTCB->fg = DEC_YELLOW;
                    else if (npTCB->nParm[i] == 4) /* Underline (set underline flag) */
                        npTCB->ul = TRUE;
                    else if (npTCB->nParm[i] == 5) /* Blink (simulate with red) */
                        npTCB->fg = DEC_RED;
                    else if (npTCB->nParm[i] == 7) /* Inverse (set inverted flag) */
                        npTCB->inv = TRUE;
                    else if (npTCB->nParm[i] == 8) { /* Invisible (black on black) */
                        npTCB->fg = DEC_BLACK;
                        npTCB->bg = DEC_BLACK;
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
VOID WriteTermCharSelSeq
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Handle a character being sent to the terminal.  This function handles     */
/*  control sequences as well as characters to be written.                    */
/*                                                                            */
(char TermChar)     /* Character for terminal.                                */
/*----------------------------------------------------------------------------*/

{
    BYTE * pCharSetReq = NULL;

    switch (TermChar) {
        case 'A':       /* UK symbols */
            pCharSetReq = sUKASCII;
            break;

        case 'B':       /* ASCII Symbols */
            pCharSetReq = sUSASCII;
            break;

        case '0':       /* Special Graphics (Line Draw) */
            pCharSetReq = sLinedraw;
            break;

        case '1':       /* Alt Char ROM - National Symbols */
            pCharSetReq = sNational;
            break;

        case '2':       /* Alt Graphics ROM (Line Draw) */
            pCharSetReq = sLinedraw;
            break;
    }


    if (pCharSetReq != NULL)
        switch (npTCB->nCharSelSet) {
            case 0:
                npTCB->pCharSetG0 = pCharSetReq;
                break;

            case 1:
                npTCB->pCharSetG1 = pCharSetReq;
                break;
        }

    npTCB->bCharSelSeq = FALSE;
}

/*----------------------------------------------------------------------------*/
VOID WriteTermPoundSeq
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Handle a character being sent to the terminal.  This function handles     */
/*  control sequences as well as characters to be written.                    */
/*                                                                            */
(char TermChar)     /* Character for terminal.                                */
/*----------------------------------------------------------------------------*/

{
    switch (TermChar) {
        case '3':       /* Double high and wide (top half) */
            SetLineAttr(npTCB->nCurRow, LA_DBLTOP);
            break;

        case '4':       /* Double high and wide (bottom half) */
            SetLineAttr(npTCB->nCurRow, LA_DBLBOT);
            break;

        case '5':       /* Normal height and width */
            SetLineAttr(npTCB->nCurRow, LA_NORMAL);
            break;

        case '6':       /* Double wide */
            SetLineAttr(npTCB->nCurRow, LA_DBLWIDE);
            break;

        case '8':       /* Test Pattern, Fill screen with char 'E'  */
            ScrollScrn(0, TERMROWS, NULL);
            FillScrn(NULL, 'E');
            FillAttr(NULL, DEFATTR);
            break;
    }

    if ((GetLineAttr(npTCB->nCurRow) != LA_NORMAL) && (npTCB->nCurCol >= (TERMCOLS / 2)))
        npTCB->nCurCol = ((TERMCOLS / 2) - 1);

    npTCB->bPoundSeq = FALSE;
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
            int i;

            for (i = npTCB->nCurCol + 1; (i < TERMCOLS) && (npTCB->bTab[i] == FALSE); i++);
            if (GetLineAttr(npTCB->nCurRow) == LA_NORMAL)
                npTCB->nCurCol = min(i, TERMCOLS - 1);
            else
                npTCB->nCurCol = min(i, (TERMCOLS / 2) - 1);
            break;

        case 10:                    /* line feed */
        case 12:                    /* form feed (handle as line feed!!! ) */
            if (npTCB->nTop != -1) { /* scrolling region active */
                if (npTCB->nCurRow >= npTCB->nTop &&
                    npTCB->nCurRow <= npTCB->nBottom) { /* cursor is in scrolling region */
                    if (npTCB->nCurRow + 1 <= npTCB->nBottom)
                        (npTCB->nCurRow)++;
                    else {
                        SetRect(&Rect, 0, npTCB->nTop, LASTCOL, npTCB->nBottom);
                        ScrollScrn(0, 1, &Rect);
                    }
                }
                else { /* cursor is not in active scrolling region */
                    if (npTCB->nCurRow + 1 < TERMROWS)
                        (npTCB->nCurRow)++;
                }
            }
            else { /* no active scrolling region */
                if (npTCB->nCurRow + 1 < TERMROWS)
                    (npTCB->nCurRow)++;
                else
                    ScrollScrn(0, 1, NULL);
            }

            if ((GetLineAttr(npTCB->nCurRow) != LA_NORMAL) && (npTCB->nCurCol >= (TERMCOLS / 2)))
                npTCB->nCurCol = ((TERMCOLS / 2) - 1);
            break;

        case 13:                    /* carriage return */
            npTCB->nCurCol = 0;
            break;

        case 14:                    /* ^N - select G1 character set */
            npTCB->nCurCharSet = 1;
            break;

        case 15:                    /* ^O - select G0 character set */
            npTCB->nCurCharSet = 0;
            break;

        case 27:                    /* escape */
            npTCB->bEscape = TRUE;
            break;

        default:                    /* all other characters */
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
    int ActiveCols;

    if (GetLineAttr(npTCB->nCurRow) == LA_NORMAL)
        ActiveCols = TERMCOLS;
    else
        ActiveCols = TERMCOLS / 2;

    if (npTCB->nCurCol >= ActiveCols) {
        if (npTCB->TermSet.LineWrap) {
            WriteTermCharCtl('\r');
            WriteTermCharCtl('\n');
        }
        else
            npTCB->nCurCol = (ActiveCols - 1);
    }

    if (npTCB->bInsert) {
        SetRect(&Rect, npTCB->nCurCol, npTCB->nCurRow,
                       LASTCOL, npTCB->nCurRow);
        ScrollScrn(-1, 0, &Rect);
    }

    WriteScrnChar((npTCB->nCurCol)++, npTCB->nCurRow,
                  npTCB->nCurCharSet == 0 ? npTCB->pCharSetG0[TermChar] :
                                            npTCB->pCharSetG1[TermChar]);

    if (!npTCB->TermSet.LineWrap && (npTCB->nCurCol >= ActiveCols))
        npTCB->nCurCol = (ActiveCols - 1);
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
        if (npTCB->bVT52)
            WriteTermCharEscVT52(TermChar);
        else
            WriteTermCharEsc(TermChar);
    else if (npTCB->bDirectCursorSeq)
        WriteDirectCursorSeq(TermChar);
    else if (npTCB->bEscSeq)
        WriteTermCharEscSeq(TermChar);
    else if (npTCB->bCharSelSeq)
        WriteTermCharSelSeq(TermChar);
    else if (npTCB->bPoundSeq)
        WriteTermPoundSeq(TermChar);
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
    int ActiveCols;

    npTCB = (TCB *)LocalLock(hEmul);

    for (i = 0; i < StrLen; i++)
        WriteTermChar((char)(TermStr[i] & 0177), Raw);

    FlushScrnBuf();

    if (GetLineAttr(npTCB->nCurRow) == LA_NORMAL)
        ActiveCols = TERMCOLS;
    else
        ActiveCols = TERMCOLS / 2;
    SetCurPos(min(npTCB->nCurCol, (ActiveCols - 1)), npTCB->nCurRow, FALSE);

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

    if (npTCB->bVT52) {
        switch (cKey) {
            case DECKEY_LFARR:
                lstrcpy(szWorkStr, "\x1B" "D");
                break;

            case DECKEY_UPARR:
                lstrcpy(szWorkStr, "\x1B" "A");
                break;

            case DECKEY_RTARR:
                lstrcpy(szWorkStr, "\x1B" "C");
                break;

            case DECKEY_DNARR:
                lstrcpy(szWorkStr, "\x1B" "B");
                break;

            case DECKEY_PF1:
                lstrcpy(szWorkStr, "\x1B" "P");
                break;

            case DECKEY_PF2:
                lstrcpy(szWorkStr, "\x1B" "Q");
                break;

            case DECKEY_PF3:
                lstrcpy(szWorkStr, "\x1B" "R");
                break;

            case DECKEY_PF4:
                lstrcpy(szWorkStr, "\x1B" "S");
                break;

            case DECKEY_KP0:
            case DECKEY_KP1:
            case DECKEY_KP2:
            case DECKEY_KP3:
            case DECKEY_KP4:
            case DECKEY_KP5:
            case DECKEY_KP6:
            case DECKEY_KP7:
            case DECKEY_KP8:
            case DECKEY_KP9:
                if (npTCB->bApplication) {
                    lstrcpy(szWorkStr, "\x1B" "??");
                    szWorkStr[2] = (char)('p' + (char)(cKey - (UINT)DECKEY_KP0));
                }
                else {
                    szWorkStr[0] = (char)('0' + (char)(cKey - (UINT)DECKEY_KP0));
                    szWorkStr[1] = '\0';
                }
                break;

            case DECKEY_KPCOMA:
                lstrcpy(szWorkStr, npTCB->bApplication ? "\x1B" "?l" : ",");
                break;

            case DECKEY_KPENTER:
                lstrcpy(szWorkStr, npTCB->bApplication ? "\x1B" "?M" : "\r");
                break;

            case DECKEY_KPMINUS:
                lstrcpy(szWorkStr, npTCB->bApplication ? "\x1B" "?m" : "-");
                break;

            case DECKEY_KPDOT:
                lstrcpy(szWorkStr, npTCB->bApplication ? "\x1B" "?n" : ".");
                break;

            default:
                LocalUnlock(hEmul);
                return;
        }
    }
    else {
        switch (cKey) {
            case DECKEY_LFARR:
                lstrcpy(szWorkStr, npTCB->bCursor ? "\x1B" "OD" : "\x1B" "[D");
                break;

            case DECKEY_UPARR:
                lstrcpy(szWorkStr, npTCB->bCursor ? "\x1B" "OA" : "\x1B" "[A");
                break;

            case DECKEY_RTARR:
                lstrcpy(szWorkStr, npTCB->bCursor ? "\x1B" "OC" : "\x1B" "[C");
                break;

            case DECKEY_DNARR:
                lstrcpy(szWorkStr, npTCB->bCursor ? "\x1B" "OB" : "\x1B" "[B");
                break;

            case DECKEY_PF1:
                lstrcpy(szWorkStr, "\x1B" "OP");
                break;

            case DECKEY_PF2:
                lstrcpy(szWorkStr, "\x1B" "OQ");
                break;

            case DECKEY_PF3:
                lstrcpy(szWorkStr, "\x1B" "OR");
                break;

            case DECKEY_PF4:
                lstrcpy(szWorkStr, "\x1B" "OS");
                break;

            case DECKEY_KP0:
            case DECKEY_KP1:
            case DECKEY_KP2:
            case DECKEY_KP3:
            case DECKEY_KP4:
            case DECKEY_KP5:
            case DECKEY_KP6:
            case DECKEY_KP7:
            case DECKEY_KP8:
            case DECKEY_KP9:
                if (npTCB->bApplication) {
                    lstrcpy(szWorkStr, "\x1B" "O?");
                    szWorkStr[2] = (char)('p' + (char)(cKey - (UINT)DECKEY_KP0));
                }
                else {
                    szWorkStr[0] = (char)('0' + (char)(cKey - (UINT)DECKEY_KP0));
                    szWorkStr[1] = '\0';
                }
                break;

            case DECKEY_KPCOMA:
                lstrcpy(szWorkStr, npTCB->bApplication ? "\x1B" "Ol" : ",");
                break;

            case DECKEY_KPENTER:
                lstrcpy(szWorkStr, npTCB->bApplication ? "\x1B" "OM" : "\r");
                break;

            case DECKEY_KPMINUS:
                lstrcpy(szWorkStr, npTCB->bApplication ? "\x1B" "Om" : "-");
                break;

            case DECKEY_KPDOT:
                lstrcpy(szWorkStr, npTCB->bApplication ? "\x1B" "On" : ".");
                break;

            default:
                LocalUnlock(hEmul);
                return;
        }
    }

    SendTermStr(szWorkStr, strlen(szWorkStr));
    if (npTCB->TermSet.LocalEcho)
        WriteTerm(hEmul, szWorkStr, strlen(szWorkStr), FALSE);

    LocalUnlock(hEmul);
}

extern "C" BOOL DLLEXPORT CALLBACK DoKeyMap(HANDLE X(hEmul), KEYMAPENTRY FAR * lpkme)
{
    int    i;
    KEYMAP km2;

    for (i = 0; dkm[i].szKeyName != NULL; i++) {
        if (_fstricmp(dkm[i].szKeyName, lpkme->szKeyName) == 0) {
            km2.bShift = lpkme->bShift;
            km2.bControl = lpkme->bShift;
            km2.bAlternate = lpkme->bAlternate;
            km2.vk = lpkme->vk;
            km2.tk = dkm[i].nKeyID;
            KeyMapAdd(&km2);
            break;
        }
    }

    if (dkm[i].szKeyName == NULL) {
        char szMessage[80];

        wsprintf(szMessage, "Unknown DEC key neumonic %s!",
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

    npTCB->bVT52 = FALSE;
    ResetTerm();

    LocalUnlock(hEmul);

    KeyMapClear();
    if (!KeyMapParse("KERMDEC.KEY"))
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
