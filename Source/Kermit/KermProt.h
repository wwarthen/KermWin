/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMPROT.H                                   **
**                                                                            **
**  This include file contains the definitions, variables, and function       **
**  prototypes used during a Kermit protocol.                                 **
**                                                                            **
*******************************************************************************/

/* DEFINITIONS ---------------------------------------------------------------*/

#define MAXSP 1024
#define MAXRP 1024

#define MAXTRY 5
#define SP     32

#define KERMITSEND 's'
#define KERMITRECEIVE 'v'
#define KERMITSERVER 'x'
#define KERMITGET 'r'
#define KERMITHOST 'c'
#define KERMITGENERIC 'g'

/* Macros */
#define tochar(ch)     ((char)((ch) + SP))
#define unchar(ch)     ((int)((ch) - SP))
#define ctl(ch)        ((char)((ch) ^ 64))
#define wpostoseq(pos) ((high - (pos)) & 63)
#define wseqtopos(seq) ((high - (seq)) & 63)
#define wseqintab(seq) ((wseqtopos(seq) >= 0) && (wseqtopos(seq) <= wsize))


/* DATA STORAGE DECLARATIONS -------------------------------------------------*/
#pragma pack(2)
typedef struct {
    WORD PadChar;           /* ascii char (0-31 or 127) */
    WORD PadCount;          /* 0-99 characters */
    WORD StartChar;         /* ascii control char (0-31) */
    WORD EndChar;           /* ascii control char (0-31) */
    WORD CtlPrefix;         /* ascii char (33-126) */
} PktInfo;
#pragma pack()

#pragma pack(2)
typedef struct {
    PktInfo Send;           /* send packet parameters */
    PktInfo Recv;           /* receive packet parameters */
} PktBlk;
#pragma pack()

CLASS PktBlk      PktSet;

#pragma pack(2)
typedef struct {
    WORD SendPktSize;       /* max pkt length (20-94 or 1000) */
    WORD RecvPktSize;
    WORD SendTimeout;       /* seconds to timeout (0-99) */
    WORD RecvTimeout;
    WORD RetryLimit;        /* max retries (0-99) */
    WORD BlockCheck;        /* Block Check: 0=type 1, 1=type 2, 2=type 3 */
    WORD DebugPacket;       /* TRUE if packet debugging requested */
    WORD DebugState;        /* TRUE if state debugging requested */
    WORD DebugOther;        /* TRUE if other debugging requested */
    WORD Attributes;        /* TRUE if file attribute capability selected */
    WORD LongPackets;       /* TRUE if long packet capability selected (OBSOLETE!) */
    WORD Windowing;         /* TRUE if windowing capability selected (OBSOLETE!) */
    WORD WndSize;           /* max number of window slots allowed (1-32) */
} ProtBlk;
#pragma pack()

CLASS ProtBlk     ProtSet;

/* Kermit Protocol Negotiation */
CLASS int spsiz,        /* Send Packet Size */
          rpsiz,        /* Receive Packet Size */
          stimo,        /* Send Timeout */
          rtimo,        /* Receive Timeout */
          spadn,        /* Send Padding (Number of Characters) */
          rpadn,        /* Receive Padding (Number of Characters) */
          bctr,         /* Block Check Type Requested (Desired By Us) */
          bctu,         /* Block Check Type Used (Currently in Use) */

          ebq,          /* 8th-bit Quote Prefix */
          ebqflg,       /* 8th-bit Quoting Flag */
          rqf,          /* Request (8th-bit) Quoting Flag */
          rq,           /* Received 8th-bit Quote Bid */
          sq,           /* Sent 8th-bit Quote Bid */

          rpt,          /* Repeat Count */
          rptq,         /* Repeat Quote Prefix */
          rptflg,       /* Repeat Quoting Flag */

          wsize,        /* Sliding Window Size */
          wpktlen,      /* Sliding Window Packet Length */
          high,         /* Highest (latest) entry in window table */
          low;          /* Lowest (earliest) entry in window table */


