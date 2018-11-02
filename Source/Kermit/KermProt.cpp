/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMPROT.C                                   **
**                                                                            **
**  This module contains Kermit protocol implementation.  The routines used   **
**  are derived from the book by Frank daCruz and are modified for            **
**  compatibility with MS Windows.                                            **
**                                                                            **
*******************************************************************************/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include "kermprot.h"

#include <dos.h>

/*----------------------------------------------------------------------------*/
void PRVFUNC error(int seq2, char *s)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    spack('E', seq2, strlen(s), s);

    if (local) {
        tmsg("ERROR: %s", (LPSTR)s);
    }

    tterm();
}

/*----------------------------------------------------------------------------*/
int PRVFUNC winit(int seq2)
{
    int i;

    /* allocate window table from global memory */

    hwindata = GlobalAlloc(GHND, wpktlen * wsize);

    if (hwindata == NULL)
        return(-1);

    /* lock window data in memory */

    pwindata = GlobalLock(hwindata);

    if (pwindata == NULL) {
        GlobalFree(hwindata);
        return(-1);
    }

    for (i = 0; i < wsize; i++) { /* clear table */
        table[i].ack = 1;           /* default to ACKed */
        table[i].retries = 0;       /* no retries yet */
    }

    high = seq2;                   /* table pos = 0 */
    low = (seq2 - wsize + 1) & 63; /* table pos = wsize - 1 */

    return(0);
}

/*----------------------------------------------------------------------------*/
void PRVFUNC wterm()
{
    /* unlock window data */

    if (pwindata != NULL) {
        GlobalUnlock(hwindata);
        pwindata = NULL;
    }

    /* free window data */

    if (hwindata != NULL) {
        GlobalFree(hwindata);
        hwindata = NULL;
    }
}

/*----------------------------------------------------------------------------*/
void PRVFUNC wrotate(char * data2)
{
    int i;

    /* fixup high and low */

    low = (low + 1) & 63;
    high = (high + 1) & 63;

    /* rotate table */

    for (i = (wsize - 1); i > 0; i--)
        table[i] = table[i - 1];
    table[0].ack = 0;
    table[0].retries = 0;

    /* shift data */

    for (i = ((wsize - 1) * wpktlen) - 1; i >= 0; i--)
        ((LPSTR)pwindata)[i + wpktlen] = ((LPSTR)pwindata)[i];

    /* insert new data */

    lstrcpy(WDATA(0), data2);
}

/*----------------------------------------------------------------------------*/
int PRVFUNC spack(char type, int n, int len, char far *d)
{
    int i=0, j, k;

    for (i = 0; i < spadn; i++)
        sndpkt[i] = spadc;

    sndpkt[i++] = smark;
    k = i++;
    sndpkt[i++] = tochar(n);
    sndpkt[i++] = type;
    j = len + bctu + 2;
    if (j > 94) {
        j -= 2;
        sndpkt[k] = tochar(0);
        sndpkt[i++] = tochar(j / 95);
        sndpkt[i++] = tochar(j % 95);
        sndpkt[i] = '\0';
        sndpkt[i++] = tochar(chk1(sndpkt + k));
    }
    else
        sndpkt[k] = tochar(j);

    for (j = len; j > 0; j--)
        sndpkt[i++] = *d++;

    sndpkt[i] = '\0';
    switch (bctu) {
        case 1:
            sndpkt[i++] = tochar((chk1(sndpkt+k)));
            break;
        case 2:
            j = (int)chksum(sndpkt+k);
            sndpkt[i++] = tochar((j >> 6) & 077);
            sndpkt[i++] = tochar(j & 077);
            break;
        case 3:
            j = chk3(sndpkt+k);
            sndpkt[i++] = tochar((j >> 12) & 017);
            sndpkt[i++] = tochar((j >> 6) & 077);
            sndpkt[i++] = tochar(j & 077);
            break;
    }
    sndpkt[i++] = seol;
    sndpkt[i] = '\0';
    sndpkl = i;

    if (ProtSet.DebugOther) {
        tmsg("SPkt: Type=%c, Seq=%i, Chk=%i, Len=%i",
                 (char)type, (int)n, (int)bctu, (int)sndpkl);
    }

    if (ProtSet.DebugPacket) {
        tdata("SDat:", sndpkt, sndpkl);
    }

    i = ttol(sndpkt,sndpkl);

    if (state != sserv) {
        if (local && !xflag)
            tprog('.');
        ShowTypeOut(type);
        ShowPackets(++Stats.Packets);
    }

    return(i);
}

