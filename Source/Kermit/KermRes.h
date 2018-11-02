/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                                KERMRES.H                                   **
**                                                                            **
**  This is the resources include file for the application.  It contains      **
**  the global definitions used to define and access resources.               **
**                                                                            **
*******************************************************************************/

/* MENU RESOURCE CONSTANTS ---------------------------------------------------*/

#define IDM_TOPMENU             100
#define IDM_POPUPS              110

#define IDM_FILE                IDM_TOPMENU + 1
#define IDM_EDIT                IDM_TOPMENU + 2
#define IDM_SESS                IDM_TOPMENU + 3
#define IDM_KERMIT              IDM_TOPMENU + 4
#define IDM_CONFIGURE           IDM_TOPMENU + 5
#define IDM_HELP                IDM_TOPMENU + 6
#define IDM_DEBUG               IDM_TOPMENU + 7

#define IDM_NEW                 IDM_POPUPS + 1
#define IDM_OPEN                IDM_POPUPS + 2
#define IDM_SAVE                IDM_POPUPS + 3
#define IDM_SAVEAS              IDM_POPUPS + 4
#define IDM_SCRIPT              IDM_POPUPS + 5
#define IDM_EXEC                IDM_POPUPS + 6
#define IDM_DBGROPEN            IDM_POPUPS + 7
#define IDM_DBGRBREAK           IDM_POPUPS + 8
#define IDM_EXIT                IDM_POPUPS + 9
#define IDM_COPY                IDM_POPUPS + 10
#define IDM_PASTE               IDM_POPUPS + 11
#define IDM_REVIEW              IDM_POPUPS + 12
#define IDM_CONNECT             IDM_POPUPS + 13
#define IDM_BREAK               IDM_POPUPS + 14
#define IDM_PRINT               IDM_POPUPS + 15
#define IDM_TRANSMIT            IDM_POPUPS + 16
#define IDM_CAPTURE             IDM_POPUPS + 17
#define IDM_SEND                IDM_POPUPS + 18
#define IDM_RECEIVE             IDM_POPUPS + 19
#define IDM_SERVER              IDM_POPUPS + 20
#define IDM_GET                 IDM_POPUPS + 21
#define IDM_HOST                IDM_POPUPS + 22
#define IDM_GENERIC             IDM_POPUPS + 23
#define IDM_CANFILE             IDM_POPUPS + 24
#define IDM_CANBATCH            IDM_POPUPS + 25
#define IDM_STOP                IDM_POPUPS + 26
#define IDM_ABORT               IDM_POPUPS + 27
#define IDM_RETRY               IDM_POPUPS + 28
#define IDM_SESSION             IDM_POPUPS + 29
#define IDM_TERMINAL            IDM_POPUPS + 30
#define IDM_COMMUNICATIONS      IDM_POPUPS + 31
#define IDM_DEVICE              IDM_POPUPS + 32
#define IDM_PROTOCOL            IDM_POPUPS + 33
#define IDM_LOGGING             IDM_POPUPS + 34
#define IDM_FONT                IDM_POPUPS + 35
#define IDM_HELPCTX             IDM_POPUPS + 36
#define IDM_HELPSRCH            IDM_POPUPS + 37
#define IDM_HELPHELP            IDM_POPUPS + 38
#define IDM_ABOUT               IDM_POPUPS + 39
#define IDM_DEBDCB              IDM_POPUPS + 40
#define IDM_DEBTEST1            IDM_POPUPS + 41
#define IDM_DEBTEST2            IDM_POPUPS + 42
#define IDM_DEBTEST3            IDM_POPUPS + 43
#define IDM_DEBTEST4            IDM_POPUPS + 44
#define IDM_DEBTEST5            IDM_POPUPS + 45
#define IDM_DEBTEST6            IDM_POPUPS + 46

/* DIALOG RESOURCE CONSTANTS -------------------------------------------------*/

#define IDD_OK                  IDOK
#define IDD_CANCEL              IDCANCEL
#define IDD_ABORT               IDABORT
#define IDD_RETRY               IDRETRY
#define IDD_IGNORE              IDIGNORE
#define IDD_YES                 IDYES
#define IDD_NO                  IDNO

#define IDD_VERSION             197
#define IDD_SAVE                198
#define IDD_INFO                199

#define IDD_NAME                200
#define IDD_PATH                201
#define IDD_AT                  202
#define IDD_USERID              203
#define IDD_PASSWORD            204
#define IDD_CONNLIST            205
#define IDD_EMULLIST            206
#define IDD_PROTLIST            207

