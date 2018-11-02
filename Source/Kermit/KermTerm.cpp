/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMTERM.C                                   **
**                                                                            **
**  This module contains the terminal emulation procedures.  A 24 X 80 TTY    **
**  is currently the only emulation supported (more to come).                 **
**                                                                            **
*******************************************************************************/

/* TODO ------------------------------------------------------------------------

  1. ReadLine capabiity currently uses a <BS> to attempt to erase characters
     when the user presses the BackSpace key.  This works poorly if the user
     has crossed a line boundary (cursor does not back up to previous line
     and continue erasing characters).  Enhance ReadLine behaviour to handle
     backspace across multiple lines.

  2. Enhance ReadLine cpability to provide a command line history.

------------------------------------------------------------------------------*/



/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"

#include <commdlg.h>

#include "kermtloc.h"
#include "kermtapi.h"
#include "kermtxfc.h"

#define COLFROMPT(x) ((x / xTermChar) + xOffset)
#define ROWFROMPT(y) ((y / yTermChar) + yOffset)

#define ONSCRN(x, y) (((x) >= 0) && ((x) < TERMCOLS) && ((y) >= 0) && ((y) < TERMROWS))
#define INRECT(x, y) (((x) >= Rect.left) && ((x) <= Rect.right) && \
                      ((y) >= Rect.top) && ((y) <= Rect.bottom))
#define SCRNCH(x, y) (*ScreenChar(fpTB, x, y))
#define SCRNAT(x, y) (*ScreenChar(fpAB, x, y))

#define ATTR_BOLD       0x80
#define ATTR_UNDERLINE  0x40

#define ATTR_FGBLACK    0x0
#define ATTR_FGBLUE     0x8
#define ATTR_FGGREEN    0x10
#define ATTR_FGCYAN     0x18
#define ATTR_FGRED      0x20
#define ATTR_FGMAGENTA  0x28
#define ATTR_FGYELLOW   0x30
#define ATTR_FGWHITE    0x38

#define ATTR_BGBLACK    0x0
#define ATTR_BGBLUE     0x1
#define ATTR_BGGREEN    0x2
#define ATTR_BGCYAN     0x3
#define ATTR_BGRED      0x4
#define ATTR_BGMAGENTA  0x5
#define ATTR_BGYELLOW   0x6
#define ATTR_BGWHITE    0x7

#define LA_NORMAL       0
#define LA_DBLWIDE      1
#define LA_DBLTOP       2
#define LA_DBLBOT       3

#define VM_NORMAL       0
#define VM_REVERSE      1


#define DEFCHAR         (' ')
#define DEFATTR         (ATTR_FGWHITE | ATTR_BGBLACK)

#define ATS_ADJSIZETOFONT       1
#define ATS_ADJRCTOSIZE         2
#define ATS_ADJSIZETORC         3
#define ATS_ADJFONTTOSIZE       4
#define ATS_ADJFONTTOSIZENOFIT  5

#define RCS_TERMSIZE        1
#define RCS_CURWINSIZE      2
#define RCS_PREVWINSIZE     3
#define RCS_AUTOMATIC       4

#define MAXKEYMAPENTRIES    100

/* TYPEDEFS ------------------------------------------------------------------*/

typedef struct {
    char * szKeyName;
    UINT   nKeyID;
} VIRTUALKEYMAP;

/* LOCAL VARIABLES -----------------------------------------------------------*/

static KEYMAP KeyMap [MAXKEYMAPENTRIES];    /* Array of keymap structures */
static int    nKeyMapEntries = 0;

static VIRTUALKEYMAP vkm [] = {
    {"BACK",        VK_BACK},
    {"TAB",         VK_TAB},
    {"CLEAR",       VK_CLEAR},
    {"RETURN",      VK_RETURN},
    {"SHIFT",       VK_SHIFT},
    {"CONTROL",     VK_CONTROL},
    {"MENU",        VK_MENU},
    {"PAUSE",       VK_PAUSE},
    {"CAPITAL",     VK_CAPITAL},
    {"ESCAPE",      VK_ESCAPE},
    {"SPACE",       VK_SPACE},
    {"PRIOR",       VK_PRIOR},
    {"NEXT",        VK_NEXT},
    {"END",         VK_END},
    {"HOME",        VK_HOME},
    {"LEFT",        VK_LEFT},
    {"UP",          VK_UP},
    {"RIGHT",       VK_RIGHT},
    {"DOWN",        VK_DOWN},
    {"SELECT",      VK_SELECT},
    {"EXECUTE",     VK_EXECUTE},
    {"SNAPSHOT",    VK_SNAPSHOT},
    {"INSERT",      VK_INSERT},
    {"DELETE",      VK_DELETE},
    {"HELP",        VK_HELP},
    {"0",           (0x30 + 0)},
    {"1",           (0x30 + 1)},
    {"2",           (0x30 + 2)},
    {"3",           (0x30 + 3)},
    {"4",           (0x30 + 4)},
    {"5",           (0x30 + 5)},
    {"6",           (0x30 + 6)},
    {"7",           (0x30 + 7)},
    {"8",           (0x30 + 8)},
    {"9",           (0x30 + 9)},
    {"A",           (0x41 + ('A' - 'A'))},
    {"B",           (0x41 + ('B' - 'A'))},
    {"C",           (0x41 + ('C' - 'A'))},
    {"D",           (0x41 + ('D' - 'A'))},
    {"E",           (0x41 + ('E' - 'A'))},
    {"F",           (0x41 + ('F' - 'A'))},
    {"G",           (0x41 + ('G' - 'A'))},
    {"H",           (0x41 + ('H' - 'A'))},
    {"I",           (0x41 + ('I' - 'A'))},
    {"J",           (0x41 + ('J' - 'A'))},
    {"K",           (0x41 + ('K' - 'A'))},
    {"L",           (0x41 + ('L' - 'A'))},
    {"M",           (0x41 + ('M' - 'A'))},
    {"N",           (0x41 + ('N' - 'A'))},
    {"O",           (0x41 + ('O' - 'A'))},
    {"P",           (0x41 + ('P' - 'A'))},
    {"Q",           (0x41 + ('Q' - 'A'))},
    {"R",           (0x41 + ('R' - 'A'))},
    {"S",           (0x41 + ('S' - 'A'))},
    {"T",           (0x41 + ('T' - 'A'))},
    {"U",           (0x41 + ('U' - 'A'))},
    {"V",           (0x41 + ('V' - 'A'))},
    {"W",           (0x41 + ('W' - 'A'))},
    {"X",           (0x41 + ('X' - 'A'))},
    {"Y",           (0x41 + ('Y' - 'A'))},
    {"Z",           (0x41 + ('Z' - 'A'))},
    {"DASH",        0xBD},      /* Some day I need to figure out why MS include */
    {"EQUAL",       0xBB},      /* files do not provide constants for these. */
    {"LBRACKET",    0xDB},
    {"RBRACKET",    0xDD},
    {"BACKSLASH",   0xDC},
    {"SEMICOLON",   0xBA},
    {"SQUOTE",      0xDE},
    {"COMMA",       0xBC},
    {"DOT",         0xBE},
    {"SLASH",       0xBF},
    {"BACKSQUOTE",  0xC0},
    {"NUMPAD0",     VK_NUMPAD0},
    {"NUMPAD1",     VK_NUMPAD1},
    {"NUMPAD2",     VK_NUMPAD2},
    {"NUMPAD3",     VK_NUMPAD3},
    {"NUMPAD4",     VK_NUMPAD4},
    {"NUMPAD5",     VK_NUMPAD5},
    {"NUMPAD6",     VK_NUMPAD6},
    {"NUMPAD7",     VK_NUMPAD7},
    {"NUMPAD8",     VK_NUMPAD8},
    {"NUMPAD9",     VK_NUMPAD9},
    {"MULTIPLY",    VK_MULTIPLY},
    {"ADD",         VK_ADD},
    {"SEPARATOR",   VK_SEPARATOR},
    {"SUBTRACT",    VK_SUBTRACT},
    {"DECIMAL",     VK_DECIMAL},
    {"DIVIDE",      VK_DIVIDE},
    {"F1",          VK_F1},
    {"F2",          VK_F2},
    {"F3",          VK_F3},
    {"F4",          VK_F4},
    {"F5",          VK_F5},
    {"F6",          VK_F6},
    {"F7",          VK_F7},
    {"F8",          VK_F8},
    {"F9",          VK_F9},
    {"F10",         VK_F10},
    {"F11",         VK_F11},
    {"F12",         VK_F12},
    {"F13",         VK_F13},
    {"F14",         VK_F14},
    {"F15",         VK_F15},
    {"F16",         VK_F16},
    {"F17",         VK_F17},
    {"F18",         VK_F18},
    {"F19",         VK_F19},
    {"F20",         VK_F20},
    {"F21",         VK_F21},
    {"F22",         VK_F22},
    {"F23",         VK_F23},
    {"F24",         VK_F24},
    {"NUMLOCK",     VK_NUMLOCK},
    {"SCROLL",      VK_SCROLL},
    {NULL,          0}
};

BOOL    bSuppress;
BOOL    bAdjustSize;
BOOL    bSizeActive;

int     nCurRow;
int     nCurCol;
int     xTermChar;         /* current terminal font width */
int     yTermChar;         /* current terminal font height */
int     xTermSize;         /* current terminal window width in pixels*/
int     yTermSize;         /* current terminal window height in pixels */
int     xScroll;           /* current max vertical scroll amt allowed in chars */
int     yScroll;           /* current max horizontal scroll amt allowed in chars */
int     xOffset;           /* current term window vertical offset in chars */
int     yOffset;           /* current term window horizontal offset in chars */
int     yOffsetSave;       /* saved (for review) term window horizontal offset in chars */
LONG    lBufIndex;         /* index to logical start of wrapping term buffer */
int     OrgX;
int     OrgY;
int     DestX;
int     DestY;
int     SaveX;
int     SaveY;
BOOL    bTrack;
HWND    hTermWnd;          /* terminal window handle */

HANDLE  hTermBuf = NULL;   /* handle to terminal screen buffer */
HANDLE  hAttrBuf = NULL;   /* handle to terminal attributes buffer */

BYTE    cCurAttr;

int     xBufStart;
int     xBufEnd;
int     yBuf;
BOOL    bBuf;
BOOL    bSizeAvail = FALSE;
BOOL    bAnsiCharSet = FALSE;

static char szCurModule [64] = "";

/* LOCAL DATA ----------------------------------------------------------------*/

#define AM_SP 0xC0  /* special mask    binary 11000000 */
#define AM_FG 0x38  /* foreground mask binary 00111000 */
#define AM_BG 0x07  /* background mask binary 00000111 */

#define GETATTRSP(attr)     ((BYTE)(((BYTE)attr & AM_SP) >> 6))
#define GETATTRFG(attr)     ((BYTE)(((BYTE)attr & AM_FG) >> 3))
#define GETATTRBG(attr)     ((BYTE)(((BYTE)attr & AM_BG) >> 0))

#define RGB_BLACK   (RGB(0,0,0))
#define RGB_BLUE    (RGB(0,0,255))
#define RGB_GREEN   (RGB(0,255,0))
#define RGB_CYAN    (RGB(0,255,255))
#define RGB_RED     (RGB(255,0,0))
#define RGB_MAGENTA (RGB(255,0,255))
#define RGB_YELLOW  (RGB(255,255,0))
#define RGB_WHITE   (RGB(255,255,255))

static COLORREF StdColorMap[] = {RGB_BLACK, RGB_BLUE, RGB_GREEN, RGB_CYAN,
                                 RGB_RED, RGB_MAGENTA, RGB_YELLOW, RGB_WHITE};
