/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**               KERMTAPI.H                   **
**                                                                            **
**  Include file for all Kermit terminal modules.                             **
**                                                                            **
*******************************************************************************/

/* CONSTANTS -----------------------------------------------------------------*/

#define APIPROCCNT     19

/* API FUNCTION PROTOTYPES ---------------------------------------------------*/

extern "C" VOID CALLBACK __export ApiSendTermChar(char);
extern "C" VOID CALLBACK __export ApiSendTermStr(LPSTR, int);
extern "C" VOID CALLBACK __export ApiFlushScrnBuf(VOID);
extern "C" VOID CALLBACK __export ApiWriteScrnChar(int, int, char);
extern "C" VOID CALLBACK __export ApiWriteScrnStr(int, int, LPSTR, int);
extern "C" VOID CALLBACK __export ApiScrollScrn(int, int, LPRECT);
extern "C" VOID CALLBACK __export ApiFillScrn(LPRECT, BYTE);
extern "C" VOID CALLBACK __export ApiFillAttr(LPRECT, BYTE);
extern "C" VOID CALLBACK __export ApiSetCurPos(int, int, BOOL);
extern "C" VOID CALLBACK __export ApiGetCurPos(LPINT, LPINT);
extern "C" VOID CALLBACK __export ApiDrawEmul(LPSTR);
extern "C" VOID CALLBACK __export ApiSetAttr(BYTE);
extern "C" VOID CALLBACK __export ApiSetTermSize(int, int);
extern "C" VOID CALLBACK __export ApiSetLineAttr(int, BYTE);
extern "C" BYTE CALLBACK __export ApiGetLineAttr(int);
extern "C" VOID CALLBACK __export ApiSetVideoMode(int);
extern "C" VOID CALLBACK __export ApiKeyMapClear(VOID);
extern "C" BOOL CALLBACK __export ApiKeyMapAdd(KEYMAP FAR *);
extern "C" BOOL CALLBACK __export ApiKeyMapParse(LPSTR);

VOID PUBFUNC InitApiXfc(HINSTANCE);
FARPROC FAR * PUBFUNC GetApiProcTable(VOID);
