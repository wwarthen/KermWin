/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMEMUL.H                                   **
**                                                                            **
**  Include file for all Kermit terminal modules.                             **
**                                                                            **
*******************************************************************************/

/* CONSTANTS -----------------------------------------------------------------*/

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

#define APIPROCCNT      19

#ifdef _WIN32
#define __export
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __export
#endif

/* API FUNCTION TABLE INDEXES ------------------------------------------------*/

#define nSendTermChar  0
#define nSendTermStr   1
#define nFlushScrnBuf  2
#define nWriteScrnChar 3
#define nWriteScrnStr  4
#define nScrollScrn    5
#define nFillScrn      6
#define nFillAttr      7
#define nSetCursorPos  8
#define nGetCursorPos  9
#define nDrawEmul      10
#define nSetAttr       11
#define nSetTermSize   12
#define nSetLineAttr   13
#define nGetLineAttr   14
#define nSetVideoMode  15
#define nKeyMapClear   16
#define nKeyMapAdd     17
#define nKeyMapParse   18

/* API FUNCTION DEFINITIONS --------------------------------------------------*/

typedef struct {
    BOOL bShift;
    BOOL bControl;
    BOOL bAlternate;
    UINT vk;
    UINT tk;
} KEYMAP;

typedef struct {
    BOOL bShift;
    BOOL bControl;
    BOOL bAlternate;
    UINT vk;
    char szKeyName[64];
} KEYMAPENTRY;

typedef VOID (CALLBACK __export *tagSendTermChar)(char);
typedef VOID (CALLBACK __export *tagSendTermStr)(LPSTR, int);
typedef VOID (CALLBACK __export *tagFlushScrnBuf)(VOID);
typedef VOID (CALLBACK __export *tagWriteScrnChar)(int, int, char);
typedef VOID (CALLBACK __export *tagWriteScrnStr)(int, int, LPSTR, int);
typedef VOID (CALLBACK __export *tagScrollScrn)(int, int, LPRECT);
typedef VOID (CALLBACK __export *tagFillScrn)(LPRECT, BYTE);
typedef VOID (CALLBACK __export *tagFillAttr)(LPRECT, BYTE);
typedef VOID (CALLBACK __export *tagSetCurPos)(int, int, BOOL);
typedef VOID (CALLBACK __export *tagGetCurPos)(LPINT, LPINT);
typedef VOID (CALLBACK __export *tagDrawEmul)(LPSTR);
typedef VOID (CALLBACK __export *tagSetAttr)(BYTE);
typedef VOID (CALLBACK __export *tagSetTermSize)(int, int);
typedef VOID (CALLBACK __export *tagSetLineAttr)(int, BYTE);
typedef BYTE (CALLBACK __export *tagGetLineAttr)(int);
typedef VOID (CALLBACK __export *tagSetVideoMode)(int);
typedef VOID (CALLBACK __export *tagKeyMapClear)(VOID);
typedef BOOL (CALLBACK __export *tagKeyMapAdd)(KEYMAP FAR *);
typedef BOOL (CALLBACK __export *tagKeyMapParse)(LPSTR);

/* API FUNCTION CALL MACROS --------------------------------------------------*/

#define SendTermChar (*(tagSendTermChar)(npTCB->APIProcs[nSendTermChar]))
#define SendTermStr (*(tagSendTermStr)(npTCB->APIProcs[nSendTermStr]))
#define FlushScrnBuf (*(tagFlushScrnBuf)(npTCB->APIProcs[nFlushScrnBuf]))
#define WriteScrnChar (*(tagWriteScrnChar)(npTCB->APIProcs[nWriteScrnChar]))
#define WriteScrnStr (*(tagWriteScrnStr)(npTCB->APIProcs[nWriteScrnStr]))
#define ScrollScrn (*(tagScrollScrn)(npTCB->APIProcs[nScrollScrn]))
#define FillScrn (*(tagFillScrn)(npTCB->APIProcs[nFillScrn]))
#define FillAttr (*(tagFillAttr)(npTCB->APIProcs[nFillAttr]))
#define SetCurPos (*(tagSetCurPos)(npTCB->APIProcs[nSetCursorPos]))
#define GetCurPos (*(tagGetCurPos)(npTCB->APIProcs[nGetCursorPos]))
#define DrawEmul (*(tagDrawEmul)(npTCB->APIProcs[nDrawEmul]))
#define SetAttr (*(tagSetAttr)(npTCB->APIProcs[nSetAttr]))
#define SetTermSize (*(tagSetTermSize)(npTCB->APIProcs[nSetTermSize]))
#define SetLineAttr (*(tagSetLineAttr)(npTCB->APIProcs[nSetLineAttr]))
#define GetLineAttr (*(tagGetLineAttr)(npTCB->APIProcs[nGetLineAttr]))
#define SetVideoMode (*(tagSetVideoMode)(npTCB->APIProcs[nSetVideoMode]))
#define KeyMapClear (*(tagKeyMapClear)(npTCB->APIProcs[nKeyMapClear]))
#define KeyMapAdd (*(tagKeyMapAdd)(npTCB->APIProcs[nKeyMapAdd]))
#define KeyMapParse (*(tagKeyMapParse)(npTCB->APIProcs[nKeyMapParse]))

/* EMULATION DLL FUNCTION PROTOTYPES -----------------------------------------*/

extern "C" int    DLLEXPORT CALLBACK /* __export */ SetupTerm(HANDLE);
extern "C" VOID   DLLEXPORT CALLBACK /* __export */ WriteTerm(HANDLE, LPSTR, int, BOOL);
extern "C" HANDLE DLLEXPORT CALLBACK /* __export */ OpenTerm(HWND, LPSTR, FARPROC FAR *);
extern "C" VOID   DLLEXPORT CALLBACK /* __export */ CloseTerm(HANDLE);
extern "C" BOOL   DLLEXPORT CALLBACK /* __export */ GetTermConfig(HANDLE, LPSTR);
extern "C" VOID   DLLEXPORT CALLBACK /* __export */ ProcessTermChar(HANDLE, char);
extern "C" VOID   DLLEXPORT CALLBACK /* __export */ ProcessTermKey(HANDLE, UINT);
extern "C" BOOL   DLLEXPORT CALLBACK /* __export */ DoKeyMap(HANDLE, KEYMAPENTRY FAR *);