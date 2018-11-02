/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMTDEF.H                                   **
**                                                                            **
**  Include file for all Kermit terminal modules.                             **
**                                                                            **
*******************************************************************************/

/* FUNCTION PROTOTYPES -------------------------------------------------------*/

int     PUBFUNC DefSetupTerm(VOID);
VOID    PUBFUNC DefWriteTerm(LPSTR, int, BOOL);
BOOL    PUBFUNC DefOpenTerm(LPSTR, FARPROC FAR *);
VOID    PUBFUNC DefCloseTerm(VOID);
BOOL    PUBFUNC DefGetTermConfig(LPSTR);
VOID    PUBFUNC DefProcessTermChar(char);
VOID    PUBFUNC DefProcessTermKey(UINT);
BOOL    PUBFUNC DefDoKeyMap(KEYMAPENTRY FAR *);