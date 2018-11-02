/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMTXFC.C                                   **
**                                                                            **
**  This module contains the terminal emulation program interface functaions. **
**                                                                            **
*******************************************************************************/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include "kermtxfc.h"
#include "kermtloc.h"
#include "kermtdef.h"
#include "kermtapi.h"

/* CONSTANTS -----------------------------------------------------------------*/

#define DLLPROCCNT       8

/* EMULATION DLL FUNCTION TABLE INDEXES --------------------------------------*/

#define nSetupTerm       0
#define nWriteTerm       1
#define nOpenTerm        2
#define nCloseTerm       3
#define nGetTermConfig   4
#define nProcessTermChar 5
#define nProcessTermKey  6
#define nDoKeyMap        7

/* EMULATION DLL FUNCTION DEFINITIONS ----------------------------------------*/

typedef int     (CALLBACK *tagSetupTerm)(HANDLE);
typedef VOID    (CALLBACK *tagWriteTerm)(HANDLE, LPSTR, int, BOOL);
typedef HANDLE  (CALLBACK *tagOpenTerm)(HWND, LPSTR, FARPROC FAR *);
typedef VOID    (CALLBACK *tagCloseTerm)(HANDLE);
typedef BOOL    (CALLBACK *tagGetTermConfig)(HANDLE, LPSTR);
typedef VOID    (CALLBACK *tagProcessTermChar)(HANDLE, char);
typedef VOID    (CALLBACK *tagProcessTermKey)(HANDLE, UINT);
typedef BOOL    (CALLBACK *tagDoKeyMap)(HANDLE, KEYMAPENTRY FAR *);

/* EMULATION DLL FUNCTION CALL MACROS ----------------------------------------*/

#define EmuSetupTerm (*(tagSetupTerm)(DLLProcs[nSetupTerm]))
#define EmuWriteTerm (*(tagWriteTerm)(DLLProcs[nWriteTerm]))
#define EmuOpenTerm (*(tagOpenTerm)(DLLProcs[nOpenTerm]))
#define EmuCloseTerm (*(tagCloseTerm)(DLLProcs[nCloseTerm]))
#define EmuGetTermConfig (*(tagGetTermConfig)(DLLProcs[nGetTermConfig]))
#define EmuProcessTermChar (*(tagProcessTermChar)(DLLProcs[nProcessTermChar]))
#define EmuProcessTermKey (*(tagProcessTermKey)(DLLProcs[nProcessTermKey]))
#define EmuDoKeyMap (*(tagDoKeyMap)(DLLProcs[nDoKeyMap]))

/* LOCAL STORAGE -------------------------------------------------------------*/

HINSTANCE  hLibrary = NULL;
HANDLE     hEmul = NULL;

FARPROC DLLProcs[DLLPROCCNT];
char *  DLLFuncs[DLLPROCCNT] = {"SetupTerm",
                                "WriteTerm",
                                "OpenTerm",
                                "CloseTerm",
                                "GetTermConfig",
                                "ProcessTermChar",
                                "ProcessTermKey",
                                "DoKeyMap"};

static char    szCurModule [64] = "";

/*******************************************************************************
*                                                                              *
* Emulation Interface Layer (invokes DLL or default emulation routine)         *
*                                                                              *
*******************************************************************************/

