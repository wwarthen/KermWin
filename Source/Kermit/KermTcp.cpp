/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMTCP.CPP                                  **
**                                                                            **
**  This module contains the functions to interface with the Windows          **
**  interupt driven communications driver.                                    **
**                                                                            **
*******************************************************************************/

/* DEFINES -------------------------------------------------------------------*/

#define SETTCPMSG(msg)    SetDlgItemText(hWndStat, IDD_CONNMSG, msg)
#define SETTCPSTAT(stat)  SetDlgItemText(hWndStat, IDD_CONNSTAT, stat)

#define SOCKET_MESSAGE  (WM_USER + 1)

#define TN_IAC  '\xFF'
#define TN_DONT '\xFE'
#define TN_DO   '\xFD'
#define TN_WONT '\xFC'
#define TN_WILL '\xFB'
#define TN_SB   '\xFA'
#define TN_GA   '\xF9'
#define TN_EL   '\xF8'
#define TN_EC   '\xF7'
#define TN_AYT  '\xF6'
#define TN_AO   '\xF5'
#define TN_IP   '\xF4'
#define TN_BRK  '\xF3'
#define TN_DM   '\xF2'
#define TN_NOP  '\xF1'
#define TN_SE   '\xF0'

#define TN_TERMTYPE '\x18'

#define TN_ECHO '\x01'
#define TN_SUPPRESS_GA '\x03'

#define TN_IS '\x00'
#define TN_SEND '\x01'

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include "kermtcp.h"
#include <winsock.h>

/* CONSTANTS -----------------------------------------------------------------*/

#define WSADLLPROCCNT           17

/* WINSOCK DLL FUNCTION TABLE INDEXES ----------------------------------------*/

#define nWSAStartup             0
#define nWSACleanup             1
#define nsocket                 2
#define nsetsockopt             3
#define nbind                   4
#define ngetservbyname          5
#define nhtons                  6
#define ninet_addr              7
#define ngethostbyname          8
#define nconnect                9
#define nWSAAsyncSelect         10
#define nclosesocket            11
#define nWSAGetLastError        12
#define nsend                   13
#define nrecv                   14
#define nWSASetBlockingHook     15
#define nWSACancelBlockingCall  16

/* WINSOCK DLL FUNCTION DEFINITIONS ------------------------------------------*/

typedef int     (CALLBACK *tagWSAStartup)(WORD, LPWSADATA);
typedef int     (CALLBACK *tagWSACleanup)(void);
typedef SOCKET  (CALLBACK *tagsocket)(int, int, int);
typedef int     (CALLBACK *tagsetsockopt)(SOCKET, int, int, const char FAR *, int);
typedef int     (CALLBACK *tagbind)(SOCKET, const struct sockaddr FAR *, int);
typedef struct servent FAR * (CALLBACK *taggetservbyname)(const char FAR *, const char FAR *);
typedef u_short (CALLBACK *taghtons)(u_short);
typedef unsigned long (CALLBACK *taginet_addr)(const char FAR *);
typedef struct hostent FAR *     (CALLBACK *taggethostbyname)(const char FAR *);
typedef int     (CALLBACK *tagconnect)(SOCKET, const struct sockaddr FAR *, int);
typedef int     (CALLBACK *tagWSAAsyncSelect)(SOCKET, HWND, u_int, long);
typedef int     (CALLBACK *tagclosesocket)(SOCKET);
typedef int     (CALLBACK *tagWSAGetLastError)(void);
typedef int     (CALLBACK *tagsend)(SOCKET, const char FAR *, int, int);
typedef int     (CALLBACK *tagrecv)(SOCKET, char FAR *, int, int);
typedef FARPROC (CALLBACK *tagWSASetBlockingHook)(FARPROC);
typedef int     (CALLBACK *tagWSACancelBlockingCall)(void);

/* WINSOCK DLL FUNCTION CALL MACROS ------------------------------------------*/

