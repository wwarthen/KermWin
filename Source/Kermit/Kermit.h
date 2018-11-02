/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                                KERMIT.H                                    **
**                                                                            **
**  This is the primary include file for the application.  It contains        **
**  global definitions and variables.  It also contains global function       **
**  prototypes.                                                               **
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

#define PUBFUNC     FAR
#define PRVFUNC     static NEAR

#pragma warning(disable: 4201 4514 4702)

#ifdef _WIN32
#define __export
#define DLLIMPORT __declspec(dllexport)
#else
#define DLLIMPORT __export
#endif

/* INCLUDES ------------------------------------------------------------------*/

#define  _CFRONT_PASS_
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "kermver.h"
#include "kermres.h"

/* VERSION INFORMATION -------------------------------------------------------*/

/* DEBUG CONTROL -------------------------------------------------------------*/

#include "kermdeb.h"

#define ASSERT(exp) DebAssert(__FILE__, __LINE__, exp, #exp);
#define TRACE(n, msg) DebTrace(__FILE__, __LINE__, n, msg);

#define LIMITTEXT(hdlg, id, field) \
        (Edit_LimitText(GetDlgItem((hdlg), (id)), (sizeof(field) - 1)))

#define CHKBUTTON(hdlg, fid, lid, sel) \
        (CheckRadioButton((hdlg), (fid), (lid), (fid) + (sel)))

/* CHILD WINDOW IDENTIFIERS --------------------------------------------------*/

    /* Children of top level window                                           */
#define WID_STATUS   100    /* Status line window (com port, term type, etc.) */
#define WID_TERM     101    /* Terminal window                                */
#define WID_INFO     102    /* Information line window                        */

    /* Children of information line                                           */
#define WID_INFOMSG  100    /* Message window (dynamic menu help, etc.)       */
#define WID_INFOTIME 101    /* Time of day window                             */

    /* Children of status line window                                         */
#define WID_STCOMM   100    /* Communications status window                   */
#define WID_STPROT   101    /* Protocol status window                         */
#define WID_STEMUL   102    /* Emulation status window                        */
#define WID_STDEV    103    /* Device status window                           */
#define WID_STTIME   104    /* Time online status window                      */

    /* Status window message levels                                           */

#define ML_SESSION      0   /* Session Messages                               */
#define ML_SCRIPT       1   /* Script Messages                                */
#define ML_PROTOCOL     2   /* Protocol Messages                              */
#define ML_ERROR        3   /* Error Messages                                 */
#define ML_MENU         4   /* Menu Messages                                  */

    /* Windows Message extensions                                             */

#define WM_USER_INVOKEPENDING   (WM_USER + 0)

    /* Set Timer constants                                                    */

#define ST_RESET    0
#define ST_START    1
#define ST_STOP     2

    /* GetCommInfo constants                                                  */

#define GCI_BAUDRATE   0
#define GCI_PARITY     1
#define GCI_DATABITS   2
#define GCI_STOPBITS   3
#define GCI_CONNTYPE   4

    /* Connection Type constants                                              */

#define CT_ASYNC    1
#define CT_TCPIP    2

    /* FlushCommQueue constants                                               */

#define FCQ_ALLQUEUES  0
#define FCQ_TXQUEUE    1
#define FCQ_RXQUEUE    2

    /* Comm Event Constants                                                   */

#define CE_CLOSED 1

    /* Comm Disconnect Strategies                                             */

#define CD_VERIFY 1
#define CD_FORCE  2
#define CD_ABORT  3

    /* Close Action Requests                                                  */

#define CAR_NONE       0
#define CAR_QUIT       1
#define CAR_RESET      2
#define CAR_OPEN       3
#define CAR_DISCONNECT 4
#define CAR_DISCONNECTED 5

/* VARIABLE DECLARATIONS -----------------------------------------------------*/

typedef BOOL PUBFUNC CHKFUNC(VOID);
typedef BOOL WATCHPROC(DWORD dwElapsed, DWORD dwLimit);


typedef char FILNAM [16];
typedef char PATHNAM [256];

CLASS char   szAppName [] INIT("Kermit");
CLASS char   szAppTitle [] INIT(APP_TITLE);
CLASS char   szVersion [] INIT("Version " VER_DESC " (" BUILD_OS ") " BUILD_TYPE);
CLASS char   szBuild   [] INIT(REL_TYPE " Build " BLD_DESC ", " BUILD_DATE);
CLASS char   szCopyright [] INIT(COPYRIGHT);
CLASS char   szContact [] INIT(CONTACT);