/*----------------------------------------------------------------------------*/
VOID PUBFUNC SetupTerm(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (DLLProcs[nSetupTerm] == NULL)
        DefSetupTerm();
    else
        EmuSetupTerm(hEmul);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC WriteTerm(LPSTR TermStr, int StrLen, BOOL Raw)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (DLLProcs[nWriteTerm] == NULL)
        DefWriteTerm(TermStr, StrLen, Raw);
    else
        EmuWriteTerm(hEmul, TermStr, StrLen, Raw);
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC OpenTerm(LPSTR lpConfig)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (DLLProcs[nOpenTerm] == NULL)
        DefOpenTerm(lpConfig, GetApiProcTable());
    else
        hEmul = EmuOpenTerm(hAppWnd, lpConfig, GetApiProcTable());

    LocFillScrn(NULL, ' ');
    LocSetCurPos(0, 0, FALSE);

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC CloseTerm(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (DLLProcs[nCloseTerm] == NULL)
        DefCloseTerm();
    else
        EmuCloseTerm(hEmul);
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC GetTermConfig(LPSTR lpConfig)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (DLLProcs[nGetTermConfig] == NULL)
        return(DefGetTermConfig(lpConfig));
    else
        return(EmuGetTermConfig(hEmul, lpConfig));
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC ProcessTermChar(char cChar)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (DLLProcs[nProcessTermChar] == NULL)
        DefProcessTermChar(cChar);
    else
        EmuProcessTermChar(hEmul, cChar);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC ProcessTermKey(UINT cKey)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (DLLProcs[nProcessTermKey] == NULL)
        DefProcessTermKey(cKey);
    else
        EmuProcessTermKey(hEmul, cKey);
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC DoKeyMap(KEYMAPENTRY FAR * lpkme)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    BOOL bResult;

    if (DLLProcs[nDoKeyMap] == NULL)
        bResult = DefDoKeyMap(lpkme);
    else
        bResult = EmuDoKeyMap(hEmul, lpkme);

    return bResult;
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC ClearEmulXfc(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int i;

    for (i = 0; i < DLLPROCCNT; i++)
        DLLProcs[i] = NULL;

    if (hLibrary) {
        FreeLibrary(hLibrary);
        hLibrary = NULL;
    }
}

/*----------------------------------------------------------------------------*/
BOOL PRVFUNC InitEmulXfc(HINSTANCE hInst, PSTR pszModule)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    PATHNAM szFileName;
    int i;

    GetModuleFileName(hInst, szFileName, sizeof(szFileName));
    *(strrchr(szFileName, '\\') + 1) = '\0';
    lstrcat(szFileName, pszModule);
    lstrcat(szFileName, ".TRM");

    hLibrary = LoadLibrary(szFileName);
#ifdef _WIN32
    if (hLibrary == NULL) {
#else
    if (hLibrary < HINSTANCE_ERROR) {
#endif
        hLibrary = NULL;
        return(FALSE);
    }

    for (i = 0; i < DLLPROCCNT; i++)
        DLLProcs[i] = GetProcAddress(hLibrary, DLLFuncs[i]);

    return(TRUE);
}

BOOL PUBFUNC EMInit(VOID)
{
    InitApiXfc(hAppInst);

    ClearEmulXfc();

    OpenTerm(NULL);

//    LocDrawEmul("No Emulation");

    return(TRUE);
}

BOOL PUBFUNC EMUnload(VOID)
{
    CloseTerm();
    LocKeyMapClear();

    ClearEmulXfc();

    lstrcpy(szCurModule, "");

    LocDrawEmul("No Emulation");

    return(TRUE);
}

BOOL PUBFUNC EMLoad(PSTR pszModule, BOOL bReset)
{
    if (!bReset && strcmp(szCurModule, pszModule) == 0)
        return TRUE;

    EMUnload();

    lstrcpy(szCurModule, pszModule);

    if (!InitEmulXfc(hAppInst, pszModule))
        return(FALSE);

    LocDrawEmul("Unknown Emulation");
    LocSetTermSize(25, 80);

    OpenTerm(NULL);

    return(TRUE);
}

BOOL PUBFUNC EMSetConfig(UINT X(wInfoSize), PSTR pszInfo)
{
    CloseTerm();
    LocKeyMapClear();
    OpenTerm(pszInfo);

    return(TRUE);
}

BOOL PUBFUNC EMGetConfig(UINT * pwInfoSize, PSTR pszInfo)
{
    char sConfigData[128];

    GetTermConfig(sConfigData);

    *pwInfoSize = sizeof(sConfigData);
    memcpy(pszInfo, sConfigData, sizeof(sConfigData));

    return(TRUE);
}

BOOL PUBFUNC EMSetup(HINSTANCE X(hInst), HWND X(hWnd))
{
    SetupTerm();

    bChanged = TRUE;

    return(TRUE);
}