/*----------------------------------------------------------------------------*/
char PRVFUNC rpack (void)
{
    int i, j, x, type, rlnpos;
    char pbc[4];

    /*************************************************************/
    /*                                                           */
    /*  FIXME: This function can return with rdatap set even     */
    /*  though it points to nothing!  rdatap is used a lot!      */
    /*                                                           */
    /*************************************************************/

    if ((j = ttinl(rcvpkt, MAXRP, reol, stimo)) == 0)
        return(0);

    rsn = rln = -1;

    if (j < 0) {
        if (state != sserv)
            ShowTypeIn('T');
        if (ProtSet.DebugOther)
            tmsg("RPkt: Type=T (timeout)");
        return('T');
    }

    if (ProtSet.DebugPacket)
        tdata("RDat:", rcvpkt, j);

    for (i = 0; rcvpkt[i] != rmark && (i < j); i++);

    if (i == j) {
        if (state != sserv)
            ShowTypeIn('Q');
        if (ProtSet.DebugOther)
            tmsg("RPkt: Type=Q (corrupt packet, no start char found)");
        return('Q');
    }

    rlnpos = ++i;
    if ((j = unchar(rcvpkt[i++])) == 0) {
        j = rlnpos + 5;
        if (j > MAXRP) {
            if (state != sserv)
                ShowTypeIn('Q');
            if (ProtSet.DebugOther)
                tmsg("RPkt: Type=Q (corrupt packet, max len exceeded), Len=%i", (int)j);
            return('Q');
        }
        x = rcvpkt[j];
        rcvpkt[j] = '\0';
        if (unchar(x) != chk1(rcvpkt+rlnpos)) {
            if (state != sserv)
                ShowTypeIn('Q');
            if (ProtSet.DebugOther)
                tmsg("RPkt: Type=Q (bad hdr chk1), Len=%i", (int)j);
            return('Q');
        }
        rcvpkt[j] = (char)x;
        rln = unchar(rcvpkt[j - 2]) * 95 + unchar(rcvpkt[j - 1]) - bctu;
        j = 3;
    }
    else {
        rln = j - bctu - 2;
        j = 0;
    }
    rsn = unchar(rcvpkt[i++]);
    type = rcvpkt[i++];
    i += j;
    rdatap = rcvpkt + i;
    j = rln + i;

    if (j > MAXRP) {
        if (state != sserv)
            ShowTypeIn('Q');
        if (ProtSet.DebugOther)
            tmsg("RPkt: Type=Q (bad pkt, max data len exceeded), Len=%i", (int)j);
        return('Q');
    }

    for (x = 0; x < bctu; x++)
        pbc[x] = rcvpkt[j+x];
    rcvpkt[j] = '\0';

    switch (bctu) {
        case 1:
            if (unchar(*pbc) != (char)chk1(rcvpkt+rlnpos)) {
                if (state != sserv)
                    ShowTypeIn('Q');
                if (ProtSet.DebugOther)
                    tmsg("RPkt: Type=Q (bad data chk1), Len=%i", (int)j);
                return('Q');
            }
            break;

        case 2:
            x = unchar(*pbc) << 6 | unchar(pbc[1]);
            if (x != (int)chksum(rcvpkt+rlnpos)) {
                if (state != sserv)
                    ShowTypeIn('Q');
                if (ProtSet.DebugOther)
                    tmsg("RPkt: Type=Q (bad data chk2), Len=%i", (int)j);
                return('Q');
            }
            break;

        case 3:
            x = unchar(*pbc) << 12 | unchar(pbc[1]) << 6 | unchar(pbc[2]);
            if (x != chk3(rcvpkt+rlnpos)) {
                if (state != sserv)
                    ShowTypeIn('Q');
                if (ProtSet.DebugOther)
                    tmsg("RPkt: Type=Q (bad data chk3), Len=%i", (int)j);
                return('Q');
            }
            break;

        default:
            if (state != sserv)
                ShowTypeIn('Q');
            if (ProtSet.DebugOther)
                tmsg("RPkt: Type=Q (state error? state != sserv), Len=%i", (int)j);
            return('Q');
    }

    if (state != sserv)
        ShowTypeIn((char)type);

    if (ProtSet.DebugOther)
        tmsg("RPkt: Type=%c, Seq=%i, Chk=%i, Len=%i",
                 (char)type, (int)rsn, (int)bctu, (int)rln);

    return((char)type);
}

/*----------------------------------------------------------------------------*/
int PRVFUNC input(void)
{
    UINT   cbOutQue;

    static int xtry;
           int type;
           int i;

    if (start != 0) {
        type = start;
        start = 0;
        xtry = 0;
        return(type);
    }

    if (state == ssdat) {

        /* if "oldest" entry ACKed and send buf empty... */

        CheckCommStatus(NULL, &cbOutQue);

        if ((table[wsize - 1].ack == 1) && cbOutQue <= MAXSP) {
            int i2, x;

            /* try to build and send another packet */

            if ((x = sdata((high + 1) & 63)) == 0) {

                /* no more data! */

                if (ProtSet.DebugOther)
                    tmsg("Wait ACKs: ");

                /* search for unACKed window table entries */

                for (i2 = wsize - 1; i2 >= 0; i2--) {
                    if (ProtSet.DebugOther)
                        tmsg(" %i-%i", wpostoseq(i2), table[i2].ack);

                    if (table[i2].ack == 0)
                        break;
                }

                /* if everybody ACKed, report EOF, otherwise
                   fall thru to receive logic */

                if (i2 < 0) {

                    /* return 'Z' to indicate we are done sending
                       and table is empty.
                       NOTE: 'Z' is actually intended to be used as
                             the packet type to indicate the end of
                             a receive operation, I am extending its
                             meaning here to indicate we are done
                             sending! */

                    return('Z');
                }
            }
            else if (x < 0)
                /* error in sdata */

                return(-1);
            else {
                /* rotate table and fall thru to rcv pkt */

                wrotate(data);

                if (ProtSet.DebugOther)
                    tmsg("Sent %i, High=%i, Low=%i",
                             high, high, low);
            }
        }
    }

    if ((type = rpack()) == 0) {
        return(0);
    }

    if (cc) {
        tmsg("Kermit Aborted by User");
        EndKermit(-1);
        return(0);
    }

    if (type == 'E')
        return(type);

    if (wsize > 1 && state == ssdat) {
        if (strchr("TQN",type) && (xtry > limit)) {
            xtry = 0;
            return('T');
        }

        if (type == 'T') {
            /* resend oldest unACKed packet */
            xtry++;
            for (i = wsize - 1; i >= 0; i--) {
                if (table[i].ack == 0) {
                    spack('D', wpostoseq(i),
                          lstrlen(WDATA(i)), WDATA(i));
                    break;
                }
            }
            cr = FALSE;
            return(0);
        }

        if (type == 'N') {
            /* resend desired (based on rsn) packet */
            xtry++;
            if (wseqintab(rsn))
                spack('D', rsn,
                      lstrlen(WDATA(wseqtopos(rsn))),
                      WDATA(wseqtopos(rsn)));
            cr = FALSE;
            return(0);
        }

        if (type == 'Q') {
            /* ignore corrupt packets */
            return(0);
        }

        /* return a valid packet */
        xtry = 0;
        return(type);
    }

    if (wsize > 1 && state == srdat) {
        if (strchr("TQ",type)) {
            if (xtry > limit) {
                xtry = 0;
                return('T');
            }

            xtry++;

            cr = FALSE;

            /* nak most desired packet */

            for (i = 0; i < wsize; i++) {
                if (table[i].ack == 0) {
                    nak((high - i) & 63);
                    return(0);
                }
            }

            nak((high + 1) & 63);
            return(0);
        }

        xtry = 0;
        return(type);
    }

    if (rsn != seq || strchr("TQN",type)) {
        if ((type == 'N') && (rsn == ((seq + 1) & 63)))
            type = 'Y';
        else if ((type == 'Y') && (rsn == ((seq - 1) & 63)))
            return(0);
        else if (xtry > limit && state != sserv)
            type = 'T';
        else {
            xtry++;
            resend();
            cr = FALSE;
            return(0);
        }
    }

    xtry = 0;
    return(type);
}