#define WSAStartup (*(tagWSAStartup)(WSAProcs[nWSAStartup]))
#define WSACleanup (*(tagWSACleanup)(WSAProcs[nWSACleanup]))
#define socket (*(tagsocket)(WSAProcs[nsocket]))
#define setsockopt (*(tagsetsockopt)(WSAProcs[nsetsockopt]))
#define bind (*(tagbind)(WSAProcs[nbind]))
#define getservbyname (*(taggetservbyname)(WSAProcs[ngetservbyname]))
#define htons (*(taghtons)(WSAProcs[nhtons]))
#define inet_addr (*(taginet_addr)(WSAProcs[ninet_addr]))
#define gethostbyname (*(taggethostbyname)(WSAProcs[ngethostbyname]))
#define connect (*(tagconnect)(WSAProcs[nconnect]))
#define WSAAsyncSelect (*(tagWSAAsyncSelect)(WSAProcs[nWSAAsyncSelect]))
#define closesocket (*(tagclosesocket)(WSAProcs[nclosesocket]))
#define WSAGetLastError (*(tagWSAGetLastError)(WSAProcs[nWSAGetLastError]))
#define send (*(tagsend)(WSAProcs[nsend]))
#define recv (*(tagrecv)(WSAProcs[nrecv]))
#define WSASetBlockingHook (*(tagWSASetBlockingHook)(WSAProcs[nWSASetBlockingHook]))
#define WSACancelBlockingCall (*(tagWSACancelBlockingCall)(WSAProcs[nWSACancelBlockingCall]))

/* LOCAL DATA ----------------------------------------------------------------*/

#pragma pack(2)
typedef struct {
    char    szHost [128];       /* Telnet host address/DNS Name */
    char    szPort [32];        /* Telnet port (or service name) */
    char    szTermType [32];    /* Description of terminal type in use */
    WORD    bSendCRLF;          /* Send CR as CR LF */
    WORD    bSupNeg;            /* Suppress Telnet negotiation */
} TcpipConnBlk;
#pragma pack()

static TcpipConnBlk     TcpipConnSet;

static TcpipConnBlk     TcpipConnDef = {"", "telnet", "vt100", FALSE, FALSE};

static BOOL bCommEcho = FALSE;

static HWND hCommNotifyWnd = NULL;

static HINSTANCE hWinSockLib = NULL;

static FARPROC lpTcpDlgProc = NULL;

static FARPROC lpTcpHookProc = NULL;

static FARPROC WSAProcs[WSADLLPROCCNT];
static char *  WSAFuncs[WSADLLPROCCNT] = {
    "WSAStartup",
    "WSACleanup",
    "socket",
    "setsockopt",
    "bind",
    "getservbyname",
    "htons",
    "inet_addr",
    "gethostbyname",
    "connect",
    "WSAAsyncSelect",
    "closesocket",
    "WSAGetLastError",
    "send",
    "recv",
    "WSASetBlockingHook",
    "WSACancelBlockingCall"};

static BOOL    bIAC;
static BOOL    bCommand;
static char    cCommand;
static BOOL    bSubcommand;
static char    cSubcommand;
static BOOL    bSubdata;
static int     nSubdata;
static char    szSubdata[64];

static SOCKET  cli_sock;

