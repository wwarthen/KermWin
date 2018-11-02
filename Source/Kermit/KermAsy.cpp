/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMASY.C                                    **
**                                                                            **
**  This module contains the functions to interface with the Windows          **
**  interupt driven communications driver.                                    **
**                                                                            **
*******************************************************************************/

/* DEFINES -------------------------------------------------------------------*/

#define RXBUFSIZE (1024 * 8)  /* 8KB Receive Buffer */
#define TXBUFSIZE (1024 * 4)  /* 4KB Send Buffer */

#ifdef _WIN32
#define INVCOMDEV(id) (id == INVALID_HANDLE_VALUE)
#else
#define INVCOMDEV(id) (id < 0)
#endif

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include "kermasy.h"

/* CONSTANTS -----------------------------------------------------------------*/

/* LOCAL DATA ----------------------------------------------------------------*/

#pragma pack(2)
typedef struct {
    WORD CommPort;          /* 0=COM1:, 1=COM2:, etc. */
    WORD BaudRate;          /* 300, 1200, 2400, etc. (obsolete, see BigBaudRate) */
    WORD DataBits;          /* int Size: 4-8 valid */
    WORD Parity;            /* 0-4 = None,Odd,Even,Mark,Space */
    WORD StopBits;          /* Stop Bits: 0,1,2 = 1,1.5,2 */
    WORD FlowControl;       /* 0=None, 1=Xon/Xoff */
    WORD HandShake;         /* 0=None, 1=RTS/DTR */
    DWORD BigBaudRate;      /* New format to allow higher baud rates */
} AsyncConnBlk;
#pragma pack()

static AsyncConnBlk     acbAct;

static AsyncConnBlk     acbDef = {0, 0, 8, 0, 0, 0, 1, 9600};

#ifndef _WIN32

#define BAUDMAPSIZE 13

typedef struct
{
    DWORD       dwBaudRate;
    WORD        wBaudRate;
} BaudMapBlk;

static BaudMapBlk BaudMap [BAUDMAPSIZE] =
{
    {110,       CBR_110},
    {300,       CBR_300},
    {600,       CBR_600},
    {1200,      CBR_1200},
    {2400,      CBR_2400},
    {4800,      CBR_4800},
    {9600,      CBR_9600},
    {14400,     CBR_14400},
    {19200,     CBR_19200},
    {38400,     CBR_38400},
    {56000,     CBR_56000},
    {128000,    CBR_128000},
    {256000,    CBR_256000}
};
#endif

    /* Handle to open communications port                                     */
#ifdef _WIN32
static HANDLE idComDev = INVALID_HANDLE_VALUE;
static HANDLE hCommWatchThread = NULL;
static HANDLE hCommWriteThread = NULL;
static DWORD  dwCommWatchThreadID = 0;
static DWORD  dwCommWriteThreadID = 0;
static OVERLAPPED oRead;
static OVERLAPPED oWrite;
static HANDLE hEventPost = NULL;
static HANDLE hWritePost = NULL;
static CRITICAL_SECTION csWriteBuf;
static BOOL bWriteBuf = FALSE;
static BOOL bRunThreads = FALSE;
static int nWriteBuf = 0;
static char lpsWriteBuf[TXBUFSIZE];
#else
static int    idComDev = -1;
#endif

static HWND hCommNotifyWnd = NULL;

// we are going to fake the CN_EVENT notifications using another
// thread in Win32

#ifdef _WIN32
#define WM_COMMNOTIFY       0x0044

#define CN_RECEIVE          0x0001
#define CN_TRANSMIT         0x0002
#define CN_EVENT            0x0004
#endif

/*----------------------------------------------------------------------------*/
VOID PUBFUNC AsyDrawStComm(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    char szBuf[32];
    static char ParityMap[] = "NOEMS";
    static char * StopBitsMap[] = {"1", "1½", "2"};

    wsprintf(szBuf, "COM%i: %lu,%c,%u,%s",
             acbAct.CommPort + 1, acbAct.BigBaudRate, ParityMap[acbAct.Parity],
             acbAct.DataBits, (LPSTR)StopBitsMap[acbAct.StopBits]);

    SetStatus(WID_STCOMM, szBuf);
}

VOID PUBFUNC AsyDebugCommMsgBox(VOID)

/*  Description of what function does. */

/*  Param(s):   x...............Description. */

/*  Returns:    Result description. */