static COLORREF RevColorMap[] = {RGB_WHITE, RGB_BLUE, RGB_GREEN, RGB_CYAN,
                                 RGB_RED, RGB_MAGENTA, RGB_YELLOW, RGB_BLACK};
static COLORREF ColorMap [8];

#define FONT_COUNT  4

static HFONT    FontMap[FONT_COUNT] = {NULL,     /* Normal */
                                       NULL,     /* Underline */
                                       NULL,     /* Bold */
                                       NULL};    /* Bold Underline */

#define FM_NORMAL   0
#define FM_UL       1
#define FM_BOLD     2
#define FM_BOLDUL   3

#define FONT_NORMAL     (FontMap[FM_NORMAL])
#define FONT_BOLD       (FontMap[FM_BOLD])
#define FONT_UL         (FontMap[FM_UL])
#define FONT_BOLDUL     (FontMap[FM_BOLDUL])

#pragma pack(2)
typedef struct {
    LONG lfHeight;
    LONG lfWidth;
    LONG lfWeight;
    BYTE lfCharSet;
    BYTE lfPitchAndFamily;
    char lfFaceName[LF_FACESIZE];
} TermBlk;
#pragma pack()

TermBlk     TermSet;

TermBlk     TermDef = {0, 0, 0, OEM_CHARSET, FIXED_PITCH | FF_MODERN, "Terminal"};

/*******************************************************************************
*                                                                              *
* Private Terminal Functions (helpers)                                         *
*                                                                              *
*******************************************************************************/

/*----------------------------------------------------------------------------*/
LPSTR ScreenChar(LPVOID fpTB, int x, int y)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    LONG lBufPos;

    lBufPos = lBufIndex - (LONG)((UINT)CURSIZE) +
              (LONG)((LONG)y * (LONG)(CURCOLS + 2)) +
              (LONG)(x + 2);
    if (lBufPos < 0)
        lBufPos += BUFSIZE;

    return(((LPSTR)fpTB + lBufPos));
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC UpdateTermCaret(int x, int y)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    static BOOL bCaretExists =  FALSE;
    static int  xCurCaret = 0;
    static int  yCurCaret = 0;
    int         xCaret;
    int         yCaret;
    BOOL        bWide;

    bWide = !bReview && (LocGetLineAttr(y) != LA_NORMAL);

    if (GetFocus() != hTermWnd) {

        /* Don't want a caret at this time, destroy it if it exists and return */
        if (bCaretExists) {
            HideCaret(hTermWnd);
            DestroyCaret();
            bCaretExists = FALSE;
        }

        return;
    }

    if (bSuppress)
        return;

    /* if caret exists but is wrong size, also destroy to allow creation at
       proper size. */

    xCaret = (bWide ? (xTermChar * 2) : xTermChar);
    yCaret = yTermChar;

    if (bCaretExists && ((xCaret != xCurCaret) || (yCaret != yCurCaret))) {
        HideCaret(hTermWnd);
        DestroyCaret();
        bCaretExists = FALSE;
    }

    if (!bCaretExists) {
        CreateCaret(hTermWnd, NULL, xCaret, yCaret);
        xCurCaret = xCaret;
        yCurCaret = yCaret;
    }

    SetCaretPos((((bWide ? (x * 2) : x) - xOffset) * xTermChar),
                ((y - yOffset) * yTermChar));

    if (!bCaretExists) {
        ShowCaret(hTermWnd);
        bCaretExists = TRUE;
    }
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC SetTermFont(BOOL bSizeToTermWnd)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    HDC         hDC;
    TEXTMETRIC  tm;
    HFONT       hPrevFont;
    HFONT       hWorkFont;
    int         i;
    LOGFONT     lf;
    RECT        rc;


    /* If it appears that we have no idea what font size to ask for (height = 0)
       set up reasonable defaults.  Due to the wide variety of display types
       the only way I can come up with to do this is to ask for the OEM_FIXED_FONT
       stock object and use its characteristics! */

    if (TermSet.lfHeight == 0) {
        hDC = GetDC(hTermWnd);
        hPrevFont = (HFONT)SelectObject(hDC, GetStockObject(OEM_FIXED_FONT));
        GetTextMetrics(hDC, &tm);

        memset(&TermSet, 0, sizeof(TermSet));
        TermSet.lfHeight           = tm.tmHeight;
        TermSet.lfWidth            = tm.tmAveCharWidth;
        TermSet.lfWeight           = tm.tmWeight;
        TermSet.lfCharSet          = tm.tmCharSet;
        TermSet.lfPitchAndFamily   = tm.tmPitchAndFamily;
        GetTextFace(hDC, LF_FACESIZE, TermSet.lfFaceName);

        SelectObject(hDC, hPrevFont);
        ReleaseDC(hTermWnd, hDC);
    }

    memset(&lf, '\0', sizeof(lf));
    lf.lfHeight = (int)TermSet.lfHeight;
    lf.lfWidth = (int)TermSet.lfWidth;
    lf.lfWeight = FW_NORMAL;
    lf.lfUnderline = FALSE;
    lf.lfCharSet = TermSet.lfCharSet;
//    lf.lfQuality = PROOF_QUALITY;
    lf.lfPitchAndFamily = TermSet.lfPitchAndFamily;
    lstrcpy(lf.lfFaceName, TermSet.lfFaceName);

    /* Line below is just to avoid warning that rc may be uninitialized */
    rc.top = rc.bottom = rc.left = rc.right = 0;

    if (bSizeToTermWnd && bSizeAvail) {

        /* We want the client rect, but without any scrollbar area, if */
        /* any, so we use window rect and subtract the border width!   */

        GetWindowRect(hTermWnd, &rc);

        lf.lfHeight = ((rc.bottom - rc.top) / TERMROWS);
        lf.lfWidth = ((rc.right - rc.left) / TERMCOLS);
    }

    DebMsg(DL_INFO, "Font requested: %s @ %ix%i",
        (LPSTR)lf.lfFaceName, lf.lfHeight, lf.lfWidth);

    for (i = 0; i <= 1; i++) {
        hWorkFont = CreateFontIndirect(&lf);

        hDC = GetDC(hTermWnd);
        hPrevFont = (HFONT)SelectObject(hDC, hWorkFont);
        GetTextMetrics(hDC, &tm);

#ifdef _WIN32
        xTermChar = (int)tm.tmAveCharWidth;
        yTermChar = (int)(tm.tmHeight);
#else
        xTermChar = tm.tmAveCharWidth;
        yTermChar = tm.tmHeight;
#endif

        /* the following hack handles the bizarre case where the TEXTMETRIC
           field tmAveCharWidth for a TrueType font can indicate the wrong
           character width.  Since we are only dealing with fixed pitch
           fonts, just get the width of a single character (say X). */

        GetCharWidth(hDC, 'X', 'X', &xTermChar);

        memset(&TermSet, 0, sizeof(TermSet));
        TermSet.lfHeight           = tm.tmHeight;
        TermSet.lfWidth            = tm.tmAveCharWidth;
        TermSet.lfWeight           = tm.tmWeight;
        TermSet.lfCharSet          = tm.tmCharSet;
        TermSet.lfPitchAndFamily   = tm.tmPitchAndFamily;
        GetTextFace(hDC, LF_FACESIZE, TermSet.lfFaceName);

        /* If an ANSI character set has been selected, remember this so      */
        /* we can do an appropriate trnslation later (emulation modules      */
        /* all assume that they are dealing with the OEM character set.      */

        bAnsiCharSet = (tm.tmCharSet == ANSI_CHARSET);

        /* We don't need the Font object itself anymore                      */

        SelectObject(hDC, hPrevFont);
        ReleaseDC(hTermWnd, hDC);
        DeleteObject(hWorkFont);

        /* Another hack because in the case of a bitmap font, the Windows    */
        /* font selector algorithm simply chooses a font with a height that  */
        /* fits.  The width may still be too much which causes a horizontal  */
        /* scroll bar which then causes a vertical scroll bar (ugh!).  Fix   */
        /* is to subtract the height of a horizontal scroll bar and repeat   */
        /* the font request so that even if resulting font is too wide, the  */
        /* worst that will happen is we will get only a horizontal scroll    */
        /* bar.  Someday I should just enumerate the fonts when a bitmap     */
        /* font is used and manually ensure the font fits vertically AND     */
        /* horizontally!                                                     */

        if (bSizeToTermWnd && bSizeAvail && (xTermChar > lf.lfWidth)) {
//          lf.lfHeight = ((rc.bottom - rc.top - (yBord * 2) -
            lf.lfHeight = ((rc.bottom - rc.top -
                            GetSystemMetrics(SM_CYHSCROLL)) / TERMROWS);
        }
        else
            break;
    }

    DebMsg(DL_INFO, "Provided: %s @ %ix%i",
                       (LPSTR)TermSet.lfFaceName, TermSet.lfHeight,
                       TermSet.lfWidth);

    for (i = 0; i < FONT_COUNT; i++)
        if (FontMap[i] != NULL) {
            DeleteObject(FontMap[i]);
            FontMap[i] = NULL;
        }

    lf.lfWeight = FW_NORMAL;
    lf.lfUnderline = FALSE;
    FONT_NORMAL = CreateFontIndirect(&lf);

    lf.lfWeight = FW_BOLD;
    lf.lfUnderline = FALSE;
    FONT_BOLD = CreateFontIndirect(&lf);

    lf.lfWeight = FW_NORMAL;
    lf.lfUnderline = TRUE;
    FONT_UL = CreateFontIndirect(&lf);

    lf.lfWeight = FW_BOLD;
    lf.lfUnderline = TRUE;
    FONT_BOLDUL = CreateFontIndirect(&lf);
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC ResetTermWindow(BOOL bDraw)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    RECT rc;

    int xScrollBar, yScrollBar;

        /* make sure screen gets repainted */
    InvalidateRect(hTermWnd, NULL, TRUE);
    bSuppress = TRUE;

        /* update cursor size and position */
    UpdateTermCaret(nCurCol, nCurRow);

    if (!bSizeAvail)
        return;

        /* update scroll bars and window offsets */
    bSizeActive = TRUE;

    GetWindowRect(hTermWnd, &rc);

//    xTermSize = (rc.right - rc.left) - (xBord * 2);
//    yTermSize = (rc.bottom - rc.top) - (yBord * 2);
    xTermSize = (rc.right - rc.left);
    yTermSize = (rc.bottom - rc.top);

//    xScrollBar = (GetSystemMetrics(SM_CXVSCROLL) - xBord);
//    yScrollBar = (GetSystemMetrics(SM_CYHSCROLL) - yBord);
    xScrollBar = (GetSystemMetrics(SM_CXVSCROLL));
    yScrollBar = (GetSystemMetrics(SM_CYHSCROLL));

#ifdef HORZ_PRIORITY

    if ((xTermSize / xTermChar) < CURCOLS) {
        // horizontal scroll bar needed!
        yTermSize -= yScrollBar;
        if ((yTermSize / yTermChar) < CURROWS) {
            // vertical scroll bar needed also
            xTermSize -= xScrollBar;
        }
    }
    else {
        // no horizontal scroll bar needed so far!
        if ((yTermSize / yTermChar) < CURROWS) {
            // vertical scroll bar needed, adjust and recheck for horizontal
            xTermSize -= xScrollBar;
            if ((xTermSize / xTermChar) < CURCOLS)
                // horizontal scroll bar needed after all!
                yTermSize -= yScrollBar;
        }
    }