/*----------------------------------------------------------------------------*/
VOID PUBFUNC TcpDrawStComm(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    SetStatus(WID_STCOMM, "TCP/IP (Telnet)");
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC TcpShowErr(LPSTR lpszErrMsg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    char sErrText[80];

    if (hWinSockLib == NULL)
        return;

    if (!LoadString(hAppInst, WSAGetLastError() - WSABASEERR + IDS_WSAERRBASE,
                    sErrText, sizeof(sErrText)))
        lstrcpy(sErrText, "Message Text Unavailable");

    MessageBeep(0);
    KermitFmtMsgBox(MB_OK | MB_ICONASTERISK,
                    "%s\n\nWinSock Error #%d - %s",
                    (LPSTR)lpszErrMsg, WSAGetLastError(), (LPSTR)sErrText);
}

/*----------------------------------------------------------------------------*/
int PRVFUNC TcpWriteCmdStr(LPSTR lpsCommStr, int nStrLen)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    BOOL  bPrevCommEcho;
    int   nResult;

    bPrevCommEcho = bCommEcho;
    bCommEcho = FALSE;

    nResult = WriteCommStr(lpsCommStr, nStrLen);

    bCommEcho = bPrevCommEcho;

    return nResult;
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export TcpDlgProc(HWND hDlg, unsigned message,
                                  UINT wParam, LONG X(lParam))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDD_CANCEL:
                    MessageBeep(0);
                    SETTCPSTAT("Aborting...");
                    if (hWinSockLib != NULL)
                        WSACancelBlockingCall();
                    EndDialog(hDlg, TRUE);
                    break;

                default:
                    return(FALSE);
                    break;
            }
            break;

        default:
            return(FALSE);
            break;
    }

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC TcpStatus(BOOL bShow)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    static HWND    hWndXfrPrev = NULL;

    if (bShow) {
        if (hWndStat != NULL)
            return;

        hWndXfrPrev = hWndStat;
        lpTcpDlgProc = MakeProcInstance((FARPROC)TcpDlgProc, hAppInst);
        hWndStat = CreateDialog(hAppInst, "TcpDlgBox", hAppWnd,
                               (DLGPROC)lpTcpDlgProc);
        SETTCPMSG("TCP/IP Connection");
        SETTCPSTAT("Initializing...");
        ShowWindow(hWndStat, SW_SHOW);

        EnableWindow(hAppWnd, FALSE);
    }
    else {
        if (hWndStat == NULL)
            return;

        EnableWindow(hAppWnd, TRUE);
        DestroyWindow(hWndStat);
        hWndStat = hWndXfrPrev;
        X32(FreeProcInstance(lpTcpDlgProc);)
        lpTcpDlgProc = NULL;
    }
}