{
    DCB     dcb;
#ifndef _WIN32
    COMSTAT cs;
#endif
    char    MsgStr  [512];

#ifndef _WIN32
    char StatStr [64];
#endif

    if (INVCOMDEV(idComDev))
        return;

#ifdef _WIN32
    dcb.DCBlength = sizeof(dcb);
#endif

    GetCommState(idComDev, &dcb);
#ifndef _WIN32
    GetCommError(idComDev, &cs);
#endif

#ifdef _WIN32
    wsprintf(MsgStr, "Baudrate=%lu, "
                     "fBinary=%i, "
                     "fParity=%i, "
                     "fOutxCtsFlow=%i, "
                     "fOutxDsrFlow=%i, "
                     "fDtrControl=%i, "
                     "fDsrSensitivity=%i, "
                     "fTXContinueOnXoff=%i, "
                     "fOutX=%i, "
                     "fInX=%i, "
                     "fErrorChar=%i, "
                     "fNull=%i, "
                     "fRtsControl=%i, "
                     "fAbortOnError=%i, "
                     "XonLim=%i, "
                     "XoffLim=%i, "
                     "ByteSize=%i, "
                     "Parity=%i, "
                     "StopBits=%i, "
                     "XonChar=%i, "
                     "XoffChar=%i, "
                     "ErrorChar=%i, "
                     "EofChar=%i, "
                     "EvtChar=%i",
                     (DWORD)dcb.BaudRate,
                     (int)dcb.fBinary,
                     (int)dcb.fParity,
                     (int)dcb.fOutxCtsFlow,
                     (int)dcb.fOutxDsrFlow,
                     (int)dcb.fDtrControl,
                     (int)dcb.fDsrSensitivity,
                     (int)dcb.fTXContinueOnXoff,
                     (int)dcb.fOutX,
                     (int)dcb.fInX,
                     (int)dcb.fErrorChar,
                     (int)dcb.fNull,
                     (int)dcb.fRtsControl,
                     (int)dcb.fAbortOnError,
                     (int)dcb.XonLim,
                     (int)dcb.XoffLim,
                     (int)dcb.ByteSize,
                     (int)dcb.Parity,
                     (int)dcb.StopBits,
                     (int)dcb.XonChar,
                     (int)dcb.XoffChar,
                     (int)dcb.ErrorChar,
                     (int)dcb.EofChar,
                     (int)dcb.EvtChar);
#else
    StatStr[0] = '\0';
    if (cs.status & CSTF_CTSHOLD)
        strcat(StatStr, "CTSHOLD ");
    if (cs.status & CSTF_DSRHOLD)
        strcat(StatStr, "DSRHOLD ");
    if (cs.status & CSTF_RLSDHOLD)
        strcat(StatStr, "RLSDHOLD ");
    if (cs.status & CSTF_XOFFHOLD)
        strcat(StatStr, "XOFFHOLD ");
    if (cs.status & CSTF_XOFFSENT)
        strcat(StatStr, "XOFFSENT ");
    if (cs.status & CSTF_EOF)
        strcat(StatStr, "EOF ");
    if (cs.status & CSTF_TXIM)
        strcat(StatStr, "TXIM ");

    wsprintf(MsgStr, "BaudRate=%u, "
                     "ByteSize=%i, "
                     "Parity=%i, "
                     "StopBits=%i, "
                     "RlsTimeout=%i, "
                     "CtsTimeout=%i, "
                     "DsrTimeout=%i, "
                     "fBinary=%i, "
                     "fRtsDisable=%i, "
                     "fParity=%i, "
                     "fOutxCtsFlow=%i, "
                     "fOutxDsrFlow=%i, "
                     "fDtrDisable=%i, "
                     "fOutX=%i, "
                     "fInX=%i, "
                     "PeChar=%i, "
                     "fNull=%i, "
                     "fChEvt=%i, "
                     "fDtrflow=%i, "
                     "fRtsflow=%i, "
                     "XonChar=%i, "
                     "XoffChar=%i, "
                     "XonLim=%i, "
                     "XoffLim=%i, "
                     "PeChar=%i, "
                     "EvtChar=%i, "
                     "EofChar=%i, "
                     "TxDelay=%i\n\n"
                     "Status=%s, "
                     "cbInQue=%i, "
                     "cbOutQue=%i",
             dcb.BaudRate,
             dcb.ByteSize,
             dcb.Parity,
             dcb.StopBits,
             dcb.RlsTimeout,
             dcb.CtsTimeout,
             dcb.DsrTimeout,
             dcb.fBinary,
             dcb.fRtsDisable,
             dcb.fParity,
             dcb.fOutxCtsFlow,
             dcb.fOutxDsrFlow,
             dcb.fDtrDisable,
             dcb.fOutX,
             dcb.fInX,
             dcb.PeChar,
             dcb.fNull,
             dcb.fChEvt,
             dcb.fDtrflow,
             dcb.fRtsflow,
             dcb.XonChar,
             dcb.XoffChar,
             dcb.XonLim,
             dcb.XoffLim,
             dcb.PeChar,
             dcb.EvtChar,
             dcb.EofChar,
             dcb.TxDelay,
             (LPSTR)StatStr,
             cs.cbInQue,
             cs.cbOutQue);
#endif
    MessageBox(hAppWnd, MsgStr, "Debug: Communications DCB", MB_OK);
}

#ifdef _WIN32
/*----------------------------------------------------------------------------*/
VOID PRVFUNC AsyShowErr(LPSTR lpszDesc, DWORD dwError, int nPort)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    char szMsgBuf[96];

    SysErrText(dwError, szMsgBuf, sizeof(szMsgBuf));

    KermitFmtMsgBox(MB_OK | MB_ICONEXCLAMATION,
                    "%s [COM%i:]\n%s (#%lu)",
                    (LPSTR)lpszDesc, nPort + 1,
                    (LPSTR)szMsgBuf, dwError);
}
#else
/*----------------------------------------------------------------------------*/
VOID PRVFUNC AsyShowErr(LPSTR lpszDesc, int nResult, int nPort)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    char sErrText[80];

    if (!LoadString(hAppInst, IDS_COMMERRBASE - nResult,
                    sErrText, sizeof(sErrText)))
        LoadString(hAppInst, IDS_COMMERRBASE, sErrText, sizeof(sErrText));

    MessageBeep(0);
    KermitFmtMsgBox(MB_OK | MB_ICONEXCLAMATION,
                    "%s [COM%i:]\n%s (#%i)",
                    (LPSTR)lpszDesc, nPort + 1,
                    (LPSTR)sErrText, nResult);
}
#endif

#ifdef _WIN32