#else  /* VERT_PRIORITY */

    if ((yTermSize / yTermChar) < CURROWS) {
        // vertical scroll bar needed!
        xTermSize -= xScrollBar;
        if ((xTermSize / xTermChar) < CURCOLS) {
            // horizontal scroll bar needed also
            yTermSize -= yScrollBar;
        }
    }
    else {
        // no vertical scroll bar needed so far!
        if ((xTermSize / xTermChar) < CURCOLS) {
            // horizontal scroll bar needed, adjust and recheck for vertical
            yTermSize -= yScrollBar;
            if ((yTermSize / yTermChar) < CURROWS)
                // vertical scroll bar needed after all!
                xTermSize -= xScrollBar;
        }
    }

#endif

    xScroll = max(0, (CURCOLS - VISCOLS));
    if (xOffset < 0)
        xOffset = 0;
    if (xOffset > xScroll)
        xOffset = xScroll;

    if (bDraw) {
        SetScrollRange(hTermWnd, SB_HORZ, 0, xScroll, FALSE);
        SetScrollPos(hTermWnd, SB_HORZ, xOffset, TRUE);
    }

    yScroll = max(0, (CURROWS - VISROWS));
    if (yOffset < 0)
        yOffset = 0;
    if (yOffset > yScroll)
        yOffset = yScroll;

    if (bDraw) {
        SetScrollRange(hTermWnd, SB_VERT, 0, yScroll, FALSE);
        SetScrollPos(hTermWnd, SB_VERT, yOffset, TRUE);
    }

/*    UpdateWindow(hTermWnd); */
    bSizeActive = FALSE;
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC AdjustTermSize(BOOL bAdjFont, int nRowColSource)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int  xDelta, yDelta;
    int  xApp, yApp;
    int  xTerm, yTerm;
    int  i;
    int  nWantRows, nWantCols;
    RECT rcApp, rcTerm;

        /* If the appication is maximized, don;t size the window!!! */

    if (IsZoomed(hAppWnd)) {

            /* Changing the font size is OK, but only if requested */

        if (bAdjFont)
            SetTermFont(TRUE);

            /* Just reset the terminal window (scroll bars and stuff)
               and return */
        ResetTermWindow(TRUE);

        return;
    }

    /* The window sizing loop is driven by a desired number of rows
       and columns.  We need to prime these values */

    if (nRowColSource == RCS_AUTOMATIC) {
        if ((VISROWS >= TERMROWS) && (VISCOLS >= TERMCOLS))
            nRowColSource = RCS_TERMSIZE;
        else
            nRowColSource = RCS_PREVWINSIZE;
    }

    switch (nRowColSource) {

        /* In this case, we want to work to the number of rows and columns
           that should be visible after a user initiated terminal window
           size change. We cheat by calling ResetTermWindow to update the
           x and yTermSize globals and then fall through. */

        /* This probably doesn't work right when combined with the
           bAdjFont option because ResetTermWindow is determining the
           visible rows/cols (x and yTermSize) without taking a likely
           font change into account! */

        case RCS_CURWINSIZE:
            ResetTermWindow(FALSE);

        /* In this case, we want to work to the number of rows and columns
           that were visible prior to a user initiated window size change. */

        case RCS_PREVWINSIZE:
            nWantRows = min(VISROWS, TERMROWS);
            nWantCols = min(VISCOLS, TERMCOLS);
            break;

        /* In this case, we simply want to work to the number of rows and
           cols that the terminal emulator is dealing with. */

        case RCS_TERMSIZE:
        default:
            nWantRows = TERMROWS;
            nWantCols = TERMCOLS;
            break;
    }

    /* try to adjust the window size up to 5 times, then give up assuming
       that we are in one of those ugly situations where the scroll bars
       and menu bar are  fighting with each other for screen space. */

    for (i = 0; i < 5; i++) {

        /* get current size of full app window and calculate size */
        GetWindowRect(hAppWnd, &rcApp);
        xApp = (int)(rcApp.right - rcApp.left);
        yApp = (int)(rcApp.bottom - rcApp.top);

        DebMsg(DL_INFO, "Screen Size Now %i x %i", xApp, yApp);

        /* get current size of term window client area */
        GetWindowRect(hTermWnd, &rcTerm);
//        xTerm = (rcTerm.right - rcTerm.left) - (xBord * 2);
//        yTerm = (rcTerm.bottom - rcTerm.top) - (yBord * 2);
        xTerm = (rcTerm.right - rcTerm.left);
        yTerm = (rcTerm.bottom - rcTerm.top);

        /* if this is the first time through and we are interested in
           picking a font more appropriate to the new window size, attempt
           to select it here */

        if (i == 0 && bAdjFont)
            SetTermFont(TRUE);

            /* compute the difference (ie. changes desired) */
        xDelta = (xTermChar * nWantCols) - xTerm;
        yDelta = (yTermChar * nWantRows) - yTerm;

            /* account for scroll bars, if necessary */
        if (nWantCols < CURCOLS)
//            yDelta += (GetSystemMetrics(SM_CYHSCROLL) - yBord);
            yDelta += (GetSystemMetrics(SM_CYHSCROLL));
        if (nWantRows < CURROWS)
//            xDelta += (GetSystemMetrics(SM_CXVSCROLL) - xBord);
            xDelta += (GetSystemMetrics(SM_CXVSCROLL));

            /* break out if no changes needed */
        if (xDelta == 0 && yDelta == 0)
            break;

            /* compute desired new size for app window */
        xApp += xDelta;
        yApp += yDelta;

        /* Prevents recursion that would result from MoveWindow() */

        bAdjustSize = TRUE;

        /* request new app window size */

        MoveWindow(hAppWnd, (int)rcApp.left, (int)rcApp.top,
           (int)(xApp), (int)(yApp), TRUE);

        /* Danger of recursion is now over, return to normal... */

        bAdjustSize = FALSE;

        /* Update our terminal window info, primarily x and yTermSize */

        ResetTermWindow(FALSE);

        /* get updated size of full app window */
        GetWindowRect(hAppWnd, &rcApp);

        /* Update the desired rows and cols based on what turns out to be visible */

        /* if we didn't get the width we asked for, compute a new desired cols
           based on the term width that it seems we can get! */
        if ((rcApp.right - rcApp.left) != xApp)
            nWantCols = VISCOLS;

        /* if we didn't get the height we asked for, compute a new desired cols
           based on the term height that it seems we can get! */
        if ((rcApp.bottom - rcApp.top) != yApp)
            nWantRows = VISROWS;
    }

    ResetTermWindow(TRUE);

//    KermitFmtMsgBox(MB_OK, "Iterations: %i", i);
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC ChooseTermFont(HINSTANCE hInst, HWND hWnd)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    CHOOSEFONT  cf;
    LOGFONT     lf;
    HDC         hDC;
    HFONT       hPrevFont;
    TEXTMETRIC  tm;

    memset(&cf, 0, sizeof(cf));

    memset(&lf, '\0', sizeof(lf));
    lf.lfHeight = (int)TermSet.lfHeight;
    lf.lfWidth = (int)TermSet.lfWidth;
    lf.lfWeight = (int)TermSet.lfWeight;
    lf.lfCharSet = TermSet.lfCharSet;
    lf.lfPitchAndFamily = TermSet.lfPitchAndFamily;
    lstrcpy(lf.lfFaceName, TermSet.lfFaceName);

    /* the following hack is used to get the "point" size of the current font
       (i.e., a negative height value) because ChooseFont doesn't deal with
       a LOGFONT containing the full character height correctly. */

    hDC = GetDC(hTermWnd);
    hPrevFont = (HFONT)SelectObject(hDC, FONT_NORMAL);
    GetTextMetrics(hDC, &tm);

    lf.lfHeight = -(tm.tmHeight - tm.tmInternalLeading);

    SelectObject(hDC, hPrevFont);
    ReleaseDC(hTermWnd, hDC);

    /* end of ridiculous hack */

    cf.lStructSize    = sizeof(CHOOSEFONT);
    cf.hwndOwner      = hWnd;
    cf.lpLogFont      = &lf;
    cf.Flags          = CF_SCREENFONTS | CF_FIXEDPITCHONLY |
                        CF_INITTOLOGFONTSTRUCT | CF_ENABLETEMPLATE;
    cf.hInstance      = hInst;
    cf.lpTemplateName = "FontDlgBox";
    cf.lStructSize    = sizeof(CHOOSEFONT);
    cf.nFontType      = REGULAR_FONTTYPE;

    if (!ChooseFont(&cf))
        return(FALSE);

    TermSet.lfHeight = lf.lfHeight;
    TermSet.lfWidth = lf.lfWidth;
    TermSet.lfWeight = lf.lfWeight;
    TermSet.lfCharSet = lf.lfCharSet;
    TermSet.lfPitchAndFamily = lf.lfPitchAndFamily;
    lstrcpy(TermSet.lfFaceName, lf.lfFaceName);

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC DrawRow(HDC hDC, int Row, int StartCol, int EndCol, BOOL bInvert)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    LPVOID fpTB, fpAB;
    LPSTR lpszTerm, lpszAttr;
    int Col, Start, Length;
    HFONT hPrevFont;
    char cLineAttr;
    char szWorkBuf[256];

    fpTB = GlobalLock(hTermBuf);
    fpAB = GlobalLock(hAttrBuf);

    lpszTerm = ScreenChar(fpTB, 0, Row);
    lpszAttr = ScreenChar(fpAB, 0, Row);
    cLineAttr = *(ScreenChar(fpTB, -2, Row));
    Start = StartCol;
    Length = 0;

    if (cLineAttr == LA_NORMAL) {
        for (Col = StartCol; Col <= EndCol; Col++) {
            if (lpszAttr[Start + Length] == lpszAttr[Start])
                Length++;
            else {
                if (bAnsiCharSet)
                    OemToAnsiBuff(lpszTerm + Start, szWorkBuf, Length);
                else
                    _fmemcpy(szWorkBuf, lpszTerm + Start, Length);
                hPrevFont = (HFONT)SelectObject(hDC, FontMap[GETATTRSP(lpszAttr[Start])]);
                SetTextColor(hDC, ColorMap[bInvert ? GETATTRBG(lpszAttr[Start]) : GETATTRFG(lpszAttr[Start])]);
                SetBkColor(hDC, ColorMap[bInvert ? GETATTRFG(lpszAttr[Start]) : GETATTRBG(lpszAttr[Start])]);
                TextOut(hDC, ((Start - xOffset) * xTermChar), (Row - yOffset) * yTermChar,
                        szWorkBuf, Length);
                SelectObject(hDC, hPrevFont);
                Start += Length;
                Length = 1;
            }
        }

        if (bAnsiCharSet)
            OemToAnsiBuff(lpszTerm + Start, szWorkBuf, Length);
        else
            _fmemcpy(szWorkBuf, lpszTerm + Start, Length);
        hPrevFont = (HFONT)SelectObject(hDC, FontMap[GETATTRSP(lpszAttr[Start])]);
        SetTextColor(hDC, ColorMap[bInvert ? GETATTRBG(lpszAttr[Start]) : GETATTRFG(lpszAttr[Start])]);
        SetBkColor(hDC, ColorMap[bInvert ? GETATTRFG(lpszAttr[Start]) : GETATTRBG(lpszAttr[Start])]);
        TextOut(hDC, ((Start - xOffset) * xTermChar), (Row - yOffset) * yTermChar,
                szWorkBuf, Length);
        SelectObject(hDC, hPrevFont);
    }
    else {
        HDC hMem1DC;
        HBITMAP hMem1Bitmap, hMem1PrevBitmap;
        HDC hMem2DC;
        HBITMAP hMem2Bitmap, hMem2PrevBitmap;
        int CharCol;
        BOOL Odd;

        hMem1Bitmap = CreateCompatibleBitmap(hDC, xTermChar, yTermChar);
        hMem1DC = CreateCompatibleDC(hDC);

        hMem2Bitmap = CreateCompatibleBitmap(hDC, xTermChar * 2, yTermChar * 2);
        hMem2DC = CreateCompatibleDC(hDC);

        hMem1PrevBitmap = (HBITMAP)SelectObject(hMem1DC, hMem1Bitmap);
        hMem2PrevBitmap = (HBITMAP)SelectObject(hMem2DC, hMem2Bitmap);

        for (Col = StartCol; Col <= EndCol; Col++) {
            CharCol = Col / 2;
            Odd = ((Col % 2) == 1);

            if (bAnsiCharSet)
                OemToAnsiBuff(ScreenChar(fpTB, CharCol, Row), szWorkBuf, 1);
            else
                _fmemcpy(szWorkBuf, ScreenChar(fpTB, CharCol, Row), 1);

            hPrevFont = (HFONT)SelectObject(hMem1DC, FontMap[GETATTRSP(lpszAttr[CharCol])]);
            SetTextColor(hMem1DC, ColorMap[bInvert ? GETATTRBG(lpszAttr[CharCol]) : GETATTRFG(lpszAttr[CharCol])]);
            SetBkColor(hMem1DC, ColorMap[bInvert ? GETATTRFG(lpszAttr[CharCol]) : GETATTRBG(lpszAttr[CharCol])]);
            TextOut(hMem1DC, 0, 0, szWorkBuf, 1);
            StretchBlt(hMem2DC, 0, 0, xTermChar * 2, yTermChar * 2,
                       hMem1DC, 0, 0, xTermChar, yTermChar, SRCCOPY);

            switch (cLineAttr) {
                case LA_DBLWIDE:
                    StretchBlt(hDC, ((Col - xOffset) * xTermChar), ((Row - yOffset) * yTermChar),
                               xTermChar, yTermChar,
                               hMem2DC, Odd ? xTermChar : 0, 0, xTermChar, yTermChar * 2, SRCCOPY);
                    break;

                case LA_DBLTOP:
                    StretchBlt(hDC, (Col - xOffset) * xTermChar, (Row - yOffset) * yTermChar,
                               xTermChar, yTermChar,
                               hMem2DC, Odd ? xTermChar : 0, 0, xTermChar, yTermChar, SRCCOPY);
                    break;

                case LA_DBLBOT:
                    StretchBlt(hDC, (Col - xOffset) * xTermChar, (Row - yOffset) * yTermChar,
                               xTermChar, yTermChar,
                               hMem2DC, Odd ? xTermChar : 0, yTermChar, xTermChar, yTermChar, SRCCOPY);
                    break;
            }

            SelectObject(hMem1DC, hPrevFont);
        }

        SelectObject(hMem1DC, hMem1PrevBitmap);
        SelectObject(hMem2DC, hMem2PrevBitmap);

        DeleteDC(hMem1DC);
        DeleteObject(hMem1Bitmap);

        DeleteDC(hMem2DC);
        DeleteObject(hMem2Bitmap);
    }

    GlobalUnlock(hTermBuf);
    GlobalUnlock(hAttrBuf);
}

