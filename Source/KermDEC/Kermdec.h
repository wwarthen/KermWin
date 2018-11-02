/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMDEC.H                                    **
**                                                                            **
**  Include file for KermDEC library module.                                  **
**                                                                            **
*******************************************************************************/

// DEFINITIONS ----------------------------------------------------------------

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

#define TERMROWS (npTCB->nTermRows)
#define TERMCOLS (npTCB->nTermCols)

#define LASTROW  (TERMROWS - 1)
#define LASTCOL  (TERMCOLS - 1)

#define MAXSEQ   80
#define MAXPARM  10

// INCLUDES -------------------------------------------------------------------

#define  _CFRONT_PASS_
#include <windows.h>
#include <windowsx.h>
#include "decver.h"
#include "kermemul.h"
#include "kermdec.rch"

// STORAGE -------------------------------------------------------------------

CLASS char   szAppName [] INIT("KermDEC");
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
    BOOL bEscape;
    BOOL bEscSeq;
    int  nParmCnt;
    char szEscSeq[MAXSEQ];
    int  nParm[MAXPARM];
    int  nCurRow, nCurCol;
    int  nSavRow, nSavCol;
    int  nTermRows, nTermCols;
    int  nTop, nBottom;
    BYTE fg, fgSav;
    BYTE bg, bgSav;
    BYTE bold, boldSav;
    BOOL ul, ulSav;
    BOOL inv, invSav;
    BOOL bIgnore;
    BOOL bCursor;
    BOOL bApplication;
    BOOL bVT52;
    BOOL bRelative;
    BOOL bInsert;
    BOOL bQFlag;
    BOOL bCharSelSeq;
    int  nCharSelSet, nCharSelSetSav;
    BOOL bPoundSeq;
    BOOL bDirectCursorSeq;
    int  nDirectCursorSeq;
    BYTE * pCharSetG0, * pCharSetG0Sav;
    BYTE * pCharSetG1, * pCharSetG1Sav;
    int  nCurCharSet, nCurCharSetSav;
    BOOL bTab[132];
} TCB;

TermBlk TermDef = {FALSE, FALSE, TRUE};

HINSTANCE hLibrary;
TCB near *npTCB;

char szIdentStr[] = "\x1B[?6c";
char szIdentStrVT52[] = "\x1B/Z";

/* Transparent Character Translation */

BYTE sTransparent[128] = {  0,   1,   2,   3,   4,   5,   6,   7,
                            8,   9,  10,  11,  12,  13,  14,  15,
                           16,  17,  18,  19,  20,  21,  22,  23,
                           24,  25,  26,  27,  28,  29,  30,  31,
                           32,  33,  34,  35,  36,  37,  38,  39,
                           40,  41,  42,  43,  44,  45,  46,  47,
                           48,  49,  50,  51,  52,  53,  54,  55,
                           56,  57,  58,  59,  60,  61,  62,  63,
                           64,  65,  66,  67,  68,  69,  70,  71,
                           72,  73,  74,  75,  76,  77,  78,  79,
                           80,  81,  82,  83,  84,  85,  86,  87,
                           88,  89,  90,  91,  92,  93,  94,  95,
                           96,  97,  98,  99, 100, 101, 102, 103,
                          104, 105, 106, 107, 108, 109, 110, 111,
                          112, 113, 114, 115, 116, 117, 118, 119,
                          120, 121, 122, 123, 124, 125, 126, 127};

/* UK-ASCII Character Translation */

