/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMTAPI.C                                   **
**                                                                            **
**  This module contains the terminal emulation program interface functaions. **
**                                                                            **
*******************************************************************************/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include "kermtloc.h"
#include "kermtapi.h"

/* LOCAL VARIABLES -----------------------------------------------------------*/

FARPROC APIProcs[APIPROCCNT];

FARPROC APIFuncs[APIPROCCNT] = {(FARPROC)ApiSendTermChar,
                (FARPROC)ApiSendTermStr,
                (FARPROC)ApiFlushScrnBuf,
                (FARPROC)ApiWriteScrnChar,
                (FARPROC)ApiWriteScrnStr,
                (FARPROC)ApiScrollScrn,
                (FARPROC)ApiFillScrn,
                (FARPROC)ApiFillAttr,
                (FARPROC)ApiSetCurPos,
                (FARPROC)ApiGetCurPos,
                (FARPROC)ApiDrawEmul,
                (FARPROC)ApiSetAttr,
                (FARPROC)ApiSetTermSize,
                (FARPROC)ApiSetLineAttr,
                (FARPROC)ApiGetLineAttr,
                (FARPROC)ApiSetVideoMode,
                (FARPROC)ApiKeyMapClear,
                (FARPROC)ApiKeyMapAdd,
                (FARPROC)ApiKeyMapParse};

/* PUBLIC FUNCTIONS ----------------------------------------------------------*/

VOID CALLBACK __export ApiSendTermChar(char cTermChar)
{
    LocSendTermChar(cTermChar);
}

VOID CALLBACK __export ApiSendTermStr(LPSTR lpszTermStr, int nStrLen)
{
    LocSendTermStr(lpszTermStr, nStrLen);
}

VOID CALLBACK __export ApiFlushScrnBuf(VOID)
{
    LocFlushScrnBuf();
}

VOID CALLBACK __export ApiWriteScrnChar(int xPos, int yPos, char ScrnChar)
{
    LocWriteScrnChar(xPos, yPos, ScrnChar);
}

VOID CALLBACK __export ApiWriteScrnStr(int xPos, int yPos, LPSTR lpsScrnStr, int nLength)
{
    LocWriteScrnStr(xPos, yPos, lpsScrnStr, nLength);
}

VOID CALLBACK __export ApiScrollScrn(int xAmount, int yAmount, LPRECT lpRect)
{
    LocScrollScrn(xAmount, yAmount, lpRect);
}

VOID CALLBACK __export ApiFillScrn(LPRECT lpRect, BYTE cChar)
{
    LocFillScrn(lpRect, cChar);
}

VOID CALLBACK __export ApiFillAttr(LPRECT lpRect, BYTE cAttr)
{
    LocFillAttr(lpRect, cAttr);
}

VOID CALLBACK __export ApiSetCurPos(int nCol, int nRow, BOOL bRel)
{
    LocSetCurPos(nCol, nRow, bRel);
}

VOID CALLBACK __export ApiGetCurPos(LPINT lpnCol, LPINT lpnRow)
{
    LocGetCurPos(lpnCol, lpnRow);
}

VOID CALLBACK __export ApiDrawEmul(LPSTR lpszEmul)
{
    LocDrawEmul(lpszEmul);
}

VOID CALLBACK __export ApiSetAttr(BYTE cNewAttr)
{
    LocSetAttr(cNewAttr);
}

VOID CALLBACK __export ApiSetTermSize(int nRows, int nCols)
{
    LocSetTermSize(nRows, nCols);
}

VOID CALLBACK __export ApiSetLineAttr(int Row, BYTE cLineAttr)
{
    LocSetLineAttr(Row, cLineAttr);
}

BYTE CALLBACK __export ApiGetLineAttr(int Row)
{
    return LocGetLineAttr(Row);
}

VOID CALLBACK __export ApiSetVideoMode(int nMode)
{
    LocSetVideoMode(nMode);
}

VOID CALLBACK __export ApiKeyMapClear(VOID)
{
    LocKeyMapClear();
}

BOOL CALLBACK __export ApiKeyMapAdd(KEYMAP FAR * lpkm)
{
    return LocKeyMapAdd(lpkm);
}

BOOL CALLBACK __export ApiKeyMapParse(LPSTR lpszKeyMapFile)
{
    return LocKeyMapParse(lpszKeyMapFile);
}

VOID PUBFUNC InitApiXfc(HINSTANCE X32(hInst))
{
    int i;

    for (i = 0; i < APIPROCCNT; i++)
    APIProcs[i] = MakeProcInstance(APIFuncs[i], hInst);
}

FARPROC FAR * PUBFUNC GetApiProcTable(VOID)
{
    return(&APIProcs[0]);
}