CLASS char   szClsKermit [] INIT("Kermit");
CLASS char   szClsKermStatus [] INIT("KermStatus");
CLASS char   szClsKermMsg [] INIT("KermMsg");
CLASS char   szClsKermTerm [] INIT("KermTerm");
CLASS char   szClsKermText [] INIT("KermText");

CLASS struct {
    DWORD   dwMajor;
    DWORD   dwMinor;
    DWORD   dwBuild;
} OSVer;

CLASS struct {
    DWORD   dwMajor;
    DWORD   dwMinor;
    DWORD   dwMajorReq;
    DWORD   dwMinorReq;
} OLEVer;

CLASS char   szOLEStatus [80] INIT("OLE Unloaded");

CLASS BOOL bOleInit INIT(FALSE);


CLASS HINSTANCE hAppInst;          /* instance handle                         */
CLASS HWND      hAppWnd;           /* handle to the main window               */
CLASS HACCEL    hMenuAccel;        /* handle to standard accelerators         */
CLASS HACCEL    hXferAccel;        /* handle to file transfer aux accelerators*/

    /* Global display/text metrics                                            */
CLASS int    xSysChar;          /* system character width                     */
CLASS int    ySysChar;          /* system character height                    */
CLASS int    yMenu;             /* system menu height                         */
CLASS int    xBord;             /* standard side border width                 */
CLASS int    yBord;             /* standard top/bottom border width           */

CLASS HWND    hWndStat INIT(NULL);
CLASS FARPROC lpXfrDlgProc;

    /* System status/configuration stuff                                      */
CLASS BOOL bChanged;           /* TRUE when configuration has changed         */
CLASS BOOL bDevice;            /* TRUE when device manager active             */
CLASS BOOL bConnected;         /* TRUE when connected                         */
CLASS BOOL bKermit;            /* TRUE when kermit protocol active            */
CLASS BOOL bSelect;            /* TRUE when active selection                  */
CLASS BOOL bReview;            /* TRUE when text review mode active           */
CLASS BOOL bReadLine;          /* TRUE when read line mode active             */
CLASS BOOL bScript;            /* TRUE when scripting active                  */
CLASS BOOL bAbort;             /* TRUE when nested msg loop abort requested   */
CLASS BOOL bScriptAbort;       /* TRUE when we only want to abort script      */
CLASS BOOL bAbortConfirm;      /* TRUE when confirmation of above desired     */
CLASS BOOL bAppSizeActive;     /* TRUE when user is sizing app window         */
CLASS BOOL bCommBusy;          /* TRUE when comm driver has stuff to do       */
CLASS BOOL bSuppCharMsg;       /* TRUE to discard next WM_(SYS)CHAR msg       */
CLASS BOOL bAppVisible;        /* TRUE when when app window visible           */
// CLASS int  nAppBusy;           /* > 0 when hourglass for whole app            */
CLASS int  nCloseReq;          /* Close Action Request (CAR_?)                */
CLASS DWORD dwBaseThreadId;    /* ThreadId of appliation base thread          */


CLASS PATHNAM szSessFileName;  /* Full pathname of session file               */
CLASS PATHNAM szWorkFileName;  /* Full pathname of session file               */
CLASS PATHNAM szHelpFileName;  /* Full pathname of help file                  */

    /* OLE Automation Globals */
CLASS DWORD dwRegisterActiveObject INIT(0);
CLASS DWORD dwRegisterCF INIT(0);
CLASS void * g_pKermOA INIT(NULL);
CLASS void * g_pKermOAF INIT(NULL);
CLASS ITypeLib * g_pKermTypeLib INIT(NULL);


#pragma pack(2)
typedef struct {
    WORD    wMode;              /* 0=Direct, 1=Dial, 2=Answer                 */
    char    szPhoneNum [32];    /* Phone number for dialing                   */
    WORD    wRedials;           /* Number of redials to allow                 */
    WORD    wDialWait;          /* Seconds to wait between dialing attempts   */
    char    szScript [80];      /* Script file to "take" after connection     */
    char    szNetID [32];
    char    szUserID [32];
    char    szPassword [32];
} ConnBlk;
#pragma pack()

CLASS ConnBlk     ConnSet;

