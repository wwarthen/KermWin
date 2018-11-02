/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMC3D.CPP                                  **
**                                                                            **
**  This module contains the functions to interface with the Windows          **
**  interupt driven communications driver.                                    **
**                                                                            **
*******************************************************************************/

/* DEFINES -------------------------------------------------------------------*/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include "kermc3d.h"

/* MACROS --------------------------------------------------------------------*/

#define OSVERMAJOR (LOBYTE(LOWORD(GetVersion())))
#define OSVERMINOR (HIBYTE(LOWORD(GetVersion())))
#define OSVERBUILD (HIWORD(GetVersion()) & 0x7FFF)

/* CONSTANTS -----------------------------------------------------------------*/

#define C3DDLLPROCCNT           5

/* WINSOCK DLL FUNCTION TABLE INDEXES ----------------------------------------*/

#define nCtl3dRegister      0
#define nCtl3dUnregister    1
#define nCtl3dColorChange   2
#define nCtl3dAutoSubclass  3
#define nCtl3dGetVer        4

/* CTL3D DLL FUNCTION DEFINITIONS --------------------------------------------*/

typedef BOOL (CALLBACK * tagCtl3dRegister)(HINSTANCE);
typedef BOOL (CALLBACK * tagCtl3dUnregister)(HINSTANCE);
typedef BOOL (CALLBACK * tagCtl3dColorChange)(void);
typedef BOOL (CALLBACK * tagCtl3dAutoSubclass)(HINSTANCE);
typedef WORD (CALLBACK * tagCtl3dGetVer)(void);

/* CTL3D DLL FUNCTION CALL MACROS --------------------------------------------*/

#define Ctl3dRegister (*(tagCtl3dRegister)(C3DProcs[nCtl3dRegister]))
#define Ctl3dUnregister (*(tagCtl3dUnregister)(C3DProcs[nCtl3dUnregister]))
#define Ctl3dColorChange (*(tagCtl3dColorChange)(C3DProcs[nCtl3dColorChange]))
#define Ctl3dAutoSubclass (*(tagCtl3dAutoSubclass)(C3DProcs[nCtl3dAutoSubclass]))
#define Ctl3dGetVer (*(tagCtl3dGetVer)(C3DProcs[nCtl3dGetVer]))

/* LOCAL DATA ----------------------------------------------------------------*/

static HINSTANCE hCtl3dLib = NULL;

static FARPROC C3DProcs[C3DDLLPROCCNT];
static char *  C3DFuncs[C3DDLLPROCCNT] = {
    "Ctl3dRegister",
    "Ctl3dUnregister",
    "Ctl3dColorChange",
    "Ctl3dAutoSubclass",
    "Ctl3dGetVer"};

