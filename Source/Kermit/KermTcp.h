/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMTCP.H                                    **
**                                                                            **
**  This module contains the functions to interface with the Windows          **
**  interupt driven communications driver.                                    **
**                                                                            **
*******************************************************************************/

/* DEFINES -------------------------------------------------------------------*/

/* INCLUDES ------------------------------------------------------------------*/

/* CONSTANTS -----------------------------------------------------------------*/

/* LOCAL DATA ----------------------------------------------------------------*/

VOID PUBFUNC TcpDrawStComm(VOID);
BOOL PUBFUNC TcpDisconnect(VOID);
BOOL PUBFUNC TcpConnect(VOID);
VOID PUBFUNC TcpCheckCommStatus(UINT * pcbInQue, UINT * pcbOutQue);
int PUBFUNC TcpReadCommStr(LPSTR lpsDest, int nMaxChars);
int PUBFUNC TcpWriteCommStr(LPSTR lpsCommStr, int nStrLen);
UINT PUBFUNC TcpGetCommInfo(int nIndex);
LRESULT PUBFUNC TcpCommNotifyWndProc(HWND hWnd, UINT message,
                                     WPARAM wParam, LPARAM lParam);
BOOL PUBFUNC CMTcpInit(VOID);
BOOL PUBFUNC CMTcpLoad(VOID);
BOOL PUBFUNC CMTcpSetConfig(UINT wInfoSize, PSTR pszInfo);
BOOL PUBFUNC CMTcpGetConfig(UINT * pwInfoSize, PSTR pszInfo);
BOOL PUBFUNC CMTcpSetup(HINSTANCE hInst, HWND hWnd);