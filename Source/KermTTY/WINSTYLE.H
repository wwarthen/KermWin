/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               WINSTYLE.H                                   **
**                                                                            **
**  This file contains constants to make it easier to define the window       **
**  styles for controls in dialog boxes.                                      **
**                                                                            **
*******************************************************************************/

#define GRP        WS_GROUP
#define TAB        WS_TABSTOP
#define DIS        WS_DISABLED
#define GRPTAB     WS_GROUP | WS_TABSTOP
#define GRPDIS     WS_GROUP | WS_DISABLED
#define TABDIS     WS_TABSTOP | WS_DISABLED
#define GRPTABDIS  WS_GROUP | WS_TABSTOP | WS_DISABLED

#define AHS        ES_AUTOHSCROLL
#define ACB        BS_AUTOCHECKBOX
#define MSL        LBS_MULTIPLESEL