/*******************************************************************************
*                                                                              *
* Local Implementation of Service Functions (public, but not exported)         *
*                                                                              *
*******************************************************************************/

/*----------------------------------------------------------------------------*/
VOID PUBFUNC LocSendTermChar(char cTermChar)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (!bConnected)
        return;

    WriteCommChar(cTermChar);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC LocSendTermStr(LPSTR lpszTermStr, int nStrLen)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (!bConnected)
        return;

    WriteCommStr(lpszTermStr, nStrLen);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC LocFlushScrnBuf(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    HDC     hDC;

    if (bBuf && !bSuppress) {
        HideCaret(hTermWnd);
        hDC = GetDC(hTermWnd);
        if (LocGetLineAttr(yBuf) == LA_NORMAL)
            DrawRow(hDC, yBuf, xBufStart, xBufEnd, FALSE);
        else
            DrawRow(hDC, yBuf, xBufStart * 2, (xBufEnd * 2) + 1, FALSE);
        ReleaseDC(hTermWnd, hDC);
        ShowCaret(hTermWnd);
    }

    bBuf = FALSE;
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC LocWriteScrnStr(int xPos, int yPos, LPSTR lpsScrnStr, int nLength)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    LPVOID  fpTB, fpAB;

    if (bReview)
        return;

    if (xPos < 0 || yPos < 0 || xPos >= TERMCOLS || yPos >= TERMROWS)
        return;

    if (xPos + nLength > TERMCOLS)
        nLength = TERMROWS - xPos;

    if (bBuf && (yPos != yBuf))
        LocFlushScrnBuf();

    fpTB = GlobalLock(hTermBuf);
    fpAB = GlobalLock(hAttrBuf);

    _fmemcpy(ScreenChar(fpTB, xPos, yPos), lpsScrnStr, nLength);
    _fmemset(ScreenChar(fpAB, xPos, yPos), cCurAttr, nLength);

    GlobalUnlock(hTermBuf);
    GlobalUnlock(hAttrBuf);

    if (bBuf) {    /* we're buffering */
        xBufStart = min(xBufStart, xPos);
        xBufEnd = max(xBufEnd, xPos + nLength);
    }
    else {          /* start buffering */
        xBufStart = xPos;
        xBufEnd = xPos + nLength;
        yBuf = yPos;
        bBuf = TRUE;
    }
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC LocWriteScrnChar(int xPos, int yPos, char ScrnChar)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    LPVOID  fpTB, fpAB;

    if (bReview)
        return;

    if (xPos < 0 || yPos < 0 || xPos >= TERMCOLS || yPos >= TERMROWS)
        return;

    if (bBuf && (yPos != yBuf))
        LocFlushScrnBuf();

    fpTB = GlobalLock(hTermBuf);
    fpAB = GlobalLock(hAttrBuf);

    (*ScreenChar(fpTB, xPos, yPos)) = ScrnChar;
    (*ScreenChar(fpAB, xPos, yPos)) = cCurAttr;

    GlobalUnlock(hTermBuf);
    GlobalUnlock(hAttrBuf);

    if (bBuf) {    /* we're buffering */
        xBufStart = min(xBufStart, xPos);
        xBufEnd = max(xBufEnd, xPos);
    }
    else {          /* start buffering */
        xBufStart = xPos;
        xBufEnd = xPos;
        yBuf = yPos;
        bBuf = TRUE;
    }
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC LocScrollScrn(int xAmount, int yAmount, LPRECT lpRect)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    LPVOID  fpTB, fpAB;
    int     x, y;
    RECT    Rect;

    if (bReview)
        return;

        /* If nothing to do, just exit (prevents problems below! */
    if (xAmount == 0 && yAmount == 0)
        return;

    fpTB = GlobalLock(hTermBuf);
    fpAB = GlobalLock(hAttrBuf);

    if (lpRect == NULL)
        SetRect(&Rect, 0, 0, TERMCOLS - 1, TERMROWS - 1);
    else
        Rect = *lpRect;

    Rect.left = max(Rect.left, 0);
    Rect.top = max(Rect.top, 0);
    Rect.right = min(Rect.right, TERMCOLS - 1);
    Rect.bottom = min(Rect.bottom, TERMROWS - 1);

    if (((Rect.top == 0) && (Rect.bottom >= CURROWS - 1) &&
         (Rect.left == 0) && (Rect.right >= CURCOLS - 1)) &&
        ((xAmount == 0) && (yAmount > 0))) {

        /* We're forward line scrolling, just wrap! */
        for (y = 0; y < yAmount; y++) {
            /* clear line that is about to be exposed first */
            _fmemset((LPVOID)((LPCSTR)fpTB + lBufIndex + 2), DEFCHAR, TERMCOLS);
            _fmemset((LPVOID)((LPCSTR)fpTB + lBufIndex), '\0', 2);
            _fmemset((LPVOID)((LPCSTR)fpAB + lBufIndex + 2), cCurAttr, TERMCOLS);
            _fmemset((LPVOID)((LPCSTR)fpAB + lBufIndex), '\0', 2);

            lBufIndex += (TERMCOLS + 2);
            if (lBufIndex >= (LONG)BUFSIZE)
                lBufIndex -= BUFSIZE;
        }
    }

    else if ((Rect.left == 0) && (Rect.right >= CURCOLS - 1) && (xAmount == 0)) {

        /* other types of line scrolling */

        if (yAmount > 0) {
            for (y = Rect.top; y <= Rect.bottom; y++) {
                /* move current line up by yAmount (with line attributes) */
                if (y >= Rect.top && y <= Rect.bottom &&
                    y - yAmount >= Rect.top && y - yAmount <= Rect.bottom) {
                    _fmemcpy(ScreenChar(fpTB, -2, y - yAmount), ScreenChar(fpTB, -2, y), TERMCOLS + 2);
                    _fmemcpy(ScreenChar(fpAB, -2, y - yAmount), ScreenChar(fpAB, -2, y), TERMCOLS + 2);
                }
                /* clear uncovered line (including line attributes) */
                if (y >= Rect.top && y <= Rect.bottom) {
                    _fmemset(ScreenChar(fpTB, 0, y), DEFCHAR, TERMCOLS);
                    _fmemset(ScreenChar(fpTB, -2, y), '\0', 2);
                    _fmemset(ScreenChar(fpAB, 0, y), cCurAttr, TERMCOLS);
                    _fmemset(ScreenChar(fpAB, -2, y), '\0', 2);
                }
            }
        }
        else if (yAmount < 0) {
            for (y = Rect.bottom; y >= Rect.top; y--) {
                /* move current line up by yAmount (with line attributes) */
                if (y >= Rect.top && y <= Rect.bottom &&
                    y - yAmount >= Rect.top && y - yAmount <= Rect.bottom) {
                    _fmemcpy(ScreenChar(fpTB, -2, y - yAmount), ScreenChar(fpTB, -2, y), TERMCOLS + 2);
                    _fmemcpy(ScreenChar(fpAB, -2, y - yAmount), ScreenChar(fpAB, -2, y), TERMCOLS + 2);
                }
                /* clear uncovered line (including line attributes) */
                if (y >= Rect.top && y <= Rect.bottom) {
                    _fmemset(ScreenChar(fpTB, 0, y), DEFCHAR, TERMCOLS);
                    _fmemset(ScreenChar(fpTB, -2, y), '\0', 2);
                    _fmemset(ScreenChar(fpAB, 0, y), cCurAttr, TERMCOLS);
                    _fmemset(ScreenChar(fpAB, -2, y), '\0', 2);
                }
            }
        }
    }

    else {
        if (yAmount >= 0) {
            for (y = (int)Rect.top; y <= (int)Rect.bottom; y++) {
                if (xAmount >= 0) {
                    for (x = (int)Rect.left; x <= (int)Rect.right; x++) {
                        if (INRECT(x, y)) {
                            if (INRECT(x - xAmount, y - yAmount)) {
                                SCRNCH(x - xAmount, y - yAmount) = SCRNCH(x, y);
                                SCRNAT(x - xAmount, y - yAmount) = SCRNAT(x, y);
                            }
                            SCRNCH(x, y) = DEFCHAR;
                            SCRNAT(x, y) = cCurAttr;
                        }
                    }
                }
                else { /* xAmount < 0 */
                    for (x = (int)Rect.right; x >= (int)Rect.left; x--) {
                        if (INRECT(x, y)) {
                            if (INRECT(x - xAmount, y - yAmount)) {
                                SCRNCH(x - xAmount, y - yAmount) = SCRNCH(x, y);
                                SCRNAT(x - xAmount, y - yAmount) = SCRNAT(x, y);
                            }
                            SCRNCH(x, y) = DEFCHAR;
                            SCRNAT(x, y) = cCurAttr;
                        }
                    }
                }
            }
        }
        else { /* yAmount < 0 */
            for (y = (int)Rect.bottom; y >= (int)Rect.top; y--) {
                if (xAmount >= 0) {
                    for (x = (int)Rect.left; x <= (int)Rect.right; x++) {
                        if (INRECT(x, y)) {
                            if (INRECT(x - xAmount, y - yAmount)) {
                                SCRNCH(x - xAmount, y - yAmount) = SCRNCH(x, y);
                                SCRNAT(x - xAmount, y - yAmount) = SCRNAT(x, y);
                            }
                            SCRNCH(x, y) = DEFCHAR;
                            SCRNAT(x, y) = cCurAttr;
                        }
                    }
                }
                else { /* xAmount < 0 */
                    for (x = (int)Rect.right; x >= (int)Rect.left; x--) {
                        if (INRECT(x, y)) {
                            if (INRECT(x - xAmount, y - yAmount)) {
                                SCRNCH(x - xAmount, y - yAmount) = SCRNCH(x, y);
                                SCRNAT(x - xAmount, y - yAmount) = SCRNAT(x, y);
                            }
                            SCRNCH(x, y) = DEFCHAR;
                            SCRNAT(x, y) = cCurAttr;
                        }
                    }
                }
            }
        }
    }

    GlobalUnlock(hTermBuf);
    GlobalUnlock(hAttrBuf);

    InvalidateRect(hTermWnd, NULL, FALSE);
    bSuppress = TRUE;

/*    UpdateWindow(hTermWnd); */
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC LocFillScrn(LPRECT lpRect, BYTE cChar)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    LPVOID  fpTB;
    int     x, y;
    RECT    Rect;

    if (bReview)
        return;

    if (lpRect == NULL)
        SetRect(&Rect, 0, 0, TERMCOLS - 1, TERMROWS - 1);
    else
        Rect = *lpRect;

    if (Rect.top < 0 || Rect.left < 0 ||
        Rect.bottom >= TERMROWS || Rect.right >= TERMCOLS)
        return;

    fpTB = GlobalLock(hTermBuf);

#ifdef _WIN32
    for (y = (int)Rect.top; y <= (int)Rect.bottom; y++) {
        for (x = (int)Rect.left; x <= (int)Rect.right; x++) {
            (*ScreenChar(fpTB, x, y)) = cChar;
        }
#else
    for (y = Rect.top; y <= Rect.bottom; y++) {
        for (x = Rect.left; x <= Rect.right; x++) {
            (*ScreenChar(fpTB, x, y)) = cChar;
        }
#endif
    }

    GlobalUnlock(hTermBuf);

    InvalidateRect(hTermWnd, NULL, FALSE);
    bSuppress = TRUE;

/*    UpdateWindow(hTermWnd); */
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC LocFillAttr(LPRECT lpRect, BYTE cAttr)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    LPVOID  fpAB;
    int     x, y;
    RECT    Rect;

    if (bReview)
        return;

    if (lpRect == NULL)
        SetRect(&Rect, 0, 0, TERMCOLS - 1, TERMROWS - 1);
    else
        Rect = *lpRect;

    if (Rect.top < 0 || Rect.left < 0 ||
        Rect.bottom >= TERMROWS || Rect.right >= TERMCOLS)
        return;

    fpAB = GlobalLock(hAttrBuf);

#ifdef _WIN32
    for (y = (int)Rect.top; y <= (int)Rect.bottom; y++) {
        for (x = (int)Rect.left; x <= (int)Rect.right; x++) {
            (*ScreenChar(fpAB, x, y)) = cAttr;
        }
#else
    for (y = Rect.top; y <= Rect.bottom; y++) {
        for (x = Rect.left; x <= Rect.right; x++) {
            (*ScreenChar(fpAB, x, y)) = cAttr;
        }
#endif
    }

    GlobalUnlock(hAttrBuf);

    InvalidateRect(hTermWnd, NULL, FALSE);
    bSuppress = TRUE;

/*    UpdateWindow(hTermWnd); */
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC LocSetCurPos(int nCol, int nRow, BOOL bRel)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int xPrevOffset;
    int yPrevOffset;

    if (bRel) {
        nCurCol += nCol;
        nCurRow += nRow;
    }
    else {
        nCurCol = nCol;
        nCurRow = nRow;
    }

    if (!bSizeAvail)
        return;

    yPrevOffset = yOffset;

    if (nCurRow < yOffset)
        yOffset = nCurRow;
    if (nCurRow > (yOffset + VISROWS - 1))
        yOffset = (nCurRow - VISROWS + 1);

    if (yOffset < 0)
        yOffset = 0;
    if (yOffset > yScroll)
        yOffset = yScroll;

    if (yPrevOffset != yOffset) {
        SetScrollPos(hTermWnd, SB_VERT, yOffset, TRUE);
        InvalidateRect(hTermWnd, NULL, FALSE);
        bSuppress = TRUE;
    }

    xPrevOffset = xOffset;

    if (nCurCol < xOffset)
        xOffset = nCurCol;
    if (nCurCol > (xOffset + VISCOLS - 1))
        xOffset = (nCurCol - VISCOLS + 1);

    if (xOffset < 0)
        xOffset = 0;
    if (xOffset > xScroll)
        xOffset = xScroll;

    if (xPrevOffset != xOffset) {
        SetScrollPos(hTermWnd, SB_HORZ, xOffset, TRUE);
        InvalidateRect(hTermWnd, NULL, FALSE);
        bSuppress = TRUE;
    }

    UpdateTermCaret(nCurCol, nCurRow);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC LocGetCurPos(LPINT lpnCol, LPINT lpnRow)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    *lpnCol = nCurCol;
    *lpnRow = nCurRow;
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC LocDrawEmul(LPSTR lpszEmul)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    char szEmul[64];

    lstrcpy(szEmul, lpszEmul);
    SetStatus(WID_STEMUL, szEmul);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC LocSetAttr(BYTE cNewAttr)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    cCurAttr = cNewAttr;
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC LocSetTermSize(int nRows, int nCols)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int     nRowColSource;
    BOOL    bAdjFont;
    LPVOID  fpTB;
    LPVOID  fpAB;
    int     y;

    if ((VISROWS >= TERMROWS) && (VISCOLS >= TERMCOLS)) {
        bAdjFont = TRUE;
        nRowColSource = RCS_TERMSIZE;
    }
    else {
        bAdjFont = FALSE;
        nRowColSource = RCS_PREVWINSIZE;
    }

    if (nRows >= 0)
        nTermRows = nRows;

    if (nCols >= 0)
        nTermCols = nCols;

    lBufIndex = 0;

    nCurRow = 0;
    nCurCol = 0;

    bBuf = FALSE;

//    TermReview(FALSE);

    if (hTermBuf != NULL) {
        GlobalFree(hTermBuf);
        hTermBuf = NULL;
    }

    hTermBuf = GlobalAlloc(GHND, BUFSIZE);
    fpTB = GlobalLock(hTermBuf);
    for (y = 0; y < BUFROWS; y++) {
        _fmemset(ScreenChar(fpTB, 0, y), DEFCHAR, TERMCOLS);
        _fmemset(ScreenChar(fpTB, -2, y), '\0', 2);
    }
    GlobalUnlock(hTermBuf);

    if (hAttrBuf != NULL) {
        GlobalFree(hAttrBuf);
        hAttrBuf = NULL;
    }

    hAttrBuf = GlobalAlloc(GHND, BUFSIZE);
    fpAB = GlobalLock(hAttrBuf);
    for (y = 0; y < BUFROWS; y++) {
        _fmemset(ScreenChar(fpAB, 0, y), DEFATTR, TERMCOLS);
        _fmemset(ScreenChar(fpAB, -2, y), '\0', 2);
    }
    GlobalUnlock(hAttrBuf);

    if (bSizeAvail)
//        AdjustTermSize(bAdjFont, nRowColSource);
        AdjustTermSize(FALSE, nRowColSource);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC LocSetLineAttr(int Row, BYTE cLineAttr)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    LPVOID  fpTB, fpAB;

    if (bReview)
        return;

    if (Row < 0 || Row >= TERMROWS)
        return;

    if (bBuf && (Row != yBuf))
        LocFlushScrnBuf();

    fpTB = GlobalLock(hTermBuf);
    fpAB = GlobalLock(hAttrBuf);

    *ScreenChar(fpTB, -2, Row) = cLineAttr;

    GlobalUnlock(hTermBuf);
    GlobalUnlock(hAttrBuf);

    /* update buffering to indicate entire row is pending redraw */
    xBufStart = 0;
    xBufEnd = TERMCOLS - 1;
    yBuf = Row;
    bBuf = TRUE;
}

/*----------------------------------------------------------------------------*/
BYTE PUBFUNC LocGetLineAttr(int Row)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    LPVOID  fpTB;
    BYTE    cLineAttr;

    fpTB = GlobalLock(hTermBuf);

    cLineAttr = *(ScreenChar(fpTB, -2, Row));

    GlobalUnlock(hTermBuf);

    return cLineAttr;
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC LocSetVideoMode(int nMode)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    switch (nMode) {
        case VM_NORMAL:
            _fmemcpy(&ColorMap, &StdColorMap, sizeof(ColorMap));
            break;

        case VM_REVERSE:
            _fmemcpy(&ColorMap, &RevColorMap, sizeof(ColorMap));
            break;
    }

    InvalidateRect(hTermWnd, NULL, FALSE);
    bSuppress = TRUE;
}

VOID LocKeyMapClear(VOID)
{
    nKeyMapEntries = 0;
}

BOOL LocKeyMapAdd(KEYMAP FAR * lpkm)
{
    if (nKeyMapEntries < MAXKEYMAPENTRIES)
        KeyMap[nKeyMapEntries++] = (*lpkm);
    else
        return FALSE;

    return TRUE;
}

#define GT_OK       0
#define GT_EOF      1
#define GT_INVALID  2

int KeyMapGetToken(PSTR szLine, PINT pnLine, PSTR szToken, int nMaxLen)
{
    int nToken;

    nToken = 0;

    /* skip shite space */
    for (; (szLine[*pnLine] <= ' '); (*pnLine)++)
        if (szLine[*pnLine] == '\0')
            return GT_EOF;

    /* pick up token */
    for (; (szLine[*pnLine] > ' '); (*pnLine)++)
        if (nToken < (nMaxLen - 1))
            szToken[nToken++] = szLine[*pnLine];

    szToken[nToken] = '\0';

    return GT_OK;
}

BOOL PRVFUNC KeyMapParseLine(int nLine, PSTR szLine, KEYMAPENTRY * pkme)
{
    char    szKeyCode[80];
    char    szKeyDef[80];
    int     nResult;
    int     i;
    int     nWork;
    int     nLineIdx;
    char    szWork[80];

    memset(pkme, 0, sizeof(KEYMAPENTRY));
    nLineIdx = 0;

    nResult = KeyMapGetToken(szLine, &nLineIdx, szKeyCode, sizeof(szKeyCode));

    switch (nResult) {
        case GT_OK:
            break;

        case GT_EOF:
            /* An empty line is OK */
            return FALSE;
            break;

        case GT_INVALID:
        default:
            KermitFmtMsgBox(MB_ICONEXCLAMATION | MB_OK,
                            "Invalid key name format @ line %d!", nLine);
            return FALSE;
            break;
    }

    nResult = KeyMapGetToken(szLine, &nLineIdx, szKeyDef, sizeof(szKeyDef));

    switch (nResult) {
        case GT_OK:
            break;

        case GT_EOF:
            KermitFmtMsgBox(MB_ICONEXCLAMATION | MB_OK,
                            "Definition for key %s missing @ line %d!",
                            (LPSTR)szKeyCode, nLine);
            return FALSE;
            break;

        case GT_INVALID:
        default:
            KermitFmtMsgBox(MB_ICONEXCLAMATION | MB_OK,
                            "Invalid key definition format @ line %d!", nLine);
            return FALSE;
            break;
    }

    #ifdef _WIN32
    #define strcmpi _stricmp
    #endif

    nWork = 0;
    for (i = 0; szKeyCode[i] != '\0'; i++) {
        switch(szKeyCode[i]) {
            case '+':
            case '-':
                szWork[nWork] = '\0';
                if (strcmpi(szWork, "Shift") == 0)
                    pkme->bShift = TRUE;
                else if (strcmpi(szWork, "Ctrl") == 0)
                    pkme->bControl = TRUE;
                else if (strcmpi(szWork, "Alt") == 0)
                    pkme->bAlternate = TRUE;
                else {
                    KermitFmtMsgBox(MB_ICONEXCLAMATION | MB_OK,
                                    "Invalid key modifier @ line %d!", nLine);
                    return FALSE;
                }
                nWork = 0;
                break;

            default:
                if (nWork < (sizeof(szWork) - 1))
                    szWork[nWork++] = szKeyCode[i];
                break;
        }
    }

    szWork[nWork] = '\0';
    for (i = 0; vkm[i].szKeyName != NULL; i++) {
        if (strcmpi(szWork, vkm[i].szKeyName) == 0) {
            pkme->vk = vkm[i].nKeyID;
            break;
        }
    }

    if (vkm[i].szKeyName == NULL) {
        KermitFmtMsgBox(MB_ICONEXCLAMATION | MB_OK,
                        "Unknown virtual key name %s @ line %d!",
                        (LPSTR)szWork, nLine);
        return FALSE;
    }

    lstrcpy(pkme->szKeyName, szKeyDef);
    return TRUE;
}

BOOL LocKeyMapParse(LPSTR szKeyMapFile)
{
    KEYMAPENTRY kme;
    int         FileHandle;
    int         nWorkLine;
    int         nLine;
    int         nRead;
    char        szWorkLine[80];
    char        WorkChar;

    GetModuleFileName(hAppInst, szWorkLine, sizeof(szWorkLine));
    *(strrchr(szWorkLine, '\\') + 1) = '\0';
    lstrcat(szWorkLine, szKeyMapFile);

//  WriteTermFmt("Setting key map from %s...", (LPSTR)szWorkLine);

    FileHandle = _lopen(szWorkLine, OF_READ);
    if (FileHandle == HFILE_ERROR) {
                char szError[64];
#ifdef _WIN32
                strerror_s(szError, sizeof(szError), GetLastError());
#else
                strcpy(szError, strerror(NULL));
#endif
        MessageBeep(0);
        KermitFmtMsgBox(MB_ICONEXCLAMATION | MB_OK,
                        "Can't open key map file '%s'\n%s",
                        (LPSTR)szWorkLine,
                        (LPSTR)szError);
        return(FALSE);
    }

    szWorkLine[0] = '\0';
    nWorkLine = 0;
    nLine = 0;
    for (;;) {
        nRead = _lread(FileHandle, &WorkChar, 1);
        if (nRead == HFILE_ERROR) {
                char szError[64];
#ifdef _WIN32
                strerror_s(szError, sizeof(szError), GetLastError());
#else
                strcpy(szError, strerror(NULL));
#endif
            MessageBeep(0);
            KermitFmtMsgBox(MB_ICONEXCLAMATION | MB_OK,
                            "Error reading key map file '%s'\n%s",
                            (LPSTR)szKeyMapFile,
                            (LPSTR)szError);
            break;
        }

        if ((nRead == 1) && (nWorkLine < (sizeof(szWorkLine) - 1)))
            szWorkLine[nWorkLine++] = WorkChar;

        if (WorkChar == '\n' || nRead == 0) {
            nLine++;
            szWorkLine[nWorkLine++] = '\0';

            if (szWorkLine[0] != '#') {
                if (KeyMapParseLine(nLine, szWorkLine, &kme)) {
                    if (!DoKeyMap(&kme)) {
                        KermitFmtMsgBox(MB_ICONEXCLAMATION | MB_OK,
                                        "Error mapping key %s @ line %d!",
                                        (LPSTR)kme.szKeyName, nLine);
                    }
                }
            }

            if (nRead == 0)
                break;

            szWorkLine[0] = '\0';
            nWorkLine = 0;
        }
    }

    _lclose(FileHandle);

    return TRUE;
}

/*******************************************************************************
*                                                                              *
* Terminal Window Procedure and Support Functions                              *
*                                                                              *
*******************************************************************************/

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC Term_OnCreate
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle kermit windows creation stuff.                                      */
/*                                                                            */
(HWND               hWnd,              /* Window that received the command.   */
 LPCREATESTRUCT     X(lpCreateStruct)) /* Create structure.                   */
/*----------------------------------------------------------------------------*/

{
    LPVOID  fpTB;
    LPVOID  fpAB;
    int     y;

    hTermWnd = hWnd;

    nTermRows = 25;
    nTermCols = 80;

    SetScrollRange(hTermWnd, SB_HORZ, 0, 0, FALSE);
    SetScrollPos(hTermWnd, SB_HORZ, 0, FALSE);

    SetScrollRange(hTermWnd, SB_VERT, 0, 0, FALSE);
    SetScrollPos(hTermWnd, SB_VERT, 0, FALSE);

    lBufIndex = 0;

    cCurAttr = DEFATTR;
    _fmemcpy(&ColorMap, &StdColorMap, sizeof(ColorMap));

    xTermSize = 0;
    yTermSize = 0;

    xScroll = 0;
    yScroll = 0;

    xOffset = 0;
    yOffset = 0;

    nCurRow = 0;
    nCurCol = 0;

    bSuppress = FALSE;
    bBuf = FALSE;

    bReview = FALSE;
    bReadLine = FALSE;
    bSelect = FALSE;
    bTrack = FALSE;

    hTermBuf = GlobalAlloc(GHND, BUFSIZE);
    fpTB = GlobalLock(hTermBuf);
    for (y = 0; y < BUFROWS; y++) {
        _fmemset(ScreenChar(fpTB, 0, y), DEFCHAR, TERMCOLS);
        _fmemset(ScreenChar(fpTB, -2, y), '\0', 2);
    }
    GlobalUnlock(hTermBuf);

    hAttrBuf = GlobalAlloc(GHND, BUFSIZE);
    fpAB = GlobalLock(hAttrBuf);
    for (y = 0; y < BUFROWS; y++) {
        _fmemset(ScreenChar(fpAB, 0, y), DEFATTR, TERMCOLS);
        _fmemset(ScreenChar(fpAB, -2, y), '\0', 2);
    }
    GlobalUnlock(hAttrBuf);

    memcpy(&TermSet, &TermDef, sizeof(TermSet));

    SetTermFont(FALSE);

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
void PRVFUNC Term_OnDestroy
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle kermit windows creation stuff.                                      */
/*                                                                            */
(HWND           X(hWnd))               /* Window that received the command.   */
/*----------------------------------------------------------------------------*/

{
    int i;

    for (i = 0; i < FONT_COUNT; i++)
        if (FontMap[i] != NULL) {
            DeleteObject(FontMap[i]);
            FontMap[i] = NULL;
        }

    if (hTermBuf != NULL) {
        GlobalFree(hTermBuf);
        hTermBuf = NULL;
    }

    if (hAttrBuf != NULL) {
        GlobalFree(hAttrBuf);
        hAttrBuf = NULL;
    }

    hTermWnd = NULL;
}

/*----------------------------------------------------------------------------*/
void PRVFUNC Term_OnSize
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle kermit window size change.                                          */
/*                                                                            */
(HWND     X(hWnd),   /* Window that received the command.                     */
 UINT     X(state),  /* Window state.                                         */
 int      cx,        /* New window width.                                     */
 int      X(cy))     /* New window height.                                    */
/*----------------------------------------------------------------------------*/

{
    static BOOL bFirstSize = TRUE;

    if (cx <= 0) {
        bSizeAvail = FALSE;
        return;
    }
    else
        bSizeAvail = TRUE;

    if (bSizeActive)
        return;

    if (!bAdjustSize)
        if (bFirstSize) {
            AdjustTermSize(FALSE, RCS_TERMSIZE);
            bFirstSize = FALSE;
        }
        else {
            if (bAppSizeActive) {
                SetTermFont(TRUE);
                ResetTermWindow(TRUE);
            }
            else {
                AdjustTermSize(TRUE, RCS_TERMSIZE);
            }
        }
}

/*----------------------------------------------------------------------------*/
void PRVFUNC Term_OnVScroll
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle vertical scroll command.                                            */
/*                                                                            */
(HWND   hWnd,        /* Window that received the command.                     */
 HWND   X(hWndCtl),  /* Handle to scroll window control.                      */
 UINT   code,        /* Scroll bar code.                                      */
 int    pos)         /* Current Scroll Bar position.                          */
/*----------------------------------------------------------------------------*/

{
    int yPrevOffset;

    yPrevOffset = yOffset;

    switch (code) {
        case SB_TOP:
            yOffset = 0;
            break;
        case SB_BOTTOM:
            yOffset = yScroll;
            break;
        case SB_PAGEUP:
            yOffset -= VISROWS;
            break;
        case SB_PAGEDOWN:
            yOffset += VISROWS;
            break;
        case SB_LINEUP:
            yOffset--;
            break;
        case SB_LINEDOWN:
            yOffset++;
            break;
        case SB_THUMBPOSITION:
            yOffset = pos;
            break;
        default:
            return;
    }

    if (yOffset < 0)
        yOffset = 0;
    if (yOffset > yScroll)
        yOffset = yScroll;

    if (yOffset != yPrevOffset) {
        SetScrollPos(hWnd, SB_VERT, yOffset, TRUE);
        InvalidateRect(hWnd, NULL, FALSE);
        bSuppress = TRUE;
    }
}

/*----------------------------------------------------------------------------*/
void PRVFUNC Term_OnHScroll
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle horizontal scroll command.                                          */
/*                                                                            */
(HWND   hWnd,        /* Window that received the command.                     */
 HWND   X(hWndCtl),  /* Handle to scroll window control.                      */
 UINT   code,        /* Scroll bar code.                                      */
 int    pos)         /* Current Scroll Bar position.                          */
/*----------------------------------------------------------------------------*/

{
    int xPrevOffset;

    xPrevOffset = xOffset;

    switch (code) {
        case SB_TOP:
            xOffset = 0;
            break;
        case SB_BOTTOM:
            xOffset = xScroll;
            break;
        case SB_PAGEUP:
            xOffset -= VISCOLS;
            break;
        case SB_PAGEDOWN:
            xOffset += VISCOLS;
            break;
        case SB_LINEUP:
            xOffset--;
            break;
        case SB_LINEDOWN:
            xOffset++;
            break;
        case SB_THUMBPOSITION:
            xOffset = pos;
            break;
        default:
            return;
    }

    if (xOffset < 0)
        xOffset = 0;
    if (xOffset > xScroll)
        xOffset = xScroll;

    if (xOffset != xPrevOffset) {
        SetScrollPos(hWnd, SB_HORZ, xOffset, TRUE);
        InvalidateRect(hWnd, NULL, FALSE);
        bSuppress = TRUE;
    }
}

/*----------------------------------------------------------------------------*/
void PRVFUNC Term_OnSetFocus
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle horizontal scroll command.                                          */
/*                                                                            */
(HWND   X(hWnd),           /* Window that received the command.               */
 HWND   X(hWndOldFocus))   /* Window with previous focus.                     */
/*----------------------------------------------------------------------------*/

{
    UpdateTermCaret(nCurCol, nCurRow);
}

/*----------------------------------------------------------------------------*/
void PRVFUNC Term_OnKillFocus
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle horizontal scroll command.                                          */
/*                                                                            */
(HWND   X(hWnd),           /* Window that received the command.               */
 HWND   X(hWndNewFocus))   /* Window about to get focus.                      */
/*----------------------------------------------------------------------------*/

{
    UpdateTermCaret(nCurCol, nCurRow);
}

/*----------------------------------------------------------------------------*/
void PRVFUNC Term_OnLButtonDown
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle left mouse button down.                                             */
/*                                                                            */
(HWND      hWnd,              /* Window that received the command.            */
 BOOL      X(fDoubleClick),   /* Double click?                                */
 int       x,                 /* Double click?                                */
 int       y,                 /* Double click?                                */
 UINT      keyFlags)          /* Key flags.                                   */
/*----------------------------------------------------------------------------*/

{
    if (bReview) {
        bSelect = TRUE;
        bTrack = TRUE;
        SetCapture(hWnd);

        DestX = COLFROMPT(x);
        if (DestX < 0)
            DestX = 0;
        if (DestX > (TERMCOLS - 1))
            DestX = (TERMCOLS - 1);

        DestY = ROWFROMPT(y);
        if (DestY < 0)
            DestY = 0;
        if (DestY > (CURROWS - 1))
            DestY = (CURROWS - 1);

        if (!(keyFlags & MK_SHIFT)) {
            OrgX = DestX;
            OrgY = DestY;
        }

        LocSetCurPos(DestX, DestY, FALSE);
        InvalidateRect(hWnd, NULL, FALSE);
    }
}

/*----------------------------------------------------------------------------*/
void PRVFUNC Term_OnLButtonUp
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle left mouse button down.                                             */
/*                                                                            */
(HWND      X(hWnd),           /* Window that received the command.            */
 int       X(x),              /* Double click?                                */
 int       X(y),              /* Double click?                                */
 UINT      X(keyFlags))       /* Key flags.                                   */
/*----------------------------------------------------------------------------*/

{
    if (bReview) {
        bTrack = FALSE;
        ReleaseCapture();
    }
}

/*----------------------------------------------------------------------------*/
void PRVFUNC Term_OnMouseMove
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle mouse movement.                                                     */
/*                                                                            */
(HWND      hWnd,                /* Window that received the command.          */
 int       x,                   /* Double click?                              */
 int       y,                   /* Double click?                              */
 UINT      X(keyFlags))         /* Key flags.                                 */
/*----------------------------------------------------------------------------*/

{
    if (bTrack) {
        int NewX, NewY;

        NewX = COLFROMPT(x);
        if (NewX < 0)
            NewX = 0;
        if (NewX > (TERMCOLS - 1))
            NewX = (TERMCOLS - 1);

        NewY = ROWFROMPT(y);
        if (NewY < 0)
            NewY = 0;
        if (NewY > (CURROWS - 1))
            NewY = (CURROWS - 1);

        if (NewX != DestX || NewY != DestY) {
            DestX = NewX;
            DestY = NewY;
            LocSetCurPos(DestX, DestY, FALSE);
            InvalidateRect(hWnd, NULL, FALSE);
        }
    }
}

/*----------------------------------------------------------------------------*/
void PRVFUNC Term_OnChar
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle mouse movement.                                                     */
/*                                                                            */
(HWND      X(hWnd),             /* Window that received the command.          */
 UINT      ch,                  /* Character                                  */
 int       X(cRepeat))          /* Repeat Count                               */
/*----------------------------------------------------------------------------*/

{
    if (bReview) {
        if (bTrack)
            return;

        if (ch == VK_ESCAPE) {
            TermReview(FALSE);
            return;
        }
    }
    else if (bReadLine) {
        if (ch == 13) {
            bEOL = TRUE;
            szReadLineBuf[nReadLineBuf++] = '\n';
            szReadLineBuf[nReadLineBuf] = '\0';
        }
        else if (ch == 27) {
            bEOL = TRUE;
            nReadLineBuf = 0;
            szReadLineBuf[0] = '\0';
        }
        else if (ch == 8) {
            if (nReadLineBuf > 0) {
                WriteTermFmt("\x8 \x8");
                szReadLineBuf[--nReadLineBuf] = '\0';
            }
        }
        else {
            WriteTermFmt("%c", ch);
            szReadLineBuf[nReadLineBuf++] = (char)ch;
            szReadLineBuf[nReadLineBuf] = '\0';
        }
    }
    else {
        ProcessTermChar((char)ch);
    }

    return;
}

BOOL ProcessEmulateKeyDown(HWND X(hWnd), UINT vk, BOOL X(fDown),
                           int X(cRepeat), UINT flags)
{
    int i;

    BOOL bShift = ((GetKeyState(VK_SHIFT) & 0x8000) != 0);
    BOOL bControl = ((GetKeyState(VK_CONTROL) & 0x8000) != 0);
    BOOL bAlternate = ((flags & KF_ALTDOWN) != 0);

#ifdef DEBUGVKM
    for (i = 0; vkm[i].szKeyName != NULL; i++) {
        if (vk == vkm[i].nKeyID) {
            WriteTermFmt("<%s>", (LPSTR)(vkm[i].szKeyName));
            break;
        }
    }

    if (vkm[i].szKeyName == NULL)
        WriteTermFmt("<%X>", vk);

    return FALSE;
#endif

    for (i = 0; i < nKeyMapEntries; i++) {
        if ((vk == KeyMap[i].vk) && (bShift == KeyMap[i].bShift) &&
            (bControl == KeyMap[i].bControl) && (bAlternate == KeyMap[i].bAlternate)) {
//          WriteTermFmt(km[i].szSendStr);
            ProcessTermKey(KeyMap[i].tk);
            bSuppCharMsg = TRUE;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL ProcessReviewKeyDown(HWND hWnd, UINT vk, BOOL X(fDown),
                          int X(cRepeat), UINT X(flags))
{
    int CurX, CurY;
    int NewX, NewY;

    if (bTrack)
        return(TRUE);

    LocGetCurPos(&CurX, &CurY);
    NewX = CurX;
    NewY = CurY;

    switch (vk) {
        case VK_LEFT:
            if (NewX > 0) NewX--;
            break;

        case VK_RIGHT:
            if (NewX < (TERMCOLS - 1)) NewX++;
            break;

        case VK_UP:
            if (NewY > 0) NewY--;
            break;

        case VK_DOWN:
            if (NewY < (CURROWS - 1)) NewY++;
            break;

        case VK_HOME:
            if (GetKeyState(VK_CONTROL) & 0x8000)
                NewY = 0;
            NewX = 0;
            break;

        case VK_END:
            if (GetKeyState(VK_CONTROL) & 0x8000)
                NewY = (CURROWS - 1);
            NewX = (CURCOLS - 1);
            break;

        case VK_PRIOR:
            yOffset -= VISROWS;
            if (yOffset < 0)
                yOffset = 0;
            SetScrollPos(hWnd, SB_VERT, yOffset, TRUE);
            InvalidateRect(hWnd, NULL, FALSE);
            bSuppress = TRUE;
            InvalidateRect(hWnd, NULL, FALSE);
            break;

        case VK_NEXT:
            yOffset += (yTermSize / yTermChar);
            if (yOffset > yScroll)
                yOffset = yScroll;
            SetScrollPos(hWnd, SB_VERT, yOffset, TRUE);
            InvalidateRect(hWnd, NULL, FALSE);
            bSuppress = TRUE;
            InvalidateRect(hWnd, NULL, FALSE);
    }

    if ((NewX != CurX) || (NewY != CurY)) {
        if (GetKeyState(VK_SHIFT) & 0x8000) {
            if (!bSelect) {
                bSelect = TRUE;
                OrgX = CurX;
                OrgY = CurY;
            }
            DestX = NewX;
            DestY = NewY;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        else {
            if (bSelect) {
                bSelect = FALSE;
                InvalidateRect(hWnd, NULL, FALSE);
            }
        }

        LocSetCurPos(NewX, NewY, FALSE);
    }

    return FALSE;
}

BOOL ProcessInteractKeyDown(HWND X(hWnd), UINT X(vk), BOOL X(fDown),
                            int X(cRepeat), UINT X(flags))
{
    return FALSE;
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC ProcessKeyDown
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle key down.                                                           */
/*                                                                            */
(HWND      hWnd,                /* Window that received the command.          */
 UINT      vk,                  /* Virtual Key Code                           */
 BOOL      fDown,               /* Key going down (versus up)                 */
 int       cRepeat,             /* Repeat Count                               */
 UINT      flags)               /* Key flags.                                 */
/*----------------------------------------------------------------------------*/

{
    if (bReview)
        return ProcessReviewKeyDown(hWnd, vk, fDown, cRepeat, flags);
    else if (bReadLine)
        return ProcessInteractKeyDown(hWnd, vk, fDown, cRepeat, flags);
    else
        return ProcessEmulateKeyDown(hWnd, vk, fDown, cRepeat, flags);

    return FALSE;
}

/*----------------------------------------------------------------------------*/
void PRVFUNC Term_OnKeyDown
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle key down.                                                           */
/*                                                                            */
(HWND      hWnd,                /* Window that received the command.          */
 UINT      vk,                  /* Virtual Key Code                           */
 BOOL      fDown,               /* Key going down (versus up)                 */
 int       cRepeat,             /* Repeat Count                               */
 UINT      flags)               /* Key flags.                                 */
/*----------------------------------------------------------------------------*/

{
    if (!ProcessKeyDown(hWnd, vk, fDown, cRepeat, flags))
        FORWARD_WM_KEYDOWN(hWnd, vk, cRepeat, flags, DefWindowProc);
}

/*----------------------------------------------------------------------------*/
void PRVFUNC Term_OnSysKeyDown
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle key down.                                                           */
/*                                                                            */
(HWND      hWnd,                /* Window that received the command.          */
 UINT      vk,                  /* Virtual Key Code                           */
 BOOL      fDown,               /* Key going down (versus up)                 */
 int       cRepeat,             /* Repeat Count                               */
 UINT      flags)               /* Key flags.                                 */
/*----------------------------------------------------------------------------*/

{
    if (!ProcessKeyDown(hWnd, vk, fDown, cRepeat, flags))
        FORWARD_WM_SYSKEYDOWN(hWnd, vk, cRepeat, flags, DefWindowProc);
}

/*----------------------------------------------------------------------------*/
void PRVFUNC Term_OnPaint
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Handle key down.                                                           */
/*                                                                            */
(HWND      hWnd)                /* Window that received the command.          */
/*----------------------------------------------------------------------------*/

{
    PAINTSTRUCT ps;
    HDC hDC;
    RECT Rect;
    LPVOID  fpTB, fpAB;
    int Row, BegRow, BegCol, EndRow, EndCol, StartRow, StopRow;
    HBRUSH hFillBrush;

    if (hTermBuf == NULL) {
        FORWARD_WM_PAINT(hWnd, DefWindowProc);
        return;
    }

    hDC = BeginPaint(hWnd, &ps);
    Rect = ps.rcPaint;

    fpTB = GlobalLock(hTermBuf);
    fpAB = GlobalLock(hAttrBuf);

    BegRow = min(OrgY, DestY);
    EndRow = max(OrgY, DestY);
    BegCol = min(OrgX, DestX);
    EndCol = max(OrgX, DestX);

    StartRow = ROWFROMPT(Rect.top);
    StopRow = min((ROWFROMPT(Rect.bottom) + 1), CURROWS);
    StopRow = min(StopRow, (StartRow + TERMROWS));

    for (Row = StartRow; Row < StopRow; Row++) {
        if (bSelect && (Row >= BegRow) && (Row <= EndRow)) {
            DrawRow(hDC, Row, 0, BegCol - 1, FALSE);
            DrawRow(hDC, Row, BegCol, EndCol, TRUE);
            DrawRow(hDC, Row, EndCol + 1, CURCOLS - 1, FALSE);
        }
        else
            DrawRow(hDC, Row, 0, CURCOLS - 1, FALSE);
    }

    GlobalUnlock(hTermBuf);
    GlobalUnlock(hAttrBuf);

//    hFillBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    hFillBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));

//    Rect.left = Rect.left;
    Rect.top = ((StopRow - yOffset) * yTermChar);
    Rect.right = ps.rcPaint.right;
    Rect.bottom = ps.rcPaint.bottom;

    FillRect(hDC, &Rect, hFillBrush);

    Rect.left = ((TERMCOLS - xOffset) * xTermChar);
    Rect.top = 0;

    FillRect(hDC, &Rect, hFillBrush);

    DeleteObject(hFillBrush);

    EndPaint(hWnd, &ps);

    bSuppress = FALSE;

    UpdateTermCaret(nCurCol, nCurRow);
}

/*----------------------------------------------------------------------------*/
LRESULT CALLBACK __export TermWndProc(HWND hWnd, UINT message,
                                      WPARAM wParam, LPARAM lParam)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    switch (message) {

        HANDLE_MSG(hWnd, WM_CREATE, Term_OnCreate);

        HANDLE_MSG(hWnd, WM_DESTROY, Term_OnDestroy);

        HANDLE_MSG(hWnd, WM_SIZE, Term_OnSize);

        HANDLE_MSG(hWnd, WM_VSCROLL, Term_OnVScroll);

        HANDLE_MSG(hWnd, WM_HSCROLL, Term_OnHScroll);

        HANDLE_MSG(hWnd, WM_SETFOCUS, Term_OnSetFocus);

        HANDLE_MSG(hWnd, WM_KILLFOCUS, Term_OnKillFocus);

        HANDLE_MSG(hWnd, WM_LBUTTONDOWN, Term_OnLButtonDown);

        HANDLE_MSG(hWnd, WM_LBUTTONUP, Term_OnLButtonUp);

        HANDLE_MSG(hWnd, WM_MOUSEMOVE, Term_OnMouseMove);

        HANDLE_MSG(hWnd, WM_CHAR, Term_OnChar);

        HANDLE_MSG(hWnd, WM_KEYDOWN, Term_OnKeyDown);

        HANDLE_MSG(hWnd, WM_SYSKEYDOWN, Term_OnSysKeyDown);

        HANDLE_MSG(hWnd, WM_PAINT, Term_OnPaint);

        default:
            return(DefWindowProc(hWnd, message, wParam, lParam));
    }

    return(0L);
}

/*******************************************************************************
*                                                                              *
* Global Terminal Functions (services internal requests)                       *
*                                                                              *
*******************************************************************************/

BOOL PUBFUNC TMInit(VOID)
{
    return(TRUE);
}

BOOL PUBFUNC TMLoad(PSTR pszModule, BOOL bReset)
{
    if (!bReset && strcmp(szCurModule, pszModule) == 0)
        return TRUE;

    lstrcpy(szCurModule, pszModule);

    return(TRUE);
}

BOOL PUBFUNC TMSetConfig(UINT wInfoSize, PSTR pszInfo)
{
    if (wInfoSize > sizeof(TermSet))
        return(FALSE);

    memcpy(&TermSet, &TermDef, sizeof(TermSet));
    memcpy(&TermSet, pszInfo, wInfoSize);

    SetTermFont(FALSE);
    AdjustTermSize(FALSE, RCS_CURWINSIZE);

    return(TRUE);
}

BOOL PUBFUNC TMGetConfig(UINT * pwInfoSize, PSTR pszInfo)
{
    *pwInfoSize = sizeof(TermSet);
    memcpy(pszInfo, &TermSet, sizeof(TermSet));

    return(TRUE);
}

BOOL PUBFUNC TMSetup(HINSTANCE hInst, HWND hWnd)
{
    if (ChooseTermFont(hInst, hWnd)) {
        bChanged = TRUE;
        SetTermFont(FALSE);
        AdjustTermSize(FALSE, RCS_TERMSIZE);
    }

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC TerminalInit(HINSTANCE hInstance)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    WNDCLASS WndClass;

    memset(&WndClass, 0, sizeof(WndClass));

    WndClass.lpszClassName        = szClsKermTerm;
    WndClass.lpfnWndProc          = TermWndProc;
    WndClass.hInstance            = hInstance;
    WndClass.hbrBackground        = (HBRUSH)(COLOR_APPWORKSPACE + 1);
    WndClass.hbrBackground        = NULL;
    WndClass.hCursor              = LoadCursor(NULL, IDC_ARROW);

    return(RegisterClass(&WndClass) != NULL);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC WriteTermStr(LPSTR TermStr, int StrLen, BOOL Raw)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    WriteTerm(TermStr, StrLen, Raw);

    UpdateTermCaret(nCurCol, nCurRow);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC WriteTermFmt(PSTR pszFmt, ...)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    char    sWork[512];
    va_list pArg;
    int     nLen;

    va_start(pArg, pszFmt);
    nLen = wvsprintf(sWork, pszFmt, pArg);
    va_end(pArg);

    WriteTermStr(sWork, nLen, FALSE);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC ProcessTermLine(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int  nLength;
    static char RXBuf[128];

    if (bReview)
        return;

    if ((nLength = ReadCommStr(RXBuf, sizeof(RXBuf))) > 0)
        WriteTermStr(RXBuf, nLength, FALSE);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC TermCBCopy()

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{

    int     Row, BegRow, EndRow, Col, BegCol, EndCol, nData;
    HANDLE  hData;
    LPVOID  lpData, fpTB;

    if (!bSelect)
        return;

    BegRow = min(OrgY, DestY);
    EndRow = max(OrgY, DestY);
    BegCol = min(OrgX, DestX);
    EndCol = max(OrgX, DestX);

    hData = GlobalAlloc(GHND, (EndCol - BegCol + 3) * (EndRow - BegRow + 1) + 1);
    if (hData == NULL)
        return;

    lpData = GlobalLock(hData);
    if (lpData == NULL) {
        GlobalFree(hData);
        return;
    }

    fpTB = GlobalLock(hTermBuf);

    nData = 0;
    for (Row = BegRow; Row <= EndRow; Row++) {

        /* If this line is double wide, simulate double wide text for the
           clipboard by interleaving space characters with real characters.
           If column is odd copy a space char, if even copy real character. */

        if (*(ScreenChar(fpTB, -2, Row)) != LA_NORMAL) {
            for (Col = BegCol; Col <= EndCol; Col++)
                if ((Col % 2) == 0)
                    ((LPSTR)lpData)[nData++] = (*ScreenChar(fpTB, Col / 2, Row));
                else
                    ((LPSTR)lpData)[nData++] = ' ';
        }
        else {
            for (Col = BegCol; Col <= EndCol; Col++)
                ((LPSTR)lpData)[nData++] = (*ScreenChar(fpTB, Col, Row));
        }
        while (nData > 0 && ((LPCSTR)lpData)[nData - 1] == ' ')
            nData--;
        ((LPSTR)lpData)[nData++] = '\r';
        ((LPSTR)lpData)[nData++] = '\n';
    }
    ((LPSTR)lpData)[nData] = '\0';

    GlobalUnlock(hTermBuf);
    GlobalUnlock(hData);

    if (OpenClipboard(hAppWnd)) {
        EmptyClipboard();
        SetClipboardData(CF_OEMTEXT, hData);
        CloseClipboard();
    }
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC TermCBPaste()

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    HANDLE  hData;
    LPVOID  lpData;

    if (!OpenClipboard(hAppWnd))
        return;

    hData = GetClipboardData(CF_OEMTEXT);
    if (hData == NULL) {
        CloseClipboard();
        return;
    }

    lpData = GlobalLock(hData);
    if (lpData == NULL) {
        CloseClipboard();
        return;
    }

    LocSendTermStr((LPSTR)lpData, lstrlen((LPCSTR)lpData));

    GlobalUnlock(hData);
    CloseClipboard();
}

BOOL PUBFUNC TermReviewCheck(BOOL bMsgPending)
{
    if (!bReview)
        return FALSE;

    if (!bMsgPending)
        WaitMessage();

    return TRUE;
}

VOID PUBFUNC TermReview(BOOL bActivate)
{
    /* A call to TermReview while review mode is already active
       means the user is toggling review mode off! */

    if (bReadLine || (bActivate == bReview))
        return;

    if (bActivate) {
        PauseComm(TRUE);
        bReview = TRUE;
        yOffsetSave = yOffset;
        yOffset += (BUFROWS - TERMROWS);
        CheckMenuItem(GetMenu(hAppWnd), IDM_REVIEW, MF_CHECKED);
        AdjustTermSize(FALSE, RCS_PREVWINSIZE);
        LocGetCurPos(&SaveX, &SaveY);
        LocSetCurPos(0, BUFROWS - TERMROWS, TRUE);
        SetMessage(ML_PROTOCOL, "Review Mode, Output Paused, <Esc> to Resume");
    }
    else {
        PauseComm(FALSE);
        if (bTrack) {
            bTrack = FALSE;
            ReleaseCapture();
        }

        if (bSelect) {
            bSelect = FALSE;
            InvalidateRect(hTermWnd, NULL, FALSE);
        }

        bReview = FALSE;
        yOffset = yOffsetSave;
        CheckMenuItem(GetMenu(hAppWnd), IDM_REVIEW, MF_UNCHECKED);
        LocSetCurPos(SaveX, SaveY, FALSE);
        AdjustTermSize(FALSE, RCS_PREVWINSIZE);
        SetMessage(ML_PROTOCOL, NULL);
    }
}

BOOL PUBFUNC TermReadLineCheck(VOID)
{
    if (bEOL)
        return FALSE;

    if (!GetQueueStatus(QS_ALLINPUT))
        WaitMessage();

    return TRUE;
}

char * PUBFUNC TermReadLine(VOID)
{
    szReadLineBuf[0] = '\0';
    nReadLineBuf = 0;

    /* A nested call means terminate readline */

    if (bReadLine) {
        bEOL = TRUE;
        return NULL;
    }

    TermReview(FALSE);

    bEOL = FALSE;
    bReadLine = TRUE;

    while (TermReadLineCheck() && !bAbort)
        MessagePump();

    if (bAbort) {
        nReadLineBuf = 0;
        szReadLineBuf[0] = '\0';
    }

    /* An empty string means we are indicating an abort to the
       caller, so the following just gives the user some feedback
       what happened.  This is helpful because otherwise there
       is no way to tell from the screen whether the user hit
       <Enter> to process the string  or <Esc> to abort. */

    /* Note that if the user presses <Enter> immediately after the
       prompt, we will still have a <LF> in the resulting string, so
       the empty string unambiguously means EOF. */

    if (szReadLineBuf[0] == '\0')
        WriteTermFmt("   *** Terminated ***");

    WriteTermFmt("\r\n");

    bReadLine = FALSE;
    return szReadLineBuf;
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC TermSizeEnd(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    AdjustTermSize(FALSE, RCS_CURWINSIZE);
}