/*----------------------------------------------------------------------------*/
void PRVFUNC nxtpkt (void)
{
    seq = (seq + 1) & 63;
}

/*----------------------------------------------------------------------------*/
int PRVFUNC resend(void)
{
    int x;
    if (*sndpkt) {
        if (ProtSet.DebugPacket)
            tdata("Snd=", sndpkt, sndpkl);

        x = ttol(sndpkt, sndpkl);
    }
    else
        x = nak(seq);

    if (state != sserv) {
        if (local && !xflag)
            tprog('%');
        ShowRetries(++Stats.Retries);
    }

    return(x);
}

/*----------------------------------------------------------------------------*/
int PRVFUNC chk3(char *s)
{
    unsigned int c, q;
    long crc = 0;

    while ((c = *s++) != '\0') {
        if (parity) c &= 0177;
        q = (int)(crc ^ c) & 017;
        crc = (crc >> 4) ^ (q * 010201);
        q = (int)(crc ^ (c >> 4)) & 017;
        crc = (crc >> 4) ^ (q * 010201);
    }
    return((int)crc);
}


/*----------------------------------------------------------------------------*/
int PRVFUNC chk1(char *packet)
{
    int s, t;

    s = (int)chksum(packet);
    t = (((s & 192) >> 6) + s) & 63;
    return(t);
}

/*----------------------------------------------------------------------------*/
long PRVFUNC chksum(char *p)
{
    unsigned int m;
    long s;

    m = (parity) ? 0177 : 0377;
    for (s = 0; *p != '\0'; *p++)
        s += *p & m;
    return(s & 07777);
}

/*----------------------------------------------------------------------------*/
void PUBFUNC tinit(void)
{
    spsiz = ProtSet.SendPktSize;
    rpsiz = ProtSet.RecvPktSize;
    stimo = ProtSet.SendTimeout;
    rtimo = ProtSet.RecvTimeout;
    bctr = ProtSet.BlockCheck + 1;
    limit = ProtSet.RetryLimit;

    spadc = (char)PktSet.Send.PadChar;
    rpadc = (char)PktSet.Recv.PadChar;
    rpadn = PktSet.Recv.PadCount;
    spadn = PktSet.Send.PadCount;
    smark = (char)PktSet.Send.StartChar;
    rmark = (char)PktSet.Recv.StartChar;
    seol = (char)PktSet.Send.EndChar;
    reol = (char)PktSet.Recv.EndChar;
    sctlq = (char)PktSet.Send.CtlPrefix;
    rctlq = (char)PktSet.Recv.CtlPrefix;

    bctu = 1;
    ebq = '&';
    ebqflg = 0;
    rqf = -1;
    rq = 0;
    sq = 'Y';
    rpt = 0;
    rptq = '~';
    rptflg = 0;
    start = 0;
    clrinl = 0;

    capas = 10;
    atcapb = 8; atcapr = ProtSet.Attributes; atcapu = 0;
    swcapb = 4; swcapr = (ProtSet.WndSize > 1); swcapu = 0;
    lpcapb = 2; lpcapr = (ProtSet.RecvPktSize > 94 || ProtSet.SendPktSize > 94); lpcapu = 0;

    wsize = ProtSet.WndSize;

    seq = 0;
    sndpkl = 0;

    sndpkt[0] = '\0';
    rcvpkt[0] = '\0';
    isp = NULL;
    osp = NULL;
    ssc = 0;

    filsiz = 0;

    cx = 0;
    cz = 0;
    cr = 0;
    ce = 0;
    cc = 0;
    xflag = 0;
    xpkt = 0;

    parity = (GetCommInfo(GCI_PARITY) != NOPARITY);
    delay = 5;
    local = 1;
    txim = TRUE;

    server = 0;
    first = 0;
    keep = 0;

    Stats.Packets = 0;
    Stats.Bytes = 0;
    Stats.Retries = 0;

    hFind = NULL;

    SetDlgItemText(hWndStat, IDD_ACTION, "");
    ShowPackets(0);
    ShowTypeIn(' ');
    ShowTypeOut(' ');
    ShowBytes(0);
    ShowRetries(0);
//    SetDlgItemText(hWndStat, IDD_MESSAGE, "");

    ttflui();
}

/*----------------------------------------------------------------------------*/
void PUBFUNC tterm(void)
{
/*      if (fileopen)             */
/*         close and keep?        */

    /* make sure we free up any leftover window data buffers */

    wterm();
    if (local)
        tmsg("Done");

    /* free up any pending find file buffers! */

#ifdef _WIN32
    if (hFind != NULL) {
        FindClose(hFind);
        hFind = NULL;
    }
#endif
}

/*----------------------------------------------------------------------------*/
int PRVFUNC ack(int seq2)
{
    int x;
    x = spack('Y', seq2, 0, "");
    return(x);
}

/*----------------------------------------------------------------------------*/
int PRVFUNC ackl(int seq2, char *s)
{
    int x;
    x = spack('Y', seq2, strlen(s), s);
    return(x);
}

/*----------------------------------------------------------------------------*/
int PRVFUNC nak(int seq2)
{
    int x;
    x = spack('N', seq2, 0, "");
    return(x);
}

