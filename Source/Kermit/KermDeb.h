/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMDEB.H                                    **
**                                                                            **
**  Include file for debugging.                                               **
**                                                                            **
*******************************************************************************/

/* DEFINITIONS ---------------------------------------------------------------*/

/* INCLUDES ------------------------------------------------------------------*/

/* MACRO DEFINITIONS ---------------------------------------------------------*/

#define DD_NONE     0       /* No debug destination available */
#define DD_MSGBOX   1       /* Debug output to message boxes (yuk!) */
#define DD_MONITOR  2       /* Debug output to debug monitor (OK, but scrolls off screen) */
#define DD_FILE     3       /* Debug output to file (OK, but not very interactive) */

#define DL_NONE     0       /* Don't show any debug messages */
#define DL_FATAL    4       /* Fatal messages only (program is aborting!) */
#define DL_CRITICAL 8       /* Critical msgs only (program is failing!) */
#define DL_ERROR    12      /* General error msgs (try to keep going!) */
#define DL_WARNING  16      /* Potential problem detected */
#define DL_INFO     20      /* General information */

/* VARIABLE DECLARATIONS -----------------------------------------------------*/

CLASS int       nDebugLevel;
CLASS int       nDebugDest;
CLASS OFSTRUCT  ofDebugFile;

/* FUNCTION PROTOTYPES -------------------------------------------------------*/

//PSTR Fmt(PSTR, ...);
//PSTR DebugFmt(PSTR, ...);
VOID DebMsg(int, char *, ...);
VOID DebugInit(HANDLE, HANDLE, LPSTR, int);
VOID DebugTerm(VOID);
VOID DebugAddMenu(VOID);
BOOL DebugMenuCmd(HWND, int);
VOID DebAssert(char *, int, int, char *);
VOID DebTrace(char *, int, int, PSTR, ...);
VOID DebDump(int, void far *, unsigned long, char *, ...);