#pragma pack(2)
typedef struct {
    WORD SessionLogFlag;        /* TRUE is session log requested              */
    char SessionLogName [80];   /* session log filename                       */
    WORD PacketLogFlag;         /* TRUE if debug log requested                */
    char PacketLogName [80];    /* debug log filename                         */
    WORD TransLogFlag;          /* TRUE if transaction log requested          */
    char TransLogName [80];     /* transaction log filename                   */
} LogBlk;
#pragma pack()

CLASS LogBlk      LogSet;

/* FUNCTION PROTOTYPES -------------------------------------------------------*/

    /* Kermit */
BOOL PUBFUNC InitKermitClass(HINSTANCE);
BOOL PUBFUNC KermitPoll(VOID);
BOOL PUBFUNC MessagePump(VOID);

    /* KermInit */
BOOL PUBFUNC KermitInit(HINSTANCE, HINSTANCE, LPSTR, int);
void PUBFUNC GetSysValues(HWND);

    /* KermStat */
BOOL PUBFUNC StatusInit(HINSTANCE);
void PUBFUNC StatusUpdate(void);
void PUBFUNC SetStatus(int, LPSTR);
void PUBFUNC SetTimer(int nStatus);

    /* KermTXfc */
BOOL PUBFUNC EMInit(void);
BOOL PUBFUNC EMUnload(void);
BOOL PUBFUNC EMLoad(PSTR, BOOL);
BOOL PUBFUNC EMSetConfig(UINT, PSTR);
BOOL PUBFUNC EMGetConfig(UINT *, PSTR);
BOOL PUBFUNC EMSetup(HINSTANCE hInst, HWND hWnd);

    /* KermTerm */
BOOL PUBFUNC TMInit(void);
BOOL PUBFUNC TMLoad(PSTR, BOOL);
BOOL PUBFUNC TMSetConfig(UINT, PSTR);
BOOL PUBFUNC TMGetConfig(UINT *, PSTR);
BOOL PUBFUNC TMSetup(HINSTANCE hInst, HWND hWnd);
void PUBFUNC WriteTermStr(LPSTR, int, BOOL);
void PUBFUNC WriteTermFmt(PSTR, ...);
void PUBFUNC ProcessTermLine(void);
void PUBFUNC TermCBCopy(void);
void PUBFUNC TermCBPaste(void);
void PUBFUNC TermReview(BOOL);
char * PUBFUNC TermReadLine(VOID);
void PUBFUNC TermSizeEnd(void);
BOOL PUBFUNC TerminalInit(HINSTANCE);

    /* KermMsg */
BOOL PUBFUNC MessageInit(HINSTANCE);
void PUBFUNC MessageUpdate(void);
void PUBFUNC SetMessage(int, LPSTR);

    /* KermText */
BOOL PUBFUNC TextInit(HINSTANCE);
void PUBFUNC SetText(HWND, HDC, LPSTR);
int  PUBFUNC GetTextWidth(HDC, LPSTR);

    /* KermSess */
BOOL PUBFUNC ConfirmClose(void);
BOOL PUBFUNC CloseSess(void);
void PUBFUNC NewSess(void);
BOOL PUBFUNC OpenSess(LPSTR);
BOOL PUBFUNC SaveSess(BOOL);
VOID PUBFUNC SessOnConnect(VOID);
BOOL PUBFUNC SMInit(void);
BOOL PUBFUNC SMSetup(HINSTANCE, HWND);

    /* KermDev */
void PUBFUNC DMMonitor(void);
BOOL PUBFUNC DMConnect(void);
BOOL PUBFUNC DMDisconnect(int);
VOID PUBFUNC DMCommEvent(DWORD);
BOOL PUBFUNC DMInit(void);
BOOL PUBFUNC DMLoad(PSTR, BOOL);
BOOL PUBFUNC DMSetConfig(UINT, PSTR);
BOOL PUBFUNC DMGetConfig(UINT *, PSTR);
BOOL PUBFUNC DMSetup(HINSTANCE, HWND);
VOID PUBFUNC Pause(DWORD);
VOID PUBFUNC PauseMon(DWORD, WATCHPROC);
BOOL PUBFUNC Wait(PSTR, DWORD);
BOOL PUBFUNC WaitMon(PSTR, DWORD, WATCHPROC);

    /* KermDlg */
int  PUBFUNC GetRadioButton(HWND, int, int);
BOOL PUBFUNC GetValidDlgItemInt(HWND, UINT, BOOL, UINT *, UINT, UINT);
BOOL CALLBACK __export AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK __export SaveDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK __export OpenDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK __export SessDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK __export LoggingDlgProc(HWND, UINT, WPARAM, LPARAM);

    /* KermComm */