/*----------------------------------------------------------------------------*/
int PRVFUNC sinit(char c)
{
    char *s;

    s = rpar();
//    tmsg("Initializing to send");
    if (local == 0 && c == 'S' && server == 0) {
        tmsg("Escape back to local system, give RECEIVE command...");
    /*  sleep(delay);  */
    }
    return(spack(c, seq, strlen(s), s));
}

/*----------------------------------------------------------------------------*/
void PRVFUNC rinit(void)
{
//    tmsg("Initializing to receive");
}

/*----------------------------------------------------------------------------*/
int PRVFUNC gnfile(void)
{
#ifdef _WIN32
    static WIN32_FIND_DATA fd;
#else
    static struct find_t find;
#endif
    static char   szFileSpec [80];

    if (cz)
        return(0);

    if (hSendList != NULL) {

#ifdef _WIN32
#define FILE_SUPPRESS (FILE_ATTRIBUTE_DIRECTORY | \
                       FILE_ATTRIBUTE_HIDDEN |    \
                       FILE_ATTRIBUTE_SYSTEM |    \
                       FILE_ATTRIBUTE_TEMPORARY)

        do {

            if (hFind != NULL) {
                if (!FindNextFile(hFind, &fd)) {
                    FindClose(hFind);
                    hFind = NULL;
                }
            }

            if (hFind == NULL) {
                do {
                    if (ListBox_GetCount(hSendList) == 0) {
                        DestroyWindow(hSendList);
                        hSendList = NULL;
                        return(0);
                    }

                    ListBox_GetText(hSendList, 0, szFileSpec);
                    ListBox_DeleteString(hSendList, 0);

                } while ((hFind = FindFirstFile(szFileSpec, &fd)) == INVALID_HANDLE_VALUE);
            }

        } while ((fd.dwFileAttributes & FILE_SUPPRESS) != 0);

        lstrcpy(strrchr(szFileSpec, '\\') + 1, fd.cFileName);
        lstrcpy(filnam, szFileSpec);
        return(1);

#else
        if (nfils != 0) {
            if (_dos_findnext(&find) == 0) {
                lstrcpy(strrchr(szFileSpec, '\\') + 1, find.name);
                lstrcpy(filnam, szFileSpec);
                return(1);
            }

            nfils = 0;
        }

        do {
            if (ListBox_GetCount(hSendList) == 0) {
                DestroyWindow(hSendList);
                hSendList = NULL;
                hFind = NULL;
                return(0);
            }

            ListBox_GetText(hSendList, 0, szFileSpec);
            ListBox_DeleteString(hSendList, 0);

        } while (_dos_findfirst(szFileSpec, 0, &find) != 0);

        lstrcpy(strrchr(szFileSpec, '\\') + 1, find.name);
        lstrcpy(filnam, szFileSpec);
        nfils = 1;
        return(1);

#endif

    }

    if (nfils > 0) {
        OFSTRUCT OpenBuf;

        if (OpenFile(filnam, &OpenBuf, OF_PARSE) == -1)
            return(0);

        lstrcpy(filnam, (char *)(OpenBuf.szPathName));
        nfils = 0;
        return(1);
    }

    return(0);
}

/*----------------------------------------------------------------------------*/
int PRVFUNC sfile(void)
{
    int x;
    char pktnam[80];
    char MsgBuf[100];

    if (zopeni(filnam) < 0) return(-1);
    zltor(filnam, pktnam);
    x = encstr(pktnam);
    wsprintf(MsgBuf, "Sending %s", (LPSTR)pktnam);
    SetDlgItemText(hWndStat, IDD_ACTION, MsgBuf);
    if (local) {
        tmsg("Sending %s as %s", (LPSTR)filnam, (LPSTR)pktnam);
    }
    first = 1;
    maxsiz = spsiz - (bctu + (spsiz > 94 ? 0 : 2));
    nxtpkt();
    return(spack((char)(xpkt ? 'X' : 'F'), seq, x, data));
}

/*----------------------------------------------------------------------------*/
int PRVFUNC rcvfil(void)
{
    char myname[80];
    char MsgBuf[100];

    if (xflag) {
        SetDlgItemText(hWndStat, IDD_ACTION, "Receiving Screen Data");
        tmsg("");
//        tmsg("Receiving Screen Data");
        return(0);
    }

    zrtol(filnam,myname,TRUE);
    if (zopeno(myname) < 0)
        return(-1);
    else {
        wsprintf(MsgBuf, "Receiving %s", (LPSTR)myname);
        SetDlgItemText(hWndStat, IDD_ACTION, MsgBuf);
        if (local)
            tmsg("Receiving %s as %s", (LPSTR)filnam, (LPSTR)myname);
        lstrcpy(filnam, myname);
        return(0);
    }
}

/*----------------------------------------------------------------------------*/
int PRVFUNC closof(int discard)
{
    if (xflag)
        return(0);
    if (zcloso(discard) < 0)
        return(-1);
//    tmsg("OK");
    return(0);
}

/*----------------------------------------------------------------------------*/
int PRVFUNC seot(void)
{
    nxtpkt();
//    tmsg("Done");
    return(spack('B',seq,0,""));
}

/*----------------------------------------------------------------------------*/
int PRVFUNC sdata(int seq2)
{
    int x;

    if (cx || cz)
        return(0);

    x = getpkt(maxsiz);

    if (x == 0)
        return(0);

    ShowBytes(Stats.Bytes);
    return(spack('D', seq2, x, data));
}

/*----------------------------------------------------------------------------*/
int PRVFUNC seof(char *s)
{
    if (zclosi() < 0)
        return(-1);
    else {
//        if (local)
//            tmsg("OK");
        return(spack('Z', seq, strlen(s), s));
    }
}