static char szC3DStatus [80] = "Ctl3D Unloaded";

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC C3DTerm(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int i;

    for (i = 0; i < C3DDLLPROCCNT; i++)
        C3DProcs[i] = NULL;

    if (hCtl3dLib != NULL) {
        FreeLibrary(hCtl3dLib);
        hCtl3dLib = NULL;
    }

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC C3DInit(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int i;
    char * pszC3DLibFile;
    WORD wC3DVer;

  #pragma warning(push)
  #pragma warning(disable: 4996)

    if (OSVERMAJOR == 0)
    {
        lstrcpy(szC3DStatus, "Ctl3D not loaded, OS version id error");
//      MessageBox(NULL, szC3DStatus, "KermC3D Status", MB_OK | MB_ICONASTERISK);

        return FALSE;
    }

    if (OSVERMAJOR >= 4)
    {
        lstrcpy(szC3DStatus, "Ctl3D not loaded, not required by OS");
//      MessageBox(NULL, szC3DStatus, "KermC3D Status", MB_OK | MB_ICONASTERISK);

    return TRUE;
  }

  #pragma warning(pop)

#ifdef _WIN32
    pszC3DLibFile = "CTL3D32.DLL";
#else
    pszC3DLibFile = "CTL3DV2.DLL";
#endif

    {
        OFSTRUCT of;
        char szC3DLibPath[80];

        GetSystemDirectory(szC3DLibPath, sizeof(szC3DLibPath));
        lstrcat(szC3DLibPath, "\\");
        lstrcat(szC3DLibPath, pszC3DLibFile);

        if (OpenFile(szC3DLibPath, &of, OF_EXIST) == HFILE_ERROR)
        {
            wsprintf(szC3DStatus, "Ctl3D not loaded, can't locate %s", (LPSTR)pszC3DLibFile);
//          MessageBox(NULL, szC3DStatus, "KermC3D Status", MB_OK | MB_ICONASTERISK);

            return FALSE;
        }
    }

//  KermitFmtMsgBox(MB_OK, "File %s found", (LPSTR)pszC3DLibFile);

    hCtl3dLib = LoadLibrary(pszC3DLibFile);

#ifdef _WIN32
    if (hCtl3dLib == NULL)
#else
    if (hCtl3dLib < HINSTANCE_ERROR)
#endif
    {
#ifdef _WIN32
        wsprintf(szC3DStatus, "Ctl3D load failed, error %lu", (DWORD)GetLastError());
#else
        wsprintf(szC3DStatus, "Ctl3D load failed, error %i", (int)hCtl3dLib);
#endif
        hCtl3dLib = NULL;
//      MessageBox(NULL, szC3DStatus, "KermC3D Status", MB_OK | MB_ICONASTERISK);
        return FALSE;
    }

//  KermitFmtMsgBox(MB_OK, "Ctl3D Library loaded!");

    for (i = 0; i < C3DDLLPROCCNT; i++) {
        C3DProcs[i] = GetProcAddress(hCtl3dLib, C3DFuncs[i]);

        if (C3DProcs[i] == NULL) {
            char szMsgBuf [80];

            wsprintf(szMsgBuf, "Can't get ProcAddress of function %s in Ctl3d library!", (LPSTR)C3DFuncs[i]);
            MessageBox(NULL, szMsgBuf, "KermC3D Error", MB_OK | MB_ICONASTERISK);
//            KermitFmtMsgBox(MB_OK | MB_ICONASTERISK,
//                            "Can't get ProcAddress of function %s in "
//                            "Ctl3d library!", (LPSTR)C3DFuncs[i]);
        }
    }

    wC3DVer = Ctl3dGetVer();

    wsprintf(szC3DStatus, "Ctl3D loaded %s, Version %i.%i",
        (LPSTR)pszC3DLibFile, (int)HIBYTE(wC3DVer), (int)LOBYTE(wC3DVer));

//  MessageBox(NULL, szC3DStatus, "KermC3D Status", MB_OK | MB_ICONASTERISK);
    return TRUE;
}

BOOL PUBFUNC C3DRegister(HINSTANCE hInstance)
{
    if (C3DProcs[nCtl3dRegister] == NULL)
        return FALSE;
    else
        return Ctl3dRegister(hInstance);
}

BOOL PUBFUNC C3DUnregister(HINSTANCE hInstance)
{
    if (C3DProcs[nCtl3dUnregister] == NULL)
        return FALSE;
    else
        return Ctl3dUnregister(hInstance);
}

BOOL PUBFUNC C3DColorChange(void)
{
    if (C3DProcs[nCtl3dColorChange] == NULL)
        return FALSE;
    else
        return Ctl3dColorChange();
}

BOOL PUBFUNC C3DAutoSubclass(HINSTANCE hInstance)
{
    if (C3DProcs[nCtl3dAutoSubclass] == NULL)
        return FALSE;
    else
        return Ctl3dAutoSubclass(hInstance);
}

PSTR PUBFUNC C3DStatus(void)
{
    return szC3DStatus;
}