BYTE sUKASCII[128] =     {  0,   1,   2,   3,   4,   5,   6,   7,
                            8,   9,  10,  11,  12,  13,  14,  15,
                           16,  17,  18,  19,  20,  21,  22,  23,
                           24,  25,  26,  27,  28,  29,  30,  31,
                           32,  33,  34, 156,  36,  37,  38,  39,
                           40,  41,  42,  43,  44,  45,  46,  47,
                           48,  49,  50,  51,  52,  53,  54,  55,
                           56,  57,  58,  59,  60,  61,  62,  63,
                           64,  65,  66,  67,  68,  69,  70,  71,
                           72,  73,  74,  75,  76,  77,  78,  79,
                           80,  81,  82,  83,  84,  85,  86,  87,
                           88,  89,  90,  91,  92,  93,  94,  95,
                           96,  97,  98,  99, 100, 101, 102, 103,
                          104, 105, 106, 107, 108, 109, 110, 111,
                          112, 113, 114, 115, 116, 117, 118, 119,
                          120, 121, 122, 123, 124, 125, 126, 127};

/* US-ASCII Character Translation */

BYTE sUSASCII[128] =     {  0,   1,   2,   3,   4,   5,   6,   7,
                            8,   9,  10,  11,  12,  13,  14,  15,
                           16,  17,  18,  19,  20,  21,  22,  23,
                           24,  25,  26,  27,  28,  29,  30,  31,
                           32,  33,  34,  35,  36,  37,  38,  39,
                           40,  41,  42,  43,  44,  45,  46,  47,
                           48,  49,  50,  51,  52,  53,  54,  55,
                           56,  57,  58,  59,  60,  61,  62,  63,
                           64,  65,  66,  67,  68,  69,  70,  71,
                           72,  73,  74,  75,  76,  77,  78,  79,
                           80,  81,  82,  83,  84,  85,  86,  87,
                           88,  89,  90,  91,  92,  93,  94,  95,
                           96,  97,  98,  99, 100, 101, 102, 103,
                          104, 105, 106, 107, 108, 109, 110, 111,
                          112, 113, 114, 115, 116, 117, 118, 119,
                          120, 121, 122, 123, 124, 125, 126, 127};

/* Linedraw Character Translation */

BYTE sLinedraw[128] =    {  0,   1,   2,   3,   4,   5,   6,   7,
                            8,   9,  10,  11,  12,  13,  14,  15,
                           16,  17,  18,  19,  20,  21,  22,  23,
                           24,  25,  26,  27,  28,  29,  30,  31,
                           32,  33,  34,  35,  36,  37,  38,  39,
                           40,  41,  42,  43,  44,  45,  46,  47,
                           48,  49,  50,  51,  52,  53,  54,  55,
                           56,  57,  58,  59,  60,  61,  62,  63,
                           64,  65,  66,  67,  68,  69,  70,  71,
                           72,  73,  74,  75,  76,  77,  78,  79,
                           80,  81,  82,  83,  84,  85,  86,  87,
                           88,  89,  90,  91,  92,  93,  94,  32,
                            4, 177,  26,  23,  27,  25, 248, 241,
                           21,  18, 217, 191, 218, 192, 197, 196,
                          196, 196, 196, 196, 195, 180, 193, 194,
                          179, 243, 242, 227, 157, 156, 250, 127};

/* National Character Translation */

BYTE sNational[128] =    {  0,   1,   2,   3,   4,   5,   6,   7,
                            8,   9,  10,  11,  12,  13,  14,  15,
                           16,  17,  18,  19,  20,  21,  22,  23,
                           24,  25,  26,  27,  28,  29,  30,  31,
                           32,  33,  34,  35,  36,  37,  38,  39,
                           40,  41,  42,  43,  44,  45,  46,  47,
                           48,  49,  50,  51,  52,  53,  54,  55,
                           56,  57,  58,  59,  60,  61,  62,  63,
                           64,  65,  66,  67,  68,  69,  70,  71,
                           72,  73,  74,  75,  76,  77,  78,  79,
                           80,  81,  82,  83,  84,  85,  86,  87,
                           88,  89,  90,  91,  92,  93,  94,  95,
                          128, 129, 130, 131, 132, 133, 134, 135,
                          136, 137, 138, 139, 140, 141, 142, 143,
                          144, 145, 146, 147, 148, 149, 150, 151,
                          152, 153, 154, 123, 124, 125, 126, 127};