/*----------------------------------------------------------------------------*/
int PRVFUNC getpkt(int maxlen)
{
    int i, next;
    static int c;
    static char remain[6] = "\0\0\0\0\0";

    if (first == 1) {
        first = 0;
        *remain = '\0';
        c = gnchar();
        if (c < 0) {
            first = -1;
            return(size = 0);
        }
    }
    else if (first == -1) {
        return(size = 0);
    }
    for (size = 0; (data[size] = remain [size]) != '\0'; size++);
    *remain = '\0';

    rpt = 0;
    while (first > -1) {
        next = gnchar();
        if (next < 0)
            first = -1;
        osize = size;
        encode(c, next);
        c = next;

        if (size == maxlen)
            return(size);

        if (size > maxlen) {
            for (i = 0; (remain[i] = data[osize+i]) != '\0'; i++);
            size = osize;
            data[size] = '\0';
            return(size);
        }
    }
    return(size);
}

/*----------------------------------------------------------------------------*/
int PRVFUNC gnchar (void)
{
    char c;

    if (isp)
        return((c = *isp++) > 0 ? c : -1);
    else
        return(zgetc());
}

/*----------------------------------------------------------------------------*/
void PRVFUNC encode(int a, int next)
{
    int a7, b8;

    if (rptflg) {
        if (a == next) {
            if (++rpt < 94) {
                return;
            }
            else if (rpt == 94) {
                data[size++] = (char)rptq;
                data[size++] = tochar(rpt);
                rpt = 0;
            }
        }
        else if (rpt == 1) {
            rpt = 0;
            encode(a,-1);
            if (size <= maxsiz)
                osize = size;
            rpt = 0;
            encode(a,-1);
            return;
        }
        else if (rpt > 1) {
            data[size++] = (char)rptq;
            data[size++] = tochar(++rpt);
            rpt = 0;
        }
    }
    a7 = a & 127;
    b8 = a & 128;

    if (ebqflg && b8) {
        data[size++] = (char)ebq;
        a = a7;
    }

    if (a7 < 32 || a7 == 127) {
        data[size++] = sctlq;
        a = ctl(a);
    }
    else if (a7 == sctlq)
        data[size++] = sctlq;
    else if (ebqflg && a7 == ebq)
        data[size++] = sctlq;
    else if (rptflg && a7 == rptq)
        data[size++] = sctlq;

    data[size++] = (char)a;
    data[size] = '\0';
}

/*----------------------------------------------------------------------------*/
int PRVFUNC decode(char far * rdatap2)
{
    char a, a7, b8;

    while ((a = *rdatap2++) != '\0') {
        rpt = 1;
        if (rptflg) {
            if (a == (char)rptq) {
                rpt = unchar(*rdatap2++);
                a = *rdatap2++;
            }
        }
        b8 = 0;
        if (ebqflg) {
            if (a == (char)ebq) {
                b8 = (unsigned char)128;
                a = *rdatap2++;
            }
        }
        if (a == rctlq) {
            a = *rdatap2++;
            a7 = (char)(a & 127);
            if (a7 > 62 && a7 < 96)
                a = ctl(a);
        }
        a |= b8;
        for (; rpt > 0; rpt--)
            if (pnchar(a) < 0)
                return(-1);
    }
    ShowBytes(Stats.Bytes);
    return(0);
}

/*----------------------------------------------------------------------------*/
int PRVFUNC pnchar (int c)
{
    if (xflag) {
        tchar((char)c);
        Stats.Bytes++;
        return(1);
    }
    else if (osp) {
        *osp++ = (char)c;
        return(1);
    }
    else
        return(zputc(c));
}

/*----------------------------------------------------------------------------*/
int PRVFUNC encstr(char *s)
{
    first = 1;
    isp = s;
    getpkt(spsiz);
    isp = NULL;
    return(size);
}

/*----------------------------------------------------------------------------*/
void PRVFUNC decstr(char *s)
{
    osp = s;
    decode(rdatap);
    *osp = '\0';
    osp = NULL;
}
/*----------------------------------------------------------------------------*/
void PRVFUNC debugpar(char *tag, char *s, int rln2)
{
    char buf[256];
    int bufn;

    lstrcpy(buf, tag);
    bufn = lstrlen(buf);
    s--;

    // packet size
    if (rln2 >= 1)
        bufn += wsprintf(buf+bufn, " psiz=%i", (int)unchar(s[1]));

    // timeout
    if (rln2 >= 2)
        bufn += wsprintf(buf+bufn, " timo=%i", (int)unchar(s[2]));

    // number of pad characters
    if (rln2 >= 3)
        bufn += wsprintf(buf+bufn, " padn=%i", (int)unchar(s[3]));

    // pad character
    if (rln2 >= 4)
        if (ctl(s[4]) < 32)
            bufn += wsprintf(buf+bufn, " padc='^%c'", (int)ctl(s[4] + 64));
        else
            bufn += wsprintf(buf+bufn, " padc='%c'", (int)ctl(s[4]));

    // end of line character
    if (rln2 >= 5)
        if (unchar(s[5]) < 32)
            bufn += wsprintf(buf+bufn, " eol='^%c'", (int)unchar(s[5] + 64));
        else
            bufn += wsprintf(buf+bufn, " eol='%c'", (int)unchar(s[5]));

    // control quote character
    if (rln2 >= 6)
        bufn += wsprintf(buf+bufn, " ctlq='%c'", (char)(s[6]));

    // eighth bit quote character
    if (rln2 >= 7)
        bufn += wsprintf(buf+bufn, " ebq='%c'", (char)(s[7]));

    // block check type
    if (rln2 >= 8)
        bufn += wsprintf(buf+bufn, " bct=%i", (char)(s[8] - '0'));

    // repeat quote character
    if (rln2 >= 9)
        bufn += wsprintf(buf+bufn, " rptq='%c'", (char)(s[9]));

    if (rln2 >= 10) {
        bufn += wsprintf(buf+bufn, " capb=[");
        if (unchar(s[10]) & atcapb)
            buf[bufn++] = 'A';
        if (unchar(s[10]) & lpcapb)
            buf[bufn++] = 'L';
        if (unchar(s[10]) & swcapb)
            buf[bufn++] = 'W';
        buf[bufn++] = ']';
    }

    for (capas = 10; (unchar(s[capas]) & 1) && (rln2 >= capas); capas++);

    if ((unchar(s[10]) & swcapb) && (rln2 >= capas + 1))
        bufn += wsprintf(buf+bufn, " wsiz=%i", (int)unchar(s[capas + 1]));

    if ((unchar(s[10]) & lpcapb) && (rln2 >= capas + 3))
        bufn += wsprintf(buf+bufn, " psiz=%i",
                         (int)(unchar(s[capas + 2]) * 95 + unchar(s[capas + 3])));

    tmsg(buf);
}

