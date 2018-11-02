/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMASY.H                                    **
**                                                                            **
**  This module contains the functions to interface with the Windows          **
**  interupt driven communications driver.                                    **
**                                                                            **
*******************************************************************************/

/* DEFINES -------------------------------------------------------------------*/

/* INCLUDES ------------------------------------------------------------------*/

/* CONSTANTS -----------------------------------------------------------------*/

/* LOCAL DATA ----------------------------------------------------------------*/

VOID PUBFUNC AsyDrawStComm(VOID);
VOID PUBFUNC AsyDebugCommMsgBox(VOID);
BOOL PUBFUNC AsySetupComm(VOID);
BOOL PUBFUNC AsyDisconnect(VOID);
BOOL PUBFUNC AsyConnect(VOID);
VOID PUBFUNC AsyPauseComm(BOOL bPause);
int PUBFUNC AsyFlushCommQueue(int fnQueue);
VOID PUBFUNC AsySendBreak(VOID);
VOID PUBFUNC AsySetCommDTRState(BOOL bAssert);
VOID PUBFUNC AsyCheckCommStatus(UINT * pcbInQue, UINT * pcbOutQue);
int PUBFUNC AsyReadCommStr(LPSTR lpsDest, int nMaxChars);
int PUBFUNC AsyWriteCommStr(LPSTR lpsCommStr, int nStrLen);
UINT PUBFUNC AsyGetCommInfo(int nIndex);
LRESULT PUBFUNC AsyCommNotifyWndProc(HWND hWnd, UINT message,
                                     WPARAM wParam, LPARAM lParam);
BOOL PUBFUNC CMAsyInit(VOID);
BOOL PUBFUNC CMAsyLoad(VOID);
BOOL PUBFUNC CMAsySetConfig(UINT wInfoSize, PSTR pszInfo);
BOOL PUBFUNC CMAsyGetConfig(UINT * pwInfoSize, PSTR pszInfo);
BOOL PUBFUNC CMAsySetup(HINSTANCE hInst, HWND hWnd);