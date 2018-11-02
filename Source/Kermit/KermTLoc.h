/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**               KERMTLOC.H                   **
**                                                                            **
**  Include file for all Kermit terminal modules.                             **
**                                                                            **
*******************************************************************************/

/* CONSTANTS -----------------------------------------------------------------*/

#define TERMCOLS nTermCols
#define TERMROWS nTermRows
#define TERMSIZE (TERMROWS * (TERMCOLS + 2))
#define BUFROWS (int)(65536 / (TERMCOLS + 2))
#define BUFSIZE ((UINT)(BUFROWS * (TERMCOLS + 2)))
#define CURCOLS TERMCOLS
#define CURROWS ((int)(bReview ? BUFROWS : TERMROWS))
#define CURSIZE ((UINT)(bReview ? BUFSIZE : TERMSIZE))
#define VISCOLS (min((xTermSize / xTermChar), min(CURCOLS, TERMCOLS)))
#define VISROWS (min((yTermSize / yTermChar), min(CURROWS, TERMROWS)))

#define TM_EMULATE      1
#define TM_REVIEW       2
#define TM_INTERACT     3

/* TYPE DEFS -----------------------------------------------------------------*/

typedef struct {
    BOOL bShift;
    BOOL bControl;
    BOOL bAlternate;
    UINT vk;
    UINT tk;
//  char * szSendStr;
} KEYMAP;

/* FUNCTION PROTOTYPES -------------------------------------------------------*/

VOID PUBFUNC LocSendTermChar(char);
VOID PUBFUNC LocSendTermStr(LPSTR, int);
VOID PUBFUNC LocFlushScrnBuf(VOID);
VOID PUBFUNC LocWriteScrnStr(int, int, LPSTR, int);
VOID PUBFUNC LocWriteScrnChar(int, int, char);
VOID PUBFUNC LocScrollScrn(int, int, LPRECT);
VOID PUBFUNC LocFillScrn(LPRECT, BYTE);
VOID PUBFUNC LocFillAttr(LPRECT, BYTE);
VOID PUBFUNC LocSetCurPos(int, int, BOOL);
VOID PUBFUNC LocGetCurPos(LPINT, LPINT);
VOID PUBFUNC LocDrawEmul(LPSTR);
VOID PUBFUNC LocSetAttr(BYTE);
VOID PUBFUNC LocSetTermSize(int, int);
VOID PUBFUNC LocSetLineAttr(int, BYTE);
BYTE PUBFUNC LocGetLineAttr(int);
VOID PUBFUNC LocSetVideoMode(int);
VOID PUBFUNC LocKeyMapClear(VOID);
BOOL PUBFUNC LocKeyMapAdd(KEYMAP FAR *);
BOOL PUBFUNC LocKeyMapParse(LPSTR);

/* FUNCTION PROTOTYPES -------------------------------------------------------*/

CLASS int   nTermRows;      /* number of rows for the emulated display device */
CLASS int   nTermCols;      /* number of rows for the emulated display device */

CLASS char  szReadLineBuf[256];
CLASS int   nReadLineBuf;
CLASS int   bEOL;