/*----------------------------------------------------------------------------*/
void PRVFUNC spar(char *s)
{
    int x;

    s--;

    x = (rln >= 1) ? unchar(s[1]) : 80;
    spsiz = (x < 10) ? 80 : x;

    x = (rln >= 2) ? unchar(s[2]) : 5;
    stimo = (x < 0) ? 5 : x;

    spadn = 0;
    spadc = '\0';
    if (rln >= 3) {
        spadn = unchar(s[3]);
        if (rln >= 4)
            spadc = ctl(s[4]);
        else
            spadc = 0;
    }

    seol = (char)((rln >= 5) ? unchar(s[5]) : '\r');
    if ((seol < 2) || (seol > 31))
        seol = '\r';

    x = (rln >= 6) ? s[6] : '#';
    rctlq = (char)(((x > 32 && x < 63) || (x > 95 && x < 127)) ?
                   (char)x : '#');

    rq = (rln >= 7) ? s[7] : 0;
    if (rq == 'Y')
        rqf = 1;
    else if ((rq > 32 && rq < 63) || (rq > 95 && rq < 127))
        rqf = 2;
    else
        rqf = 0;
    switch (rqf) {
        case 0:
            ebqflg = 0;
            break;
        case 1:
            if (parity) {
                ebqflg = 1;
                ebq = '&';
            }
            break;
        case 2:
            ebqflg = (ebq == sq || sq == 'Y');
            if (ebqflg)
                ebq = rq;
    }

    x = 1;
    if (rln >= 8) {
        x = s[8] - '0';
        if ((x < 1) || (x > 3))
            x = 1;
    }
    bctr = x;

    if (rln >= 9) {
        rptq = s[9];
        rptflg = ((rptq > 32 && rptq < 63) || (rptq > 95 && rptq < 127));
    }
    else
        rptflg = 0;

    atcapu = lpcapu = swcapu = 0;
    if (rln >= 10) {
        x = unchar(s[10]);
        atcapu = (x & atcapb) && atcapr;
        lpcapu = (x & lpcapb) && lpcapr;
        swcapu = (x & swcapb) && swcapr;
        for (capas = 10; (unchar(s[capas]) & 1) && (rln >= capas); capas++);
    }

    wsize = 1;
    if (swcapu && rln >= capas + 1) {
        wsize = unchar(s[capas + 1]);
        if (wsize < 1 || wsize > 31) {
            wsize = 1;
            swcapu = 0;
        }
    }

    if (lpcapu) {
        if (rln >= capas + 3) {
            x = unchar(s[capas + 2]) * 95 + unchar(s[capas + 3]);
            spsiz = x > MAXSP ? MAXSP : x;
        }
    }

    if (ProtSet.DebugOther)
        debugpar("spar:", s + 1, rln);
}

/*----------------------------------------------------------------------------*/
PSTR PRVFUNC rpar(void)
{
    data[1] = tochar(94);
    data[2] = tochar((char)rtimo);
    data[3] = tochar((char)rpadn);
    data[4] = ctl(rpadc);
    data[5] = tochar(reol);
    data[6] = '#';
    switch (rqf) {
        case -1:
        case 1:
            if (parity) ebq = sq = '&';
            break;
        case 0:
        case 2:
            break;
    }
    data[7] = (char)sq;
    data[8] = (char)(bctr + '0');
    if (rptflg)
        data[9] = (char)rptq;
    else
        data[9] = '~';

    data[10] = tochar((atcapr?atcapb:0) |
                      (lpcapr?lpcapb:0) |
                      (swcapr?swcapb:0));
    data[11] = tochar(swcapr ? wsize : 0);
    data[12] = tochar(rpsiz / 95);
    data[13] = tochar(rpsiz % 95);
    data[14] = '\0';

    if (ProtSet.DebugOther)
        debugpar("rpar:", data + 1, 14);

    return(data+1);
}

/*----------------------------------------------------------------------------*/
int PRVFUNC sattr(void)
{
    int  fh;
    char sizestr[20];
    char attrstr[20];

    if (sadone)
        return(0);

    fh = _lopen(filnam, OF_READ);
    _ltoa_s((filsiz = _filelength(fh))/1024, sizestr, sizeof(sizestr), 10);
    _lclose(fh);

    sadone = 1;
    nxtpkt();
    wsprintf(attrstr, "!%c%s", tochar(strlen(sizestr)), (LPSTR)sizestr);
    return(spack('A',seq,strlen(attrstr),attrstr));
}

/*----------------------------------------------------------------------------*/
int PRVFUNC rdattr(char * X(s))
{
    return(0);
}

/*----------------------------------------------------------------------------*/
PSTR PRVFUNC setatt(char *s)
{
    int i;
    char attrtype;
    int  attrlen;
    char attrstr[20];

    i = 0;
    while (i < rln) {
        attrtype = s[i++];
        attrlen = unchar(s[i++]);
        memset(attrstr, 0, sizeof(attrstr));
        memcpy(attrstr, s+i, attrlen);
        i += attrlen;
        switch (attrtype) {
            case '!':
                if (filsiz == 0)
                    filsiz = atol(attrstr) * 1024;
                break;
            case '1':
                filsiz = atol(attrstr);
                break;
        }
    }

    return("");
}

/*----------------------------------------------------------------------------*/
int PRVFUNC scmd(char t, char *s)
{
    encstr(s);
    return(spack(t, seq, size, data));
}