CLASS int capas,        /* Position of Capabilities Mask */
          atcapb,       /* Attribute Capability */
          atcapr,       /*     Requested */
          atcapu,       /*     Used */
          swcapb,       /* Sliding Window Capability */
          swcapr,       /*     Requested */
          swcapu,       /*     Used */
          lpcapb,       /* Long Packet Capability */
          lpcapr,       /*     Requested */
          lpcapu;       /*     Used */

CLASS char spadc,       /* Send Pad Character */
           rpadc,       /* Receive Pad Character */
           seol,        /* Send End of Line Character */
           reol,        /* Receive End of Line Character */
           sctlq,       /* Send Control Quote Character */
           rctlq;       /* Receive Control Quote Character */

/* Packet Control */
CLASS int seq,          /* Current Packet Sequence Number */
          size,         /* Size of Current Send Packet Data */
          osize,        /* Size of Previous Send Packet Data */
          maxsiz,       /* Maximum Size of Send Packet Data Field */
          rln,          /* Length of Last Packet Received */
          rsn,          /* Sequence Number of Last Packet Received */
          limit,        /* Send Retry Limit */
          sndpkl,       /* Length of Current Send Packet */
          local,        /* TRUE if operator is at console */
          txim,         /* TRUE if chars waiting to be sent */
          delay;        /* Delay Before Send (in seconds) */

CLASS char sndpkt[MAXSP+100],   /* Send Packet */
           rcvpkt[MAXRP+100],   /* Receive Packet */
           *rdatap,             /* Pointer to Receive Packet Data Area */
           data[MAXRP+1],       /* Packet Data Buffer */
           *isp,                /* Input String Pointer */
           *osp,                /* Output String Pointer */
           smark,               /* Send Start of Packet Mark Character */
           rmark,               /* Receive Start of Packet Mark Character */
           ssc,                 /* ??? */
           cmarg[80],           /* ??? */
           state;               /* protocol state */

/* File Control */
CLASS char filnam[256];         /* Name of Current File */
CLASS long filsiz;              /* Length of Active File */

CLASS int nfils,                /* Number of Files in Send Group */
          cx,                   /* Cancel File Flag */
          cz,                   /* Cancel Batch Flag */
          cr,                   /* Operator Retry Request Flag */
          ce,                   /* Stop Transfer Flag (w/ Error) */
          cc,                   /* Abort Transfer Flag (Immediately) */
          xflag,                /* Data to Screen Flag */
          xpkt;                 /* Flag to Send X Packet */

CLASS FILE *ifp;                /* Input File Pointer */
CLASS FILE *ofp;                /* Output File Pointer */

/* Flags */
CLASS int server,               /* Server Operation Flag */
          start,                /* Preset Transaction Type */
          first,                /* First Input From File Flag */
          sadone,               /* Out of Attributes Flag */
          clrinl,               /* Clear Input Line Flag */
          parity,               /* Parity Flag (7 Bit Data) */
          keep;                 /* Keep Incomplete Files Flag */

/* Status Display */
typedef struct {
    int  Packets;               /* Number of Packets Sent */
    long Bytes;                 /* Number of Bytes Sent/Received */
    int  Retries;               /* Number of Send Retries */
} StatsBlk;

typedef struct {
    int     ack;
    int     retries;
} WinEntry;

CLASS WinEntry table [32];

CLASS HANDLE hwindata;

CLASS LPVOID pwindata;

CLASS StatsBlk Stats;

CLASS HWND hSendList;

CLASS HANDLE hFind;

CLASS int iNothing INIT(1);

CLASS BOOL bEndKermit INIT(FALSE);

/* MACRO DEFINITIONS ---------------------------------------------------------*/