#define IDD_HOST                208
#define IDD_PORT                209

#define IDD_BAUDRATE            210
#define IDD_4DATABITS           211
#define IDD_5DATABITS           212
#define IDD_6DATABITS           213
#define IDD_7DATABITS           214
#define IDD_8DATABITS           215
#define IDD_NOPTY               216
#define IDD_ODDPTY              217
#define IDD_EVENPTY             218
#define IDD_MARKPTY             219
#define IDD_SPACEPTY            220
#define IDD_1STOPBIT            221
#define IDD_1HSTOPBITS          222
#define IDD_2STOPBITS           223
#define IDD_FLOWCTL             224
#define IDD_HANDSHK             225
#define IDD_COM1                226
#define IDD_COM2                227
#define IDD_COM3                228
#define IDD_COM4                229

#define IDD_SPKTSIZE            230
#define IDD_RPKTSIZE            231
#define IDD_STIMEOUT            232
#define IDD_RTIMEOUT            233
#define IDD_SLIMIT              234
#define IDD_BCHK1               235
#define IDD_BCHK2               236
#define IDD_BCHK3               237
#define IDD_PKTDBG              238
#define IDD_STADBG              239
#define IDD_OTHDBG              240
#define IDD_ATRCAP              241
#define IDD_WNDSIZE             242
#define IDD_OPTIONS             243

#define IDD_SPADCHR             244
#define IDD_RPADCHR             245
#define IDD_SPADCNT             246
#define IDD_RPADCNT             247
#define IDD_SPKTSOP             248
#define IDD_RPKTSOP             249
#define IDD_SPKTEOL             250
#define IDD_RPKTEOL             251
#define IDD_SCTLPFX             252
#define IDD_RCTLPFX             253

#define IDD_LOGSESFLG           254
#define IDD_LOGSESFIL           255
#define IDD_LOGPKTFLG           256
#define IDD_LOGPKTFIL           257
#define IDD_LOGTRNFLG           258
#define IDD_LOGTRNFIL           259
#define IDD_LOGDBGFLG           260
#define IDD_LOGDBGFIL           261

#define IDD_SENDCNT             262
#define IDD_FILE                263
#define IDD_SENDLIST            264
#define IDD_DELETE              265
#define IDD_CURDIR              266
#define IDD_FILELIST            267
#define IDD_DIRLIST             268
#define IDD_SEND                269
#define IDD_ADD                 270
#define IDD_LIST                271

#define IDD_PARMS               272
#define IDD_ACTION              273
#define IDD_PACKETS             274
#define IDD_TYPEIN              275
#define IDD_TYPEOUT             276
#define IDD_BYTES               277
#define IDD_RETRIES             278
#define IDD_MESSAGE             279

#define IDD_GCMDI               280
#define IDD_GCMDC               281
#define IDD_GCMDL               282
#define IDD_GCMDF               283
#define IDD_GCMDD               284
#define IDD_GCMDU               285
#define IDD_GCMDE               286
#define IDD_GCMDT               287
#define IDD_GCMDR               288
#define IDD_GCMDK               289
#define IDD_GCMDW               290
#define IDD_GCMDM               291
#define IDD_GCMDH               292
#define IDD_GCMDQ               293
#define IDD_GCMDP               294
#define IDD_GCMDJ               295
#define IDD_GCMDV               296
#define IDD_PARM1               297
#define IDD_PARM2               298
#define IDD_PARM3               299

#define IDD_NUMBER              300
#define IDD_STATUS              301

#define IDD_LOCAL               302
#define IDD_DIAL                303
#define IDD_ANSWER              304
#define IDD_NUBMER              305
#define IDD_DIALINIT            306
#define IDD_DIALCMD             307
#define IDD_ANSINIT             308
#define IDD_HANGUP              309
#define IDD_ESCAPE              310
#define IDD_CONNECT             311
#define IDD_DIALCNT             312
#define IDD_DIALWAIT            313
#define IDD_ANSWAIT             314

#define IDD_CONNMSG             315
#define IDD_CONNSTAT            316

#define IDD_SENDCRLF            317
#define IDD_SUPNEG              318
#define IDD_TERMTYPE            319

#define IDD_EDIT                320
#define IDD_BROWSE              321
#define IDD_RUN                 322

/* STRING TABLE CONSTANTS ----------------------------------------------------*/

#define IDS_COMMERRBASE     1000

#define IDS_WSAERRBASE      2000