BOOL PUBFUNC CMInit(void);
BOOL PUBFUNC CMLoad(PSTR, BOOL);
BOOL PUBFUNC CMSetConfig(UINT, PSTR);
BOOL PUBFUNC CMGetConfig(UINT *, PSTR);
BOOL PUBFUNC CMSetup(HINSTANCE, HWND);
BOOL PUBFUNC Disconnect(void);
BOOL PUBFUNC Connect(void);
void PUBFUNC PauseComm(BOOL);
void PUBFUNC DebugComm(void);
int  PUBFUNC FlushCommQueue(int);
void PUBFUNC SendBreak(void);
void PUBFUNC SetCommDTRState(BOOL);
void PUBFUNC CheckCommStatus(UINT *, UINT *);
int  PUBFUNC ReadCommStr(LPSTR, int);
int  PUBFUNC WriteCommStr(LPSTR, int);
int  PUBFUNC WriteCommChar(char);
int  PUBFUNC WriteCommFmt(PSTR, ...);
UINT PUBFUNC GetCommInfo(int nIndex);
BOOL PUBFUNC CommInit(HINSTANCE hInstance);

    /* KermProt */
BOOL PUBFUNC PMInit(void);
BOOL PUBFUNC PMLoad(PSTR, BOOL);
BOOL PUBFUNC PMSetConfig(UINT, PSTR);
BOOL PUBFUNC PMGetConfig(UINT *, PSTR);
BOOL PUBFUNC PMSetup(HINSTANCE, HWND);
void PUBFUNC DoKermit(void);
BOOL PUBFUNC StartKermit(int);
BOOL PUBFUNC EndKermit(int);
BOOL PUBFUNC AbortXfer(BOOL);
void PUBFUNC KermitUserInt(UINT);
BOOL PUBFUNC KermitSend(void);
BOOL PUBFUNC KermitReceive(void);
BOOL PUBFUNC KermitServer(void);
BOOL PUBFUNC KermitGet(void);
BOOL PUBFUNC KermitHost(void);
BOOL PUBFUNC KermitGeneric(void);

    /* KermMisc */
int  PUBFUNC GoDialogBox(HINSTANCE, LPSTR, HWND, FARPROC);
HFILE PUBFUNC ErrOpenFile(LPSTR, LPOFSTRUCT, UINT);
int  PUBFUNC ErrDlgDirList(HWND, LPSTR, int, int, UINT);
int  PUBFUNC ErrDlgDirListComboBox(HWND, LPSTR, int, int, UINT);
HWND PUBFUNC CrtSubWnd (LPCSTR, DWORD, HWND, int, HINSTANCE);
HDWP PUBFUNC DefMovSubWnd(HDWP, HWND, int, int, int, int, int);
void PUBFUNC MoveChildWindow(HWND, int, int, int, int, int, BOOL);
HDWP PUBFUNC DeferChildWindowPos(HDWP, HWND, int, HWND, int, int, int, int, UINT);
void PUBFUNC SetKermitMenu(LPSTR);
int  PUBFUNC KermitFmtMsgBox(UINT, PSTR, ...);
// VOID PUBFUNC AppBusy(BOOL);
PSTR PUBFUNC StrCatDefExt(PSTR, PSTR);
DWORD PUBFUNC SysErrText(DWORD, LPSTR, DWORD);
BOOL PUBFUNC GetFileVerString(PSTR, PSTR);

    /* KermOLE */
void PUBFUNC GetCLSIDDesc(CLSID, PSTR);
BOOL LoadKermTypeLib(void);
ITypeInfo * GetKermTypeInfo(REFGUID);

    /* KermOA */
void DispInvokeHandler(void);

#ifdef AXSCRIPT
    /* KermAX */
PSTR PUBFUNC AXStatus(void);
BOOL PUBFUNC AXInit(void);
BOOL PUBFUNC AXTerm(void);
BOOL PUBFUNC AXScriptLoadFile(LPSTR lpszScriptFileName);
BOOL PUBFUNC AXScriptLoadImmed(LPSTR lpszScriptCode);
BOOL PUBFUNC AXScriptExec(LPSTR lpszScriptFunc);
VOID PUBFUNC AXScriptAbort(VOID);
BOOL PUBFUNC AXScriptDebugStart(void);
BOOL PUBFUNC AXScriptDebugBreak(void);

BOOL PUBFUNC AXScriptDialog(void);
BOOL PUBFUNC AXExecDialog(void);
#endif
