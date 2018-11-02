/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMTXFC.H                                   **
**                                                                            **
**  Include file for all Kermit terminal modules.                             **
**                                                                            **
*******************************************************************************/

/* TYPEDEFS ------------------------------------------------------------------*/

typedef struct {
    BOOL bShift;
    BOOL bControl;
    BOOL bAlternate;
    UINT vk;
    char szKeyName[64];
} KEYMAPENTRY;

/* FUNCTION PROTOTYPES -------------------------------------------------------*/

VOID   DLLIMPORT PUBFUNC SetupTerm(VOID);
VOID   DLLIMPORT PUBFUNC WriteTerm(LPSTR, int, BOOL);
BOOL   DLLIMPORT PUBFUNC OpenTerm(HWND, LPSTR);
VOID   DLLIMPORT PUBFUNC CloseTerm(VOID);
BOOL   DLLIMPORT PUBFUNC GetTermConfig(LPSTR);
VOID   DLLIMPORT PUBFUNC ProcessTermChar(char);
VOID   DLLIMPORT PUBFUNC ProcessTermKey(UINT);
BOOL   DLLIMPORT PUBFUNC DoKeyMap(KEYMAPENTRY FAR *);