DWORD PUBFUNC AsyCommWriteProc(LPSTR X(lpData))
{
    BOOL    fWriteStat;
    DWORD   dwBytesWritten;
    int     nBytesToWrite;

    DebMsg(DL_INFO, "Write process thread started");

    while (bRunThreads)
    {
        WaitForSingleObject(hWritePost, 500);

        EnterCriticalSection(&csWriteBuf);

        if (nWriteBuf == 0)
            LeaveCriticalSection(&csWriteBuf);
        else
        {
            DebDump(DL_INFO, lpsWriteBuf, nWriteBuf, "Writing %i chars", nWriteBuf);

            nBytesToWrite = nWriteBuf;
            fWriteStat = WriteFile(idComDev, lpsWriteBuf, nBytesToWrite,
                                   &dwBytesWritten, &oWrite);
            nWriteBuf = 0;
            LeaveCriticalSection(&csWriteBuf);

            if (fWriteStat)
            {
                DebMsg(DL_INFO, "Bytes written: %lu", dwBytesWritten);
            }
            else
            {
                DWORD dwLastError;

                dwLastError = GetLastError();

                /* Apparently, we will get an ERROR_IO_PENDING here on every   */
                /* call because the driver doesn't wait to finish sending the  */
                /* output data to return.  I have elected not to wait for the  */
                /* write to complete because it locks up the app until it is   */
                /* done for no good reason.  Not much we can do if the I/O     */
                /* fails anyway!                                               */

                if (dwLastError == ERROR_IO_PENDING)
                {
                    DebMsg(DL_INFO, "Write I/O Pending, using GetOverlappedResult()");
                    if (GetOverlappedResult(idComDev, &oWrite,
                                             &dwBytesWritten, TRUE))
                    {
                        DebMsg(DL_INFO, "Bytes written: %lu", dwBytesWritten);
                    }
                    else
                    {
                        DWORD dwLastError2;

                        dwLastError2 = GetLastError();
                        DebMsg(DL_INFO, "Write GetOverlappedResult() error 0x%lX", dwLastError2);
                    }
                }
                else
                    DebMsg(DL_INFO, "Write error 0x%lX", dwLastError);
            }
        }
    }

    DebMsg(DL_INFO, "Write process thread terminating");

    return TRUE;
}

DWORD PUBFUNC AsyCommWatchProc(LPSTR X(lpData))
{
    DWORD       dwEvtMask;
    DWORD       dwTransfer;
    OVERLAPPED  oWatch;

    DebMsg(DL_INFO, "Comm port monitor process thread started");

    oWatch.hEvent = CreateEvent(NULL,  /* no security attributes */
                                TRUE,  /* manual reset event */
                                FALSE, /* not signalled */
                                NULL); /* no name */

    while (bRunThreads)
    {
        DebMsg(DL_INFO, "WaitComm initiated");
        if (WaitCommEvent(idComDev, &dwEvtMask, &oWatch))
        {
            DebMsg(DL_INFO, "WaitComm completed OK");
        }
        else
        {
            if (GetLastError() == ERROR_IO_PENDING)
            {
                DebMsg(DL_INFO, "WaitComm I/O Pending, using GetOverlappedResult()");

                if (GetOverlappedResult(idComDev, &oWatch, &dwTransfer, TRUE))
                {
                    DebMsg(DL_INFO, "WaitComm GetOverlappedResult() completed OK");
                }
                else
                {
                    DWORD dwLastError;

                    dwLastError = GetLastError();
                    DebMsg(DL_INFO, "WaitComm GetOverlappedResult() error 0x%lX", dwLastError);
                }
            }
            else
                dwEvtMask = 0;
        }

        if (bRunThreads && (dwEvtMask & (EV_RXCHAR | EV_RLSD)))
        {
            ResetEvent(hEventPost);
            DebMsg(DL_INFO, "WaitComm posting message");
            PostMessage(hCommNotifyWnd, WM_COMMNOTIFY, (WPARAM)idComDev,
                        dwEvtMask);
            WaitForSingleObject(hEventPost, INFINITE);
        }
    }

    CloseHandle(oWatch.hEvent);

    DebMsg(DL_INFO, "Comm port monitor process thread terminating");

    return TRUE;
}

#endif

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC AsySetupComm(AsyncConnBlk * acb)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    DCB     dcb;

#ifndef _WIN32
    int Result;
#endif

    if (INVCOMDEV(idComDev))
        return FALSE;

    if (acbAct.CommPort != acb->CommPort)
    {
        KermitFmtMsgBox(MB_OK | MB_ICONEXCLAMATION,
                        "Can't switch to new comm port on active connection!");
        return FALSE;
    }

#ifdef _WIN32
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(idComDev, &dcb))
    {
        DWORD dwError;

        dwError = GetLastError();

        DebMsg(DL_INFO, "Error 0x%lX retrieving current comm settings", dwError);

        AsyShowErr("Error retrieving current comm settings",
                   dwError, acb->CommPort);

        return FALSE;
    }
#else
    if ((Result = GetCommState(idComDev, &dcb)) != 0)
    {
        DebMsg(DL_INFO, "Error 0x%X retrieving current comm settings", Result);

        AsyShowErr("Error retrieving current comm settings",
                   Result, acb->CommPort);
        return FALSE;
    }
#endif

#ifdef _WIN32
    dcb.BaudRate = acb->BigBaudRate;
#else
    {
        int i;
        WORD wBaudRate;

        wBaudRate = 0;
        for (i = 0; i < BAUDMAPSIZE; i++)
            if (acb->BigBaudRate == BaudMap[i].dwBaudRate)
            {
                wBaudRate = BaudMap[i].wBaudRate;
                break;
            }

        if (wBaudRate == 0)
        {
            if (acb->BigBaudRate <= 0xFFFF)
                wBaudRate = (WORD)acb->BigBaudRate;
            else
                wBaudRate = 57600;
        }

        dcb.BaudRate = wBaudRate;
    }