/*----------------------------------------------------------------------------*/
void PRVFUNC DoKermProt(int type)
{
    static int  filcnt;
    char        strbuf [96];

    if (type < 0) {
        ERR("sdata");
        return;
    }

    if (ProtSet.DebugState)
        tmsg("Protocol: State %i, Type: %c", state, type);

    if (ce)
        ERR("User Intervention");

    /******************************************************
    * Kermit Send state transitions...                    *
    ******************************************************/

    else if (type == 's') {
        tinit();
        if (sinit('S') < 0)
            ERR("sinit");
        else {
            filcnt = 0;
            BEGIN(ssfil);
        }
    }


    /* we expect to receive a response to our initial negotiation packet
       here.  first we parse that expected packet.  then we get and open
       the next file to be sent.  if no more, finish up.  if we successfully
       open the next file to send we build a file name packet and send it
       right away  and then we pass control to ssatr (if attributes
       has been negotiated) or to ssdat (no attributes).  the ssdat case may
       be problematic for two reasons.  1) ssdat is not expecting to parse the
       response to the file name packet, and more importantly 2) the windowing
       logic will cause the first data packet to be sent out right away without
       waiting for the resonse to the filename packet (which is in violation
       of the official protocol but in practice doesn't seem to cause much trouble.) */

    else if (state == ssfil && type == 'Y') {
        if (filcnt++ == 0)
            spar(rdatap);
        cx = 0;
        Stats.Bytes = 0;
        sadone = 0;
        bctu = bctr;
        wpktlen = spsiz;
        if (gnfile() > 0) {
            if (sfile() < 0)
                ERR("sfile");
            else if (atcapu)
                BEGIN(ssatr);
            else {
                if (winit(seq) < 0)
                    ERR("winit");
                else
                    BEGIN(ssdat);
            }
        }
        else {
            if (seot() < 0)
                ERR("seot");
            else
                BEGIN(sseot);
        }
    }

    /* we should have just received an ack to our filename packet,
       someday I should even parse this!!! */
    /* we are here because we negotiated the ability to do attribute
       packets so as a response to the ACK of the filename, we build
       and send the first attribute packet.  Control is then passed
       to ssatx because he is the guy that knows what to do with
       the responses to an attribute packet (he also may send
       additional attribute packets if he can build more).  if for
       some reason we can't build our first attribute packet, we
       just go ahead and start sending data by jumping to ssdat. */

    else if (state == ssatr && type == 'Y') {
        int x;

        if ((x = sattr()) < 0)
            ERR("sattr");
        else if (x > 0)
            BEGIN(ssatx);
        else {
            nxtpkt();
            if (winit(seq) < 0)
                ERR("winit");
            else
                BEGIN(ssdat);
        }
    }

    /* parse results of ack to previous attr pkt to start */
    /* then build and send the next attribute packet, if another can be built */
    /* if we send more attr pkts, stay in ssatx to parse results, otherwise
       start sending data using ssdat! */

    else if (state == ssatx && type == 'Y') {
        int x;

        if (rdattr(rdatap) < 0) {
            nxtpkt();
            if (seof("D") < 0)
                ERR("seof");
            else
                BEGIN(sseot);
        }
        else if ((x = sattr()) < 0)
            ERR("sattr");
        else if (x == 0) {
            if (winit(seq) < 0)
                ERR("winit");
            else {
                nxtpkt();
                BEGIN(ssdat);
            }
        }
    }

    else if (state == ssdat && type == 'Y') {
        int i;

        if (rln >= 1 && (rdatap[0] == 'X' || rdatap[0] == 'Y')) {
            if (rdatap[0] == 'X') {
                tmsg("File interrupt");
                cx = TRUE;
            }
            if (rdatap[0] == 'Z') {
                tmsg("File group interrupt");
                cz = TRUE;
            }
        }

        /* set ack flag in table */
        if (wseqintab(rsn))
            table[wseqtopos(rsn)].ack = 1;

        /* fix up "expected" pkt seq num */

        for (i = (wsize - 1); i >= 0 && table[i].ack == 1; i--);
        seq = wpostoseq(i);

        if (ProtSet.DebugOther)
            tmsg("Set Ack Pos: %i, Rsn = %i, High=%i, Low=%i, New Seq,Pos=%i,%i",
                     wseqtopos(rsn), rsn, high, low, seq, i);
    }

    else if (state == ssdat && type == 'Z') {
        if (seof((cx || cz) ? "D" : "") < 0)
            ERR("seof");
        else {
            wterm();
            BEGIN(ssfil);
        }
    }

    else if (state == sseot && type == 'Y') {
//        tmsg("Transfer Completed");
        RESUME(0);
    }

    /******************************************************
    * Kermit Receive state transitions.                   *
    ******************************************************/

    else if (type == 'v') {
        tinit();
        rinit();
        BEGIN(srini);
    }

    else if (state == srini && type == 'S') {
        spar(rdatap);
        ackl(seq, rpar());
        nxtpkt();
        bctu = bctr;
        BEGIN(srfil);
    }

    else if (state == srfil && type == 'B') {
//        tmsg("Done");
        ack(seq);
        nxtpkt();
//        tmsg("Transfer Completed");
        RESUME(0);
    }

    else if (state == srfil && type == 'F') {
        decstr(filnam);
        ack(seq);
        nxtpkt();
        BEGIN(sratt);
    }

    else if (state == sratt && type == 'A') {
        ackl(seq, setatt(rdatap));
        nxtpkt();
    }

    else if (state == sratt && type == 'D') {
        if (rcvfil() < 0)
            ERR("rcvfil");
        else {
            cx = 0;
            Stats.Bytes = 0;
            wpktlen = rpsiz;

            if (winit(seq - 1) < 0)
                ERR("winit");
            else {
                start = type;
                BEGIN(srdat);
            }
        }
    }

    else if (state == sratt && type == 'Z') {   /* Z packet here means null file */
        if (rcvfil() < 0)
            ERR("rcvfil");
        else {
            cx = 0;
            Stats.Bytes = 0;
            wpktlen = rpsiz;

            if (winit(seq - 1) < 0)
                ERR("winit");
            else {
                start = type;
                BEGIN(srdat);
            }
        }
    }

    else if (state == srdat && type == 'D') {

        /* if rsn within current table, move data and set ack;
           if data/ack already there, no harm done! */

        if (wseqintab(rsn)) {
            lstrcpy(WDATA(wseqtopos(rsn)), rdatap);
            table[wseqtopos(rsn)].ack = 1;

            if (cx)
                ackl(rsn, "X");
            else if (cz)
                ackl(rsn, "Z");
            else
                ack(rsn);
        }

        /* rsn is past end of table so we rotate the table until
           it can accept rsn or the oldest table entry is unacked */

        else while (high != rsn) {

            /* if the table can not be rotated (due to non-acked
               oldest entry) an error is triggered */

            if (table[wsize - 1].ack == 0) {
                ERR("window");
                break;
            }

            /* oldest entry has ACK, output the data, rotate table */

            if (decode(WDATA(wsize - 1)) < 0) {
                ERR("decode");
                break;
            }

            wrotate("");

            /* if the table has caught up to rsn... */

            if (high == rsn) {

                /* copy data to table, set ACK, and ACK data */

                lstrcpy(WDATA(0), rdatap);
                table[0].ack = 1;

                if (cx)
                    ackl(rsn, "X");
                else if (cz)
                    ackl(rsn, "Z");
                else
                    ack(rsn);
            }

            /* else NAK for entry we missed */

            else
                nak(high);
        }

        if (ProtSet.DebugOther) {
            tmsg("Window: High %i, Low: %i", high, low);
            tdata("  Hi=", WDATA(0), -1);
            tdata("  Lo=", WDATA(wsize - 1), -1);
        }

        /* fix up seq to be next "expected" pkt seq num */

        seq = (high + 1) & 63;
    }

    else if (state == srdat && type == 'Z') {
        int i;

        /* write out all remaining table entries */

        for (i = (wsize - 1); i >= 0; i--) {

            /* if an unACKed entry in table at this point, a
               fatal error is triggered (sender should never give
               us a 'Z' pkt until he has rcvd an ACK from us for
               all outstanding pkts! */

            if (table[i].ack == 0) {
                ERR("window");
                break;
            }

            if (decode(WDATA(i)) < 0) {
                ERR("decode");
                break;
            }
        }

        if (closof(rdatap[0] == 'D') < 0)
            ERR("closof");
        else {
            ack(seq);
            nxtpkt();
            wterm();
            BEGIN(srfil);
        }
    }

    /******************************************************
    * Kermit Client state transitions.                    *
    ******************************************************/

    else if (type == 'r') {
        tinit();
        ssc = 0;
        sinit('I');
        BEGIN(sipkt);
    }

    else if (type == 'c') {
        tinit();
        ssc = 'C';
        sinit('I');
        BEGIN(sipkt);
    }

    else if (type == 'g') {
        tinit();
        ssc = 'G';
        sinit('I');
        BEGIN(sipkt);
    }

    else if (state == sipkt && type == 'Y') {
        spar(rdatap);
        start = 'E';
    }

    else if (state == sipkt && type == 'E') {
        if (ssc) {
            if (scmd(ssc, cmarg) < 0)
                ERR("scmd");
            else
                BEGIN(srgen);
        }
        else {
            if (scmd('R', cmarg) < 0)
                ERR("scmd");
            else
                BEGIN(srini);
        }
    }

    else if (state == srgen && type == 'Y') {
        xflag = 1;
        decode(rdatap);
        RESUME(0);
    }

    else if (state == srgen && type == 'S') {
        spar(rdatap);
        ackl(seq, rpar());
        nxtpkt();
        bctu = bctr;
        BEGIN(srfil);
    }

    else if ((state == srgen || state == srfil) && type == 'X') {
        xflag = 1;
        tmsg(rdatap);
        ack(seq);
        nxtpkt();
        BEGIN(sratt);
    }

    /******************************************************
    * Kermit Server state transitions.                    *
    ******************************************************/

    else if (type == 'x') {
        SERVE;
    }

    else if (state == sserv && type == 'I') {
        spar(rdatap);
        ackl(seq, rpar());
        nxtpkt();
        seq = 0;
    }

    else if (state == sserv && type == 'R') {
        decstr(strbuf);
        nfils = 1;
        lstrcpy(filnam, strbuf);
        if (sinit('S') < 0)
            ERR("sinit");
        else {
            filcnt = 0;
            BEGIN(ssfil);
        }
    }

    else if (state == sserv && type == 'S') {
        spar(rdatap);
        ackl(seq, rpar());
        nxtpkt();
        bctu = bctr;
        BEGIN(srfil);
    }

    else if (state == sserv && type == 'G') {
        decstr(strbuf);
        start = *strbuf;
        xpkt = 1;
        BEGIN(ssgen);
    }

    else if (state == sserv && type == 'E') {
        SERVE;
    }

    else if (state == sserv) {
        ERR("Unknown Server Command");
        SERVE;
    }

    else if (state == ssgen && type == 'F') {
        ack(seq);
        nxtpkt();
        server = 0;
        RESUME(0);
    }

    else if (state == ssgen && type == 'T') {
        decstr(strbuf);
        nfils = 1;
        lstrcpy(filnam, strbuf + 2);
        if (sinit('S') < 0)
            ERR("sinit");
        else {
            filcnt = 0;
            BEGIN(ssfil);
        }
    }

    else if (state == ssgen) {
        ERR("Unknown Generic Command");
        SERVE;
    }

    /******************************************************
    * Error state transitions...                          *
    ******************************************************/

    else if (type == 'E') {
        tmsg(rdatap);
        RESUME(-1);
    }

    else if (type == 'T')
        ERR("Remote Kermit Not Responding");

    else
        ERR("Unexpected Packet Type");
}

/*----------------------------------------------------------------------------*/
void PUBFUNC DoKermit(void)
{
    int         type;

    while (!bEndKermit && (type = input()) != 0)
        DoKermProt(type);
}