#define RESUME(x) do {if (server) SERVE; else EndKermit(x);} while (iNothing != iNothing)
#define ERR(s) do {error(seq, s); RESUME(-1);} while (iNothing != iNothing)
#define BEGIN(x) do {state=x;} while (iNothing != iNothing)
#define SERVE do {tinit(); server = 1; SetDlgItemText(hWndStat, IDD_ACTION, "Server Ready"); \
                  BEGIN(sserv);} while (iNothing != iNothing)

#define WDATA(i) ((LPSTR)pwindata + ((i) * wpktlen))

/* KERMIT PROTOCOL STATES ----------------------------------------------------*/

#define ssfil 01
#define ssdat 02
#define sseot 03
#define ssatr 04
#define ssatx 05

#define srini 11
#define srfil 12
#define srdat 13
#define sratt 14

#define sipkt 21
#define srgen 22
#define sserv 23
#define ssgen 24

/* FUNCTIONS PROTOTYPES ------------------------------------------------------*/

/* System Dependent Functions (Physical Layer) */
void  PUBFUNC ttflui(void);
int   PUBFUNC ttinl(char *, int, char, int);
short PUBFUNC ttol(char *, int);
void  PUBFUNC tchar(char);
void  PUBFUNC tprog(char);
void  PUBFUNC tmsg(char *, ...);
void  PUBFUNC traw(char *s, int len);
void  PUBFUNC tdata(LPSTR lpszTag, LPSTR lpszData, int nDataLen);

/* System Dependent Functions (Presentation Layer) */
int   PUBFUNC zopeni(char *);
int   PUBFUNC zopeno(char *);
int   PUBFUNC zclosi(void);
int   PUBFUNC zcloso(int);
void  PUBFUNC zrtol(char *, char *, int);
void  PUBFUNC zltor(char *, char *);
int   PUBFUNC zgetc(void);
int   PUBFUNC zputc(int);

/* Application Layer */
void  PUBFUNC ShowRetries(int);
void  PUBFUNC ShowPackets(int);
void  PUBFUNC ShowBytes(long);
void  PUBFUNC ShowTypeIn(char);
void  PUBFUNC ShowTypeOut(char);
BOOL  CALLBACK __export StatusDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL  CALLBACK __export SendDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL  CALLBACK __export ParmDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL  PUBFUNC StartKermit(int);
BOOL  PUBFUNC EndKermit(int);

/* Presentation Layer */
int    PRVFUNC getpkt(int);
int    PRVFUNC gnchar (void);
void   PRVFUNC encode(int, int);
int    PRVFUNC decode(char far *);
int    PRVFUNC pnchar (int);
int    PRVFUNC encstr(char *);
void   PRVFUNC decstr(char *);
void   PRVFUNC spar(char *);
PSTR   PRVFUNC rpar(void);
int    PRVFUNC sattr(void);
int    PRVFUNC rdattr(char *);
PSTR   PRVFUNC setatt(char *);

/* Session Layer */

/* Transport Layer */
int    PRVFUNC input(void);
void   PRVFUNC nxtpkt (void);
void   PUBFUNC tinit(void);
void   PUBFUNC tterm(void);
void   PRVFUNC error(int, char *);
int    PRVFUNC ack(int);
int    PRVFUNC ackl(int, char *);
int    PRVFUNC nak(int);
int    PRVFUNC sinit(char);
int    PRVFUNC sfile(void);
int    PRVFUNC sdata(int);
int    PRVFUNC seof(char *);
int    PRVFUNC seot(void);
int    PRVFUNC gnfile(void);
void   PRVFUNC rinit(void);
int    PRVFUNC rcvfil(void);
int    PRVFUNC closof(void);
int    PRVFUNC scmd(char, char *);

/* Datalink Layer */
int    PRVFUNC spack(char, int, int, char far *);
int    PRVFUNC resend(void);
int    PRVFUNC chk3(char *);
int    PRVFUNC chk1(char *);
long   PRVFUNC chksum(char *);
char   PRVFUNC rpack (void);