/*----------------------------------------------------------------------------*/
BOOL PUBFUNC TcpDisconnect(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int i;

    if (hWinSockLib == NULL)
        return FALSE;

    if (cli_sock != INVALID_SOCKET) {
        closesocket(cli_sock);
        cli_sock = INVALID_SOCKET;
    }

    if (hCommNotifyWnd != NULL) {
        DestroyWindow(hCommNotifyWnd);
        hCommNotifyWnd = NULL;
    }

    WSACleanup();

#ifdef _WIN32
    /* multitasking version requires time to complete cleanup activities!!! */
    /* otherwise ugly errors if DLL is immediately unloaded!!! */
    Sleep(1000);
#endif

    X(FreeProcInstance(lpTcpHookProc);)

    for (i = 0; i < WSADLLPROCCNT; i++)
        WSAProcs[i] = NULL;

    if (hWinSockLib != NULL) {
        FreeLibrary(hWinSockLib);
        hWinSockLib = NULL;
    }

    bCommBusy = FALSE;

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export TcpBlockingHook(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    MessagePump();

    return FALSE;
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC TcpConnect(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    static char PortStr[] = "COM?";
    struct sockaddr_in  srv_addr,cli_addr;
    LPSERVENT           srv_info;
    LPHOSTENT           host_info;
    WSADATA             wsaData;
    int                 PortNum;
    int                 oobval;
    int                 i;
    char                szMsgStr[120];

    bCommEcho = FALSE;

    if (lstrlen(TcpipConnSet.szHost) == 0) {
        KermitFmtMsgBox(MB_OK | MB_ICONASTERISK,
                        "Can't connect!\n\n"
                        "No Host specified in Communications Settings.");
        return FALSE;
    }

#ifdef _WIN32
    hWinSockLib = LoadLibrary("WSOCK32.DLL");
    if (hWinSockLib == NULL) {
#else
    hWinSockLib = LoadLibrary("WINSOCK.DLL");
    if (hWinSockLib < HINSTANCE_ERROR) {
#endif
        hWinSockLib = NULL;
        return(FALSE);
    }

    for (i = 0; i < WSADLLPROCCNT; i++) {
        WSAProcs[i] = GetProcAddress(hWinSockLib, WSAFuncs[i]);

        if (WSAProcs[i] == NULL) {
            KermitFmtMsgBox(MB_OK | MB_ICONASTERISK,
                            "Can't get ProcAddress of function %s in "
                            "WinSock library!", (LPSTR)WSAFuncs[i]);
            Disconnect();
            return FALSE;
        }
    }

    cli_sock = INVALID_SOCKET;

    if (WSAStartup(0x0101, &wsaData) != 0) {
        TcpShowErr("Startup Failed!");
        return FALSE;
    }

    lpTcpHookProc = MakeProcInstance((FARPROC)TcpBlockingHook, hAppInst);

    if (WSASetBlockingHook(lpTcpHookProc) == NULL)
        TcpShowErr("Could not establish WinSock Blocking Hook!");

    hCommNotifyWnd = CreateWindow("KermCommNotify", "Kermit Comm Notify",
                                  WS_CHILDWINDOW | WS_DISABLED,
                                  0, 0, 0, 0, hAppWnd, NULL, hAppInst, NULL);

    if (hCommNotifyWnd == NULL) {
        KermitFmtMsgBox(MB_OK | MB_ICONASTERISK,
                        "Can't create comm notification window!");
        Disconnect();
        return(FALSE);
    }

    cli_sock = socket(PF_INET, SOCK_STREAM, 0);

    if (cli_sock == INVALID_SOCKET) {
        TcpShowErr("Could not create socket!");
        Disconnect();
        return FALSE;
    }

    oobval = TRUE;
    if (setsockopt(cli_sock, SOL_SOCKET, SO_OOBINLINE,
                   (const char FAR *)&oobval, sizeof(int)) == SOCKET_ERROR)
        TcpShowErr("Could not set socket option OOBINLINE!");

    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = INADDR_ANY;
    cli_addr.sin_port=0;

    if (bind(cli_sock, (LPSOCKADDR)&cli_addr, sizeof(cli_addr)) == SOCKET_ERROR) {
        TcpShowErr("Could not bind socket!");
        return FALSE;
    }

    /* the connect function and some of the prep work can block, so
       set up the app window to show an hourglass and not accept input */

    TcpStatus(TRUE);
    wsprintf(szMsgStr, "Connecting to %s", (LPSTR)TcpipConnSet.szHost);
    SETTCPMSG(szMsgStr);

    /* to "connect" we need to build the srv_addr struct,
       first, setup the family (always AF_INET) */

    srv_addr.sin_family = AF_INET;

    /* second, setup the port, if it is a number that atoi can deal with,
       just convert the resulting number to network byte order */

    PortNum = atoi(TcpipConnSet.szPort);

    if (PortNum == 0) {
        SETTCPSTAT("Resolving port/service...");
        srv_info = getservbyname(TcpipConnSet.szPort, "tcp");
        if (srv_info == NULL) {
            TcpStatus(FALSE);
            TcpShowErr("Could not determine Telnet service port!");
            Disconnect();
            return FALSE;
        }
        srv_addr.sin_port = srv_info->s_port;
    }
    else
        srv_addr.sin_port = htons((u_short)PortNum);

    /* third, and finally, setup up the ip address, if the inet_addr() function
       can deal with it, it must be a simple dotted decimal address and we are
       done, otherwise try to look it up using gethostbyname, it that fails
       we are screwed. */

    srv_addr.sin_addr.s_addr = inet_addr(TcpipConnSet.szHost);

    if (srv_addr.sin_addr.s_addr == INADDR_NONE) {
        SETTCPSTAT("Resolving host name/address...");
        host_info = gethostbyname(TcpipConnSet.szHost);
        if (host_info == NULL) {
            TcpStatus(FALSE);
            TcpShowErr("Could not resolve host name/address!");
            Disconnect();
            return FALSE;
        }

        _fmemcpy(&(srv_addr.sin_addr), host_info->h_addr, host_info->h_length);
    }

    SETTCPSTAT("Establishing socket connection...");
    if (connect(cli_sock, (LPSOCKADDR)&srv_addr, sizeof(srv_addr)) == SOCKET_ERROR) {
        TcpStatus(FALSE);
        TcpShowErr("Could not connect socket!");
        Disconnect();
        return FALSE;
    }

    TcpStatus(FALSE);

    bIAC = FALSE;
    bCommand = FALSE;
    cCommand = '\0';
    bSubcommand = FALSE;
    cSubcommand = '\0';
    bSubdata = FALSE;
    nSubdata = 0;

    if (WSAAsyncSelect(cli_sock, hCommNotifyWnd, SOCKET_MESSAGE,
                      FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
        TcpShowErr("WSAAsyncSelect Failure!");
        Disconnect();
        return FALSE;
    }

    /* According to TELNET spec, we have to start by assuming the other side
       can't do anything (even echo), so we will do our own echoing to start.
       If the remote sends a WILL ECHO, we will respond with a DO ECHO and
       turn this silly option back off. */
    bCommEcho = TRUE;

    /* Immediately tell the remote DO SUPPRESS-GO-AHEAD */

    if (!TcpipConnSet.bSupNeg) {
        TcpWriteCmdStr("\xFF\xFD\x03", 3);     /* DO ECHO */
        TcpWriteCmdStr("\xFF\xFB\x18", 3);     /* WILL TERM-TYPE */
    }

    return TRUE;
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC TcpCheckCommStatus(UINT * pcbInQue, UINT * pcbOutQue)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (pcbInQue != NULL)
        *pcbInQue = 0;

    if (pcbOutQue != NULL)
        *pcbOutQue = 0;
}

/*----------------------------------------------------------------------------*/
int PUBFUNC TcpReadCommStr(LPSTR lpsDest, int nMaxChars)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int     recv_len, i, j;
    char    buf[1024], buf2[64];
    int     err;

    if (hWinSockLib == NULL)
        return 0;

    recv_len = recv(cli_sock, buf, nMaxChars, 0);

    if (recv_len == SOCKET_ERROR) {
        err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            WriteTermFmt("\r\n<recv-err:%u>\r\n", err);
            MessageBeep(0);
        }
        return 0;
    }

    bCommBusy = (recv_len == nMaxChars);

    DebDump(DL_INFO, buf, recv_len, "Received %i chars", recv_len);

    if (TcpipConnSet.bSupNeg) {
        _fmemcpy(lpsDest, buf, recv_len);
        return recv_len;
    }

    j = 0;

    for (i = 0; i < recv_len; i++) {
        if (buf[i] == TN_IAC)
            bIAC = TRUE;
        else if (bIAC) {
            if (buf[i] == TN_IAC) {
                lpsDest[j++] = buf[i];
            }
            else if ((buf[i] == TN_WILL) || (buf[i] == TN_DO)) {
                bCommand = TRUE;
                cCommand = buf[i];
            }
            else if (buf[i] == TN_SB)
                bSubcommand = TRUE;
            else if (buf[i] == TN_SE && bSubdata) {
                bSubdata = FALSE;
                if (cSubcommand == TN_TERMTYPE && nSubdata > 0 &&
                    szSubdata[0] == TN_SEND) {
                    buf2[0] = TN_IAC;
                    buf2[1] = TN_SB;
                    buf2[2] = TN_TERMTYPE;
                    buf2[3] = TN_IS;
                    TcpWriteCmdStr(buf2, 4);
                    TcpWriteCmdStr(TcpipConnSet.szTermType, 0);
                    buf2[1] = TN_SE;
                    TcpWriteCmdStr(buf2, 2);
                }
            }
            bIAC = FALSE;
        }
        else if (bCommand) {
            buf2[0] = TN_IAC;
            if ((cCommand == TN_WILL) && (buf[i] == TN_ECHO)) {
                buf2[1] = TN_DO;
                buf2[2] = TN_ECHO;
                buf2[3] = '\0';
                TcpWriteCmdStr(buf2, 3);
                bCommand = FALSE;
                cCommand = '\0';
                bCommEcho = FALSE;
                continue;
            }
            if ((cCommand == TN_WILL) && (buf[i] == TN_SUPPRESS_GA)) {
                bCommand = FALSE;
                cCommand = '\0';
                continue;
            }
            if ((cCommand == TN_DO) && (buf[i] == TN_TERMTYPE)) {
                bCommand = FALSE;
                cCommand = '\0';
                continue;
            }
            buf2[1] = ((cCommand == TN_WILL) ? TN_DONT : TN_WONT);
            buf2[2] = buf[i];
            buf2[3] = '\0';
            TcpWriteCmdStr(buf2, 3);
            bCommand = FALSE;
            cCommand = 0;
        }
        else if (bSubcommand) {
            cSubcommand = buf[i];
            bSubdata = TRUE;
            bSubcommand = FALSE;
            nSubdata = 0;
        }
        else if (bSubdata) {
            szSubdata[nSubdata++] = buf[i];
            if (nSubdata >= sizeof(szSubdata))
                bSubdata = FALSE;
        }
        else
            lpsDest[j++] = buf[i];
    }

    lpsDest[j] = '\0';
    return j;
}

/*----------------------------------------------------------------------------*/
int PUBFUNC TcpWriteCommStr(LPSTR lpsCommStr, int nStrLen)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int     send_len;
    int     nPos, nStart, nLen, nSend;

    if (hWinSockLib == NULL)
        return 0;

    DebDump(DL_INFO, lpsCommStr, nStrLen, "Writing %i chars", nStrLen);

    send_len = 0;

    if (nStrLen == 0)
        nStrLen = lstrlen(lpsCommStr);

    for (nPos = 0, nStart = 0, nLen = 0; nPos < nStrLen; nPos++) {
        if (lpsCommStr[nPos] == '\r') {
            if (bCommEcho)
                WriteTermStr(lpsCommStr + nStart, nLen + 1, FALSE);

            nSend = send(cli_sock, lpsCommStr + nStart, nLen + 1, 0);

            if (nSend == SOCKET_ERROR) {
                WriteTermFmt("\r\n<send-err:%u>\r\n", WSAGetLastError());
                return 0;
            }

            send_len += nSend;

            if (TcpipConnSet.bSendCRLF) {
                if (bCommEcho)
                    WriteTermStr("\n", 1, FALSE);

                nSend = send(cli_sock, "\n", 1, 0);

                if (nSend == SOCKET_ERROR) {
                    WriteTermFmt("\r\n<send-err:%u>\r\n", WSAGetLastError());
                    return 0;
                }
            }

            nStart = nPos + 1;
            nLen = 0;
        }
        else
            nLen++;
    }

    if (nLen > 0) {
        if (bCommEcho)
            WriteTermStr(lpsCommStr + nStart, nLen, FALSE);

        nSend = send(cli_sock, lpsCommStr + nStart, nLen, 0);

        if (nSend == SOCKET_ERROR) {
            WriteTermFmt("\r\n<send-err:%u>\r\n", WSAGetLastError());
            return 0;
        }

        send_len += nSend;
    }

    return send_len;
}

/*----------------------------------------------------------------------------*/
UINT PUBFUNC TcpGetCommInfo(int nIndex)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    switch (nIndex) {
        case GCI_BAUDRATE:
            return 19200;
            break;

        case GCI_PARITY:
            return NOPARITY;
            break;

        case GCI_DATABITS:
            return 8;
            break;

        case GCI_STOPBITS:
            return ONESTOPBIT;
            break;
    }

    return(0);
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC TcpConnDlgInit(HWND hDlg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    SetDlgItemText(hDlg, IDD_HOST, TcpipConnSet.szHost);
    SetDlgItemText(hDlg, IDD_PORT, TcpipConnSet.szPort);
    SetDlgItemText(hDlg, IDD_TERMTYPE, TcpipConnSet.szTermType);

    if (bConnected) {
        EnableWindow(GetDlgItem(hDlg, IDD_HOST), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDD_PORT), FALSE);
    }

    CheckDlgButton(hDlg, IDD_SENDCRLF, TcpipConnSet.bSendCRLF);
    CheckDlgButton(hDlg, IDD_SUPNEG, TcpipConnSet.bSupNeg);

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC TcpConnDlgTerm(HWND hDlg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    TcpipConnBlk TcpipConnWrk;

    memcpy(&TcpipConnWrk, &TcpipConnSet, sizeof(TcpipConnBlk));

    GetDlgItemText(hDlg, IDD_HOST, TcpipConnWrk.szHost,
                   sizeof(TcpipConnWrk.szHost));
    GetDlgItemText(hDlg, IDD_PORT, TcpipConnWrk.szPort,
                   sizeof(TcpipConnWrk.szPort));
    GetDlgItemText(hDlg, IDD_TERMTYPE, TcpipConnWrk.szTermType,
                   sizeof(TcpipConnWrk.szTermType));

    TcpipConnWrk.bSendCRLF = (WORD)IsDlgButtonChecked(hDlg, IDD_SENDCRLF);
    TcpipConnWrk.bSupNeg = (WORD)IsDlgButtonChecked(hDlg, IDD_SUPNEG);

    memcpy(&TcpipConnSet, &TcpipConnWrk, sizeof(TcpipConnBlk));

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export TcpConnDlgProc(HWND hDlg, unsigned message,
                                      UINT wParam, LONG X(lParam))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDD_OK:
                    if (!TcpConnDlgTerm(hDlg))
                        break;
                    EndDialog(hDlg, TRUE);
                    break;

                case IDD_CANCEL:
                    EndDialog(hDlg, FALSE);
                    break;

                default:
                    return(FALSE);
                    break;
            }
            break;

        case WM_INITDIALOG:
            return(TcpConnDlgInit(hDlg));

        default:
            return(FALSE);
    }
    return(TRUE);
}