#endif

    dcb.ByteSize = (BYTE)acb->DataBits;
    dcb.Parity = (BYTE)acb->Parity;
    dcb.StopBits = (BYTE)acb->StopBits;
    dcb.XonChar = 17;
    dcb.XoffChar = 19;
    dcb.XonLim = RXBUFSIZE / 5;
    dcb.XoffLim = RXBUFSIZE / 5;
    dcb.fOutX = dcb.fInX = (BYTE)acb->FlowControl;
    dcb.fBinary = TRUE;
    dcb.fParity = TRUE;

#ifdef _WIN32
    dcb.fOutxCtsFlow = (BYTE)acb->HandShake;
    dcb.fRtsControl = acb->HandShake ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_ENABLE;

    if (!SetCommState(idComDev, &dcb))
    {
        DWORD dwError;

        dwError = GetLastError();

        DebMsg(DL_INFO, "Error 0x%lX initializing communication settings", dwError);

        AsyShowErr("Error initializing communication settings",
                   dwError, acb->CommPort);
        return FALSE;
    }
#else
    dcb.fOutxCtsFlow = dcb.fRtsflow = (BYTE)acb->HandShake;

    /* I am now using CtsTimeout of 0 regardless of handshaking.
       MS doc says (and I have confirmed) that this setting only
       means how long WriteComm should wait for CTS to come up
       before aborting (this is regardless of fOutxCtsFlow, oddly
       enough).  A setting of 0 means just put the chars in the
       queue and return,  the queue will then observe CTS as
       appropriate.  In testing, I couldn't get the CtsTimeout
       to work anyway.  Any timeout value > 0 and
       WriteComm returns immediately and signals a CTS timeout!
       So, just leave the timeout at 0 and rely solely upon
       fOutxCtsFlow. */

//    dcb.CtsTimeout = acb->HandShake ? 5000 : 0;

    dcb.fRtsDisable = FALSE;
    dcb.fDtrDisable = FALSE;

    if ((Result = SetCommState(&dcb)) != 0)
    {
        DebMsg(DL_INFO, "Error 0x%X initializing communication settings", Result);

        AsyShowErr("Error initializing communication settings",
                   Result, acb->CommPort);

        return FALSE;
    }
#endif

    return TRUE;
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC AsyDisconnect(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
#ifndef _WIN32
    int Result;
#endif

    DebMsg(DL_INFO, "Beginning disconnect sequence");

    DebMsg(DL_INFO, "Disabling comm events");

#ifdef _WIN32

    bRunThreads = FALSE;

    if (!INVCOMDEV(idComDev))
    {
        SetCommMask(idComDev, 0);
    }

    if (hEventPost != NULL)
    {
        SetEvent(hEventPost);       /* Otherwise, could get stuck in infinite wait! */
    }

#else

    if (!INVCOMDEV(idComDev))
    {
        SetCommEventMask(idComDev, 0);
        EnableCommNotification(idComDev, NULL, -1, -1);
    }

#endif

    if (hCommNotifyWnd != NULL)
    {
        DebMsg(DL_INFO, "Destroying comm notification window");

        DestroyWindow(hCommNotifyWnd);
        hCommNotifyWnd = NULL;
    }

    DebMsg(DL_INFO, "Flushing comm port queues");

    FlushCommQueue(FCQ_ALLQUEUES);

#ifdef _WIN32

    if (hCommWatchThread != NULL)
    {
        DebMsg(DL_INFO, "Comm monitor thread shutdown");

        if (WaitForSingleObject(hCommWatchThread, 5000) != WAIT_OBJECT_0)
            DebMsg(DL_INFO, "Comm monitor thread did not end");

        if (!CloseHandle(hCommWatchThread))
            DebMsg(DL_INFO, "Error closing comm monitor thread handle");

        hCommWatchThread = NULL;
        dwCommWatchThreadID = 0;
    }

    if (hCommWriteThread != NULL)
    {
        DebMsg(DL_INFO, "Comm write thread shutdown");

        if (WaitForSingleObject(hCommWriteThread, 5000) != WAIT_OBJECT_0)
            DebMsg(DL_INFO, "Comm write thread did not end");

        if (!CloseHandle(hCommWriteThread))
            DebMsg(DL_INFO, "Error closing comm write thread handle");

        hCommWriteThread = NULL;
        dwCommWriteThreadID = 0;
    }

    DebMsg(DL_INFO, "Removing overlapped I/O handles/events");

    if (hEventPost != NULL)
    {
        CloseHandle(hEventPost);
        hEventPost = NULL;
    }

    if (hWritePost != NULL)
    {
        CloseHandle(hWritePost);
        hWritePost = NULL;
    }

    if (oRead.hEvent != NULL)
    {
        CloseHandle(oRead.hEvent);
        oRead.hEvent = NULL;
    }

    if (oWrite.hEvent != NULL)
    {
        CloseHandle(oWrite.hEvent);
        oWrite.hEvent = NULL;
    }

    if (bWriteBuf)
    {
        DeleteCriticalSection(&csWriteBuf);
        bWriteBuf = FALSE;
    }

    if (!INVCOMDEV(idComDev))
    {
        DebMsg(DL_INFO, "Closing communications port");
        if (!CloseHandle(idComDev))
        {
            DWORD dwError;

            dwError = GetLastError();

            DebMsg(DL_INFO, "Error 0x%lX closing communication port", dwError);

            AsyShowErr("Error closing communication port",
                       dwError, acbAct.CommPort);
        }
    }

    idComDev = INVALID_HANDLE_VALUE;

#else

        /* For some reason, CloseComm is failing (returning negative return
           code) when RTS/CTS flow control is turned on, although the comm
           port seems to get closed anyway.  All we can do is ignore errors
           here */

    if ((Result = (CloseComm(idComDev))) < 0)
    {
        DebMsg(DL_INFO, "Error 0x%X closing communications port", Result);

//        AsyShowErr("Error closing communication port",
//                   Result, acbAct.CommPort);
//        return FALSE;
    }

    idComDev = -1;
#endif

    DebMsg(DL_INFO, "Disconnect sequence completed");

    return TRUE;
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC AsyConnect(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    static char PortStr[] = "COM?";

#ifdef _WIN32
    COMMTIMEOUTS ctmo;
#endif

    PortStr[3] = (char)('1' + acbAct.CommPort);

    DebMsg(DL_INFO, "Beginning connect sequence on %s", (LPSTR)PortStr);

#ifdef _WIN32
    DebMsg(DL_INFO, "Opening comm port");

    idComDev = CreateFile(PortStr, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                          OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    if (idComDev == INVALID_HANDLE_VALUE)
    {
        DWORD dwError;

        dwError = GetLastError();

        DebMsg(DL_INFO, "Error 0x%lX opening communication port", dwError);

        AsyShowErr("Error opening communication port",
                   dwError, acbAct.CommPort);

        return FALSE;
    }

    SetupComm(idComDev, RXBUFSIZE, TXBUFSIZE);

    DebMsg(DL_INFO, "Setting comm timeouts");

    ctmo.ReadIntervalTimeout = INFINITE;
    ctmo.ReadTotalTimeoutMultiplier = 0;
    ctmo.ReadTotalTimeoutConstant = 0;
    ctmo.WriteTotalTimeoutMultiplier = 0;
    ctmo.WriteTotalTimeoutConstant = 0;

    if (!SetCommTimeouts(idComDev, &ctmo))
    {
        DWORD dwError;

        dwError = GetLastError();

        DebMsg(DL_INFO, "Error 0x%lX setting comm timeouts", dwError);

        AsyShowErr("Error setting comm timeouts",
                   dwError, acbAct.CommPort);

        Disconnect();

        return FALSE;
    }

    DebMsg(DL_INFO, "Setting comm event mask");

    if (!SetCommMask(idComDev, EV_RXCHAR | EV_RLSD))
    {
        DWORD dwError;

        dwError = GetLastError();

        DebMsg(DL_INFO, "Error 0x%lX setting comm mask", dwError);

        AsyShowErr("Error setting comm mask",
                   dwError, acbAct.CommPort);

        Disconnect();

        return FALSE;
    }

    nWriteBuf = 0;
    InitializeCriticalSection(&csWriteBuf);
    bWriteBuf = TRUE;

    DebMsg(DL_INFO, "Creating events");

    oRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    oWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    hEventPost = CreateEvent(NULL, TRUE, TRUE, NULL);
    hWritePost = CreateEvent(NULL, FALSE, FALSE, NULL);

    bRunThreads = TRUE;

    DebMsg(DL_INFO, "Creating comm monitor thread");

    hCommWatchThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL, 0,
                                    (LPTHREAD_START_ROUTINE)AsyCommWatchProc,
                                    NULL, CREATE_SUSPENDED, &dwCommWatchThreadID);

    if (hCommWatchThread == NULL)
    {
        DWORD dwError;

        dwError = GetLastError();

        DebMsg(DL_INFO, "Error 0x%lX creating comm monitor thread", dwError);

        AsyShowErr("Error creating comm notification thread",
                   dwError, acbAct.CommPort);

        Disconnect();
        return FALSE;
    }

    SetThreadPriority(hCommWatchThread, THREAD_PRIORITY_BELOW_NORMAL);
    ResumeThread(hCommWatchThread);

    DebMsg(DL_INFO, "Creating comm write thread");

    hCommWriteThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL, 0,
                                    (LPTHREAD_START_ROUTINE)AsyCommWriteProc,
                                    NULL, CREATE_SUSPENDED, &dwCommWriteThreadID);

    if (hCommWriteThread == NULL)
    {
        DWORD dwError;

        dwError = GetLastError();

        DebMsg(DL_INFO, "Error 0x%lX creating comm write thread", dwError);

        AsyShowErr("Error creating comm write notification thread",
                   dwError, acbAct.CommPort);

        KermitFmtMsgBox(MB_OK | MB_ICONASTERISK,
                        "Can't create comm write notification thread!");

        Disconnect();
        return FALSE;
    }

    SetThreadPriority(hCommWriteThread, THREAD_PRIORITY_BELOW_NORMAL);
    ResumeThread(hCommWriteThread);

#else

    DebMsg(DL_INFO, "Opening communications port");

    idComDev = OpenComm(PortStr, RXBUFSIZE, TXBUFSIZE);

    if (idComDev < 0)
    {
        DebMsg(DL_INFO, "Error %i opening communication port", idComDev);
        AsyShowErr("Error opening communication port",
                   idComDev, acbAct.CommPort);

        idComDev = -1;
        return FALSE;
    }

#endif

    if (!AsySetupComm(&acbAct)) {
        Disconnect();
        return FALSE;
    }

#ifndef _WIN32

    SetCommEventMask(idComDev, EV_RXCHAR | EV_RLSD | EV_RLSDS);

    GetCommEventMask(idComDev, EV_RXCHAR | EV_RLSD | EV_RLSDS);

#endif

    DebMsg(DL_INFO, "Creating comm notification window");

    hCommNotifyWnd = CreateWindow("KermCommNotify", "Kermit Comm Notify",
                                  WS_CHILDWINDOW | WS_DISABLED,
                                  0, 0, 0, 0, hAppWnd, NULL, hAppInst, NULL);

    if (hCommNotifyWnd == NULL)
    {
        DebMsg(DL_INFO, "Error creating comm notification window");

        KermitFmtMsgBox(MB_OK | MB_ICONASTERISK,
                        "Can't create comm notification window!");
        Disconnect();
        return FALSE;
    }

#ifndef _WIN32

    DebMsg(DL_INFO, "Enabling comm notifications");

    if (!EnableCommNotification(idComDev, hCommNotifyWnd, -1, -1))
    {
        DebMsg(DL_INFO, "Error enabling comm notifications");

        KermitFmtMsgBox(MB_OK | MB_ICONASTERISK,
                        "Can't enable comm port notification!");

        Disconnect();
        return FALSE;
    }

#endif

    DebMsg(DL_INFO, "Connect sequence completed on %s", (LPSTR)PortStr);

    return TRUE;
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC AsyPauseComm(BOOL bPause)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (bPause) {
        DebMsg(DL_INFO, "Communication port paused");
#ifndef _WIN32
        EnableCommNotification(idComDev, NULL, -1, -1);
#endif
    }
    else {
        DebMsg(DL_INFO, "Communication port unpaused");
#ifndef _WIN32
        EnableCommNotification(idComDev, hCommNotifyWnd, -1, -1);
#endif
    }
}

/*----------------------------------------------------------------------------*/
int PUBFUNC AsyFlushCommQueue(int fnQueue)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (INVCOMDEV(idComDev))
        return(0);

#ifdef _WIN32
    int fQueueFlags = 0;

    if (fnQueue == FCQ_ALLQUEUES || fnQueue == FCQ_TXQUEUE)
    {
        DebMsg(DL_INFO, "Flushing transmit queue");
        fQueueFlags |= (PURGE_TXABORT | PURGE_TXCLEAR);
    }

    if (fnQueue == FCQ_ALLQUEUES || fnQueue == FCQ_RXQUEUE)
    {
        DebMsg(DL_INFO, "Flushing receive queue");
        fQueueFlags |= (PURGE_RXABORT | PURGE_RXCLEAR);
    }

    return PurgeComm(idComDev, fQueueFlags);

#else
    int nResult;

    nResult = 0;

    if (fnQueue == FCQ_ALLQUEUES || fnQueue == FCQ_TXQUEUE)
    {
        DebMsg(DL_INFO, "Flushing transmit queue");
        nResult = FlushComm(idComDev, 0);
    }

    if (fnQueue == FCQ_ALLQUEUES || fnQueue == FCQ_RXQUEUE)
    {
        DebMsg(DL_INFO, "Flushing receive queue");
        nResult = FlushComm(idComDev, 1);
    }

    return nResult;

#endif
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC AsySendBreak(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    DWORD dwTime;

    if (!bConnected || bKermit)
        return;

    if (INVCOMDEV(idComDev))
        return;

    DebMsg(DL_INFO, "Sending break");

    SetCommBreak(idComDev);
    dwTime = GetTickCount();
    while ((GetTickCount() - dwTime) < 250);
    ClearCommBreak(idComDev);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC AsySetCommDTRState(BOOL bAssert)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (INVCOMDEV(idComDev))
        return;

    DebMsg(DL_INFO, "Setting DTR state to %s", (LPSTR)(bAssert ? "ON" : "OFF"));

    EscapeCommFunction(idComDev, bAssert ? SETDTR : CLRDTR);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC AsyCheckCommStatus(UINT * pcbInQue, UINT * pcbOutQue)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    COMSTAT cs;


    if (INVCOMDEV(idComDev))
    {
        if (pcbInQue != NULL)
            *pcbInQue = 0;
        if (pcbOutQue != NULL)
            *pcbOutQue = 0;
        return;
    }

#ifdef _WIN32

    {
        DWORD   dwErrorFlags;

        ClearCommError(idComDev, &dwErrorFlags, &cs);

        if (dwErrorFlags != 0)
            DebMsg(DL_INFO, "Communications error code 0x%lX", dwErrorFlags);
    }

    if (pcbInQue != NULL)
        *pcbInQue = (UINT)cs.cbInQue;

    if (pcbOutQue != NULL)
    {
        EnterCriticalSection(&csWriteBuf);
        *pcbOutQue = (UINT)nWriteBuf;
        LeaveCriticalSection(&csWriteBuf);
    }

#else

    {
        int     nComErr;

        if ((nComErr = GetCommError(idComDev, &cs)) != 0)
            DebMsg(DL_INFO, "Communications error code 0x%X", nComErr);
    }

    if (pcbInQue != NULL)
        *pcbInQue = (UINT)cs.cbInQue;

    if (pcbOutQue != NULL)
        *pcbOutQue = (UINT)cs.cbOutQue;

#endif
}

/*----------------------------------------------------------------------------*/
int PUBFUNC AsyReadCommStr(LPSTR lpsDest, int nMaxChars)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
#ifdef _WIN32
    DWORD   dwBytesRead;
#else
    int     nBytesRead;
#endif

    if (INVCOMDEV(idComDev))
        return 0;

    CheckCommStatus(NULL, NULL);

#ifdef _WIN32

    if (!ReadFile(idComDev, lpsDest, nMaxChars, &dwBytesRead, &oRead))
    {
        DWORD dwLastError;

        dwLastError = GetLastError();

        /* We should never get an ERROR_IO_PENDING because the comm    */
        /* timeouts are zeroed out so we should just return with       */
        /* whatever is immediately available.  We handle the situation */
        /* here just to be safe.  If IO_PENDING, we just wait for it   */
        /* to complete and continue.  I have no idea what will happen  */
        /* if the IO can't ever complete (dead app???).                */

        if (dwLastError == ERROR_IO_PENDING)
        {
            DebMsg(DL_INFO, "Read I/O Pending, using GetOverlappedResult()");
            if (!GetOverlappedResult(idComDev, &oRead,
                                    &dwBytesRead, TRUE))
            {
                DWORD dwLastError2;

                dwBytesRead = 0;
                dwLastError2 = GetLastError();
                DebMsg(DL_INFO, "Read GetOverlappedResult() error 0x%lX", dwLastError2);
//                WriteTermFmt("<ROR-%lu>", dwLastError);
            }
        }
        else
        {
            dwBytesRead = 0;
            DebMsg(DL_INFO, "Read error 0x%lX", dwLastError);
//            WriteTermFmt("<RFE-%lu>", dwLastError);
        }
    }

    DebDump(DL_INFO, lpsDest, dwBytesRead, "Read %lu bytes", dwBytesRead);

    bCommBusy = (dwBytesRead == (DWORD)nMaxChars);

    if (!bCommBusy)
        SetEvent(hEventPost);

    return (int)dwBytesRead;

#else

    nBytesRead = abs(ReadComm(idComDev, lpsDest, nMaxChars));

    DebDump(DL_INFO, lpsDest, nBytesRead, "Read %i bytes", nBytesRead);

    bCommBusy = (nBytesRead == nMaxChars);

    if (!bCommBusy)
        GetCommEventMask(idComDev, EV_RXCHAR);

    return(nBytesRead);

#endif
}

/*----------------------------------------------------------------------------*/
int PUBFUNC AsyWriteCommStr(LPSTR lpsCommStr, int nStrLen)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (nStrLen == 0)
        nStrLen = lstrlen(lpsCommStr);

    if (INVCOMDEV(idComDev))
        return 0;

#ifdef _WIN32

    EnterCriticalSection(&csWriteBuf);

    DebDump(DL_INFO, lpsCommStr, nStrLen, "Queueing %i bytes", nStrLen);

    if (nStrLen + nWriteBuf > sizeof(lpsWriteBuf))
    {
        DebMsg(DL_INFO, "Insufficient queue space, queue has %i bytes, queue size is %i bytes", nWriteBuf, sizeof(lpsWriteBuf));
        LeaveCriticalSection(&csWriteBuf);
        return 0;
    }

    memcpy(lpsWriteBuf + nWriteBuf, lpsCommStr, nStrLen);
    nWriteBuf += nStrLen;

    DebMsg(DL_INFO, "Queueing completed, buffer now contains %i bytes", nWriteBuf);

    LeaveCriticalSection(&csWriteBuf);

    SetEvent(hWritePost);

    return nStrLen;

#else

    {
        int nBytesWritten;

        if (nStrLen == 0)
            nStrLen = lstrlen(lpsCommStr);

        DebDump(DL_INFO, lpsCommStr, nStrLen, "Writing %i bytes", nStrLen);

        nBytesWritten = WriteComm(idComDev, lpsCommStr, nStrLen);

        DebMsg(DL_INFO, "Bytes written: %i", nBytesWritten);

        return abs(nBytesWritten);
    }

#endif
}

/*----------------------------------------------------------------------------*/
UINT PUBFUNC AsyGetCommInfo(int nIndex)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    switch (nIndex)
    {
        case GCI_BAUDRATE:
            return (WORD)min(0xFFFF, acbAct.BigBaudRate);
            break;

        case GCI_PARITY:
            return acbAct.Parity;
            break;

        case GCI_DATABITS:
            return acbAct.DataBits;
            break;

        case GCI_STOPBITS:
            return acbAct.StopBits;
            break;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC AsyConnDlgInit(HWND hDlg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int nButton;
    char szBaudRate [16];

    _ltoa_s(acbAct.BigBaudRate, szBaudRate, sizeof(szBaudRate), 10);
    SetDlgItemText(hDlg, IDD_BAUDRATE, szBaudRate);

    CheckRadioButton(hDlg, IDD_4DATABITS, IDD_8DATABITS,
                     (acbAct.DataBits + IDD_4DATABITS - 4));
    CheckRadioButton(hDlg, IDD_NOPTY, IDD_SPACEPTY,
                     (acbAct.Parity + IDD_NOPTY));
    CheckRadioButton(hDlg, IDD_1STOPBIT, IDD_2STOPBITS,
                     (acbAct.StopBits + IDD_1STOPBIT));
    CheckDlgButton(hDlg, IDD_FLOWCTL, acbAct.FlowControl);
    CheckDlgButton(hDlg, IDD_HANDSHK, acbAct.HandShake);
    CheckRadioButton(hDlg, IDD_COM1, IDD_COM4,
                     (acbAct.CommPort + IDD_COM1));

    for (nButton = IDD_COM1; nButton <= IDD_COM4; nButton++)
        EnableWindow(GetDlgItem(hDlg, nButton), !bConnected);

    return TRUE;
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC AsyConnDlgTerm(HWND hDlg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    char szBaudRate[16];

    AsyncConnBlk acbWork;

    memcpy(&acbWork, &acbAct, sizeof(acbAct));

    GetDlgItemText(hDlg, IDD_BAUDRATE, szBaudRate, sizeof(szBaudRate));
    acbWork.BigBaudRate = atol(szBaudRate);

    if (acbWork.BigBaudRate == 0)
    {
        KermitFmtMsgBox(MB_OK | MB_ICONASTERISK,
                        "Invalid Baud Rate specification!");
        return FALSE;
    }

    acbWork.DataBits = (WORD)(GetRadioButton(hDlg, IDD_4DATABITS, IDD_8DATABITS)
                       - IDD_4DATABITS + 4);
    acbWork.Parity = (WORD)(GetRadioButton(hDlg, IDD_NOPTY, IDD_SPACEPTY) - IDD_NOPTY);
    acbWork.StopBits = (WORD)(GetRadioButton(hDlg, IDD_1STOPBIT, IDD_2STOPBITS)
                       - IDD_1STOPBIT);
    acbWork.FlowControl = (WORD)IsDlgButtonChecked(hDlg, IDD_FLOWCTL);
    acbWork.HandShake = (WORD)IsDlgButtonChecked(hDlg, IDD_HANDSHK);
    acbWork.CommPort = (WORD)(GetRadioButton(hDlg, IDD_COM1, IDD_COM4) - IDD_COM1);

    if (memcmp(&acbAct, &acbWork, sizeof(acbAct)) != 0)
    {
        if (bConnected)
            if (!AsySetupComm(&acbWork))
                return FALSE;

        memcpy(&acbAct, &acbWork, sizeof(acbAct));
        bChanged = TRUE;
    }

    return TRUE;
}

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export AsyConnDlgProc(HWND hDlg, unsigned message,
                                      UINT wParam, LONG X(lParam))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    switch (message)
    {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDD_4DATABITS:
                case IDD_5DATABITS:
                case IDD_6DATABITS:
                case IDD_7DATABITS:
                case IDD_8DATABITS:
                    CheckRadioButton(hDlg, IDD_4DATABITS,
                                     IDD_8DATABITS, wParam);
                    break;

                case IDD_NOPTY:
                case IDD_ODDPTY:
                case IDD_EVENPTY:
                case IDD_MARKPTY:
                case IDD_SPACEPTY:
                    CheckRadioButton(hDlg, IDD_NOPTY, IDD_SPACEPTY, wParam);
                    break;

                case IDD_1STOPBIT:
                case IDD_1HSTOPBITS:
                case IDD_2STOPBITS:
                    CheckRadioButton(hDlg, IDD_1STOPBIT,
                                     IDD_2STOPBITS, wParam);
                    break;

                case IDD_COM1:
                case IDD_COM2:
                case IDD_COM3:
                case IDD_COM4:
                    CheckRadioButton(hDlg, IDD_COM1, IDD_COM4, wParam);
                    break;

                case IDD_OK:
                    if (!AsyConnDlgTerm(hDlg))
                        break;
                    EndDialog(hDlg, TRUE);
                    break;

                case IDD_CANCEL:
                    EndDialog(hDlg, FALSE);
                    break;

                default:
                    return FALSE;
                    break;
            }
            break;

        case WM_INITDIALOG:
            return AsyConnDlgInit(hDlg);

        default:
            return FALSE;
    }
    return TRUE;
}

#ifdef WIN32

VOID PRVFUNC AsyCommNotify_OnCommNotify
(HWND   X(hWnd),       /* Window that received the command.                   */
 int    X(cid),        /* Communication device id.                            */
 UINT   flags)         /* Notification status flag.                           */
{
    if (flags & EV_RXCHAR)
        bCommBusy = TRUE;

    if (flags & EV_RLSD)
    {
        DWORD dwModemStatus;

        GetCommModemStatus(idComDev, &dwModemStatus);

        DebMsg(DL_INFO, "Carrier detect %s", (LPSTR)((dwModemStatus & MS_RLSD_ON) ?
                     "On" : "Off"));

        if (!(dwModemStatus & MS_RLSD_ON))
            DMCommEvent(CE_CLOSED);
    }
}

#else

VOID PRVFUNC AsyCommNotify_OnCommNotify
(HWND   X(hWnd),       /* Window that received the command.                   */
 int    X(cid),        /* Communication device id.                            */
 UINT   flags)         /* Notification status flag.                           */
{
    switch (flags) {
        case CN_EVENT:
        {
            UINT iCommEvent;

            iCommEvent = GetCommEventMask(idComDev, EV_RXCHAR | EV_RLSD | EV_RLSDS);

            if (iCommEvent & EV_RXCHAR)
                bCommBusy = TRUE;

            if (iCommEvent & EV_RLSD) {
                DebMsg(DL_INFO, "Carrier detect %s", (LPSTR)((iCommEvent & EV_RLSDS) ?
                     "On" : "Off"));

                if (!(iCommEvent & EV_RLSDS))
                    DMCommEvent(CE_CLOSED);
            }

            break;
        }

        case CN_RECEIVE:
            break;

        case CN_TRANSMIT:
            break;
    }
}

#endif

LRESULT PUBFUNC AsyCommNotifyWndProc(HWND hWnd, UINT message,
                                     WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

        HANDLE_MSG(hWnd, WM_COMMNOTIFY, AsyCommNotify_OnCommNotify);

        default:
            return(DefWindowProc(hWnd, message, wParam, lParam));
    }

    return 0L;
}

BOOL PUBFUNC CMAsyInit(VOID)
{
    memcpy(&acbAct, &acbDef, sizeof(acbAct));

    return TRUE;
}

BOOL PUBFUNC CMAsyLoad(VOID)
{
    memcpy(&acbAct, &acbDef, sizeof(acbAct));

    return TRUE;
}

BOOL PUBFUNC CMAsySetConfig(UINT wInfoSize, PSTR pszInfo)
{
    if (wInfoSize > sizeof(acbAct))
        return FALSE;

    memcpy(&acbAct, &acbDef, sizeof(acbAct));
    memcpy(&acbAct, pszInfo, wInfoSize);
    if (acbAct.BaudRate != 0)
    {
        acbAct.BigBaudRate = acbAct.BaudRate;
        acbAct.BaudRate = 0;
    }

    AsySetupComm(&acbAct);

    return TRUE;
}

BOOL PUBFUNC CMAsyGetConfig(UINT * pwInfoSize, PSTR pszInfo)
{
    *pwInfoSize = sizeof(acbAct);
    memcpy(pszInfo, &acbAct, sizeof(acbAct));

    return TRUE;
}

BOOL PUBFUNC CMAsySetup(HINSTANCE hInst, HWND hWnd)
{
    GoDialogBox(hInst, "AsyncDlgBox", hWnd, (FARPROC)AsyConnDlgProc);

    return TRUE;
}
