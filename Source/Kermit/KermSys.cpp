/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMSYS.C                                    **
**                                                                            **
**  This module contains system dependent routines for the Kermit protocol.   **
**  With some luck.  These routines were based on those for the Unix          **
**  environment, but were pretty heavily modified for MS Windows.             **
**                                                                            **
*******************************************************************************/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include "kermprot.h"

int tcol = 0;

/*----------------------------------------------------------------------------*/
void PUBFUNC traw(char *s, int len)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (tcol > 0) {
        WriteTermStr("\r\n", 2, FALSE);
        tcol = 0;
    }

    WriteTermStr(s, len, TRUE);

    WriteTermStr("\r\n", 2, FALSE);
}

/*----------------------------------------------------------------------------*/
void PUBFUNC tchar(char c)
{
    if (c == '\r')
        tcol = 0;
    else if (c >= ' ')
        tcol++;

    WriteTermStr(&c, 1, FALSE);
}

/*----------------------------------------------------------------------------*/
void PUBFUNC tprog(char c)
{
    if (++tcol >= 40) {
        WriteTermStr("\r\n", 2, FALSE);
        tcol = 1;
    }

    WriteTermStr(&c, 1, FALSE);
}

/*----------------------------------------------------------------------------*/
void PUBFUNC tmsg(char *fmt, ...)
{
    char    buf[128];
    va_list arg_ptr;
    int     len;

    if (tcol > 0) {
        WriteTermStr("\r\n", 2, FALSE);
        tcol = 0;
    }

    va_start(arg_ptr, fmt);
    len = wvsprintf(buf, fmt, arg_ptr);
    va_end(arg_ptr);

    WriteTermStr(buf, len, FALSE);

    WriteTermStr("\r\n", 2, FALSE);

    SetDlgItemText(hWndStat, IDD_MESSAGE, buf);
}

/*----------------------------------------------------------------------------*/
void PUBFUNC tdata(LPSTR lpszTag, LPSTR lpszData, int nDataLen)
{
    char szWork [102];
    int  nDataPtr, nWorkPtr;

    if (tcol > 0) {
        WriteTermStr("\r\n", 2, FALSE);
        tcol = 0;
    }

    if (nDataLen < 0)
        nDataLen = lstrlen(lpszData);

    WriteTermStr(lpszTag, lstrlen(lpszTag), FALSE);

    for (nDataPtr = 0; nDataPtr < nDataLen; ) {
        for (nWorkPtr = 0; (nWorkPtr < 100) && (nDataPtr < nDataLen); nDataPtr++)
            if (lpszData[nDataPtr] < ' ') {
                szWork[nWorkPtr++] = '^';
                szWork[nWorkPtr++] = (char)((char)(lpszData[nDataPtr]) + (char)'@');
            }
            else
                szWork[nWorkPtr++] = lpszData[nDataPtr];

        szWork[nWorkPtr] = '\0';
        WriteTermStr(szWork, lstrlen(szWork), FALSE);
    }

    WriteTermStr("\r\n", 2, FALSE);
}

/*----------------------------------------------------------------------------*/
void PUBFUNC ttflui(void)
{
    FlushCommQueue(FCQ_RXQUEUE);
    clrinl = TRUE;
}

/*----------------------------------------------------------------------------*/
int PUBFUNC ttinl (char *dest, int max, char eol, int timo)
{
    static int newinl;
    static int x = 0, ccn = 0;
    static DWORD dwTime;
    static char sComBuf [256];
    static int nComBufLen, nComBufPtr;

    if (cc || cr) {
        newinl = TRUE;
        return(-1);
    }

    if (clrinl) {
        ccn = 0;
        nComBufLen = nComBufPtr = 0;
        newinl = TRUE;
        clrinl = FALSE;
    }

    if (newinl) {
        *dest = '\0';
        x = 0;
        dwTime = 0;
        newinl = FALSE;
    }

    if (dwTime == 0)
        dwTime = GetTickCount();

    if (dwTime != 0 && ((GetTickCount() - dwTime) > ((DWORD)timo * 1000L))) {
        newinl = TRUE;
        return(-1);
    }

    if (nComBufPtr >= nComBufLen) {
        nComBufLen = ReadCommStr(sComBuf, sizeof(sComBuf));
        nComBufPtr = 0;
    }

    while (nComBufPtr < nComBufLen) {
        if (x + 2 >= max) {
            newinl = TRUE;
            return(-1);
        }

        dest[x++] = sComBuf[nComBufPtr++];

        if (sComBuf[nComBufPtr - 1] == 3) {
            if (++ccn > 1) {
                tmsg("\n\r^C...");
                cc = TRUE;
                return(-1);
            }
        }
        else
            ccn = 0;

        if (sComBuf[nComBufPtr - 1] == eol) {
            dest[x] = '\0';
            newinl = TRUE;
            return(x);
        }
    }

    return(0);
}

/*----------------------------------------------------------------------------*/
short PUBFUNC ttol (char *s, int n)
{
    return((short)WriteCommStr(s, n));
}

/*----------------------------------------------------------------------------*/
int PUBFUNC zopeni(char *name)
{
    if (fopen_s(&ifp, name, "rb") != 0)
        return(-1);
    else
        return(0);
}

/*----------------------------------------------------------------------------*/
int PUBFUNC zopeno(char *name)
{
    if (fopen_s(&ofp, name, "wb") != 0)
        return(-1);
    else
        return(0);
}

/*----------------------------------------------------------------------------*/
int PUBFUNC zclosi(void)
{
    if (fclose(ifp) == EOF)
        return(-1);
    else
        return(0);
}

/*----------------------------------------------------------------------------*/
int PUBFUNC zcloso(int discard)
{
    if (fclose(ofp) == EOF)
        return(-1);
    else {
        if (discard) {
            if (remove(filnam) == EOF)
                tmsg("Discard Failed!",15);
            else
                tmsg("File Discarded!",15);
        }
        return(0);
    }
}

/*----------------------------------------------------------------------------*/
void PUBFUNC zrtol(char *n1, char *n2, int /* warn */ )
{
    /* if warn and n2 already exists, construct unique new local name */

    lstrcpy(n2,n1);
}

/*----------------------------------------------------------------------------*/
void PUBFUNC zltor(char *n1, char *n2)
{
    char *sptr;

    sptr = strrchr (n1, '\\');
    if (sptr)
        lstrcpy(n2, sptr + 1);
    else
        lstrcpy(n2, n1);
}

/*----------------------------------------------------------------------------*/
int PUBFUNC zgetc(void)
{
    int c;

    if ((c = getc(ifp)) == EOF)
        return(-1);

    Stats.Bytes++;
    return(c);
}

/*----------------------------------------------------------------------------*/
int PUBFUNC zputc(int c)
{
    if (putc(c, ofp) == EOF)
        return(-1);

    Stats.Bytes++;
    return(0);
}