LRESULT PUBFUNC TcpCommNotifyWndProc(HWND hWnd, UINT message,
                                     WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case SOCKET_MESSAGE:
            switch (WSAGETSELECTEVENT(lParam)) {
                case FD_READ:
                    bCommBusy = TRUE;
                    break;

                case FD_WRITE:
                    break;

                case FD_CLOSE:
                    DMCommEvent(CE_CLOSED);
                    break;
            }
            break;

        default:
            return(DefWindowProc(hWnd, message, wParam, lParam));
    }

    return(0L);
}

BOOL PUBFUNC CMTcpInit(VOID)
{
    memcpy(&TcpipConnSet, &TcpipConnDef, sizeof(TcpipConnSet));

    return TRUE;
}

BOOL PUBFUNC CMTcpLoad(VOID)
{
    memcpy(&TcpipConnSet, &TcpipConnDef, sizeof(TcpipConnSet));

    return TRUE;
}

BOOL PUBFUNC CMTcpSetConfig(UINT wInfoSize, PSTR pszInfo)
{
    if (wInfoSize > sizeof(TcpipConnSet))
        return(FALSE);

    memcpy(&TcpipConnSet, &TcpipConnDef, sizeof(TcpipConnSet));
    memcpy(&TcpipConnSet, pszInfo, wInfoSize);

    return TRUE;
}

BOOL PUBFUNC CMTcpGetConfig(UINT * pwInfoSize, PSTR pszInfo)
{
    *pwInfoSize = sizeof(TcpipConnSet);
    memcpy(pszInfo, &TcpipConnSet, sizeof(TcpipConnSet));

    return TRUE;
}

BOOL PUBFUNC CMTcpSetup(HINSTANCE hInst, HWND hWnd)
{
    if (GoDialogBox(hInst, "TcpipDlgBox", hWnd, (FARPROC)TcpConnDlgProc))
        bChanged = TRUE;

    return TRUE;
}