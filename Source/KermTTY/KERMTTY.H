/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMTTY.H                                    **
**                                                                            **
**  Include file for KermTTY library module.                                  **
**                                                                            **
*******************************************************************************/

/* DEFINITIONS ---------------------------------------------------------------*/

#define STRICT

#define X(x)
#ifdef _WIN32
#define X32(x)
#define X16(x) x
#else
#define X32(x) x
#define X16(x)
#endif

#ifdef GLOBAL
    #define CLASS
    #define INIT(x) = x
#else
    #define CLASS extern
    #define INIT(x)
#endif

#ifdef _WIN32
#pragma warning(disable: 4201 4514 4702)
#endif

#define TERMROWS 25
#define TERMCOLS 80

#define LASTROW  (TERMROWS - 1)
#define LASTCOL  (TERMCOLS - 1)

/* INCLUDES ------------------------------------------------------------------*/

#define  _CFRONT_PASS_
#include <windows.h>
#include <windowsx.h>
#include "ttyver.h"
#include "kermemul.h"
#include "kermtty.rch"

/* STORAGE ------------------------------------------------------------------*/

CLASS char   szAppName [] INIT("KermTTY");
CLASS char   szAppTitle [] INIT(APP_TITLE);
CLASS char   szVersion [] INIT("Version " VER_DESC " (" BUILD_OS ") " BUILD_TYPE);
CLASS char   szBuild   [] INIT(REL_TYPE " Build " BLD_DESC ", " BUILD_DATE);
CLASS char   szCopyright [] INIT(COPYRIGHT);
CLASS char   szContact [] INIT(CONTACT);

/* Terminal Window Control */
#pragma pack(2)
typedef struct {
    WORD NewLine;           /* TRUE=Auto LF on receipt of CR */
    WORD LocalEcho;         /* TRUE=Echo keyboard to term screen */
    WORD LineWrap;          /* TRUE=CR/LF when full line on term */
} TermBlk;
#pragma pack()

typedef struct {
    FARPROC APIProcs[APIPROCCNT];

    TermBlk TermSet;

    HWND hAppWnd;
    int  nCurRow, nCurCol;
    int  nSavRow, nSavCol;
} TCB;

TermBlk TermDef = {FALSE, FALSE, TRUE};

HINSTANCE hLibrary;
TCB near *npTCB;
