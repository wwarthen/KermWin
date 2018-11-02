/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMTERM.C                                   **
**                                                                            **
**  This module contains the terminal emulation default procedures.  These    **
**  routines are invoked if the emulation DLL does not provide them.          **
**                                                                            **
*******************************************************************************/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include "kermtloc.h"
#include "kermtapi.h"
#include "kermtxfc.h"
#include "kermtdef.h"

/*----------------------------------------------------------------------------*/
int PUBFUNC DefSetupTerm(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    MessageBeep(0);
    return(0);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC DefWriteTerm(LPSTR TermStr, int StrLen, BOOL X(Raw))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int  i;
    int  nCol, nRow;
    char TermChar;

    LocGetCurPos(&nCol, &nRow);

    for (i = 0; i < StrLen; i++) {
        TermChar = TermStr[i];

        if (TermChar >= 32) {
            if (nCol >= TERMCOLS)
                nCol = TERMCOLS - 1;

            LocWriteScrnChar(nCol++, nRow, TermChar);
            if (nCol >= TERMCOLS)
                    nCol = TERMCOLS - 1;

            continue;
        }

        switch (TermChar) {
            case 7:                     /* bell */
                MessageBeep(0);
                break;

            case 8:                     /* backspace */
                if (nCol > 0)
                    (nCol)--;
                break;

            case 9:                     /* tab */
                nCol = min(TERMCOLS - 1, nCol + 8 - (nCol % 8));
                break;

            case 10:                    /* line feed */
                if (nRow < TERMROWS - 1)
                    nRow++;
                else
                    LocScrollScrn(0, 1, NULL);
                break;

            case 12:                    /* form feed */
                LocFillScrn(NULL, ' ');
                nRow = 0;
                nCol = 0;
                break;

            case 13:                    /* carriage return */
                nCol = 0;
                break;
        }
    }

    LocFlushScrnBuf();
    LocSetCurPos(nCol, nRow, FALSE);
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC DefOpenTerm(LPSTR X(lpConfig), FARPROC FAR * X(CBProcs))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC DefCloseTerm(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC DefGetTermConfig(LPSTR X(lpConfig))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    return(FALSE);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC DefProcessTermChar(char cChar)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    LocSendTermChar(cChar);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC DefProcessTermKey(UINT X(cKey))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC DefDoKeyMap(KEYMAPENTRY FAR * X(lpkme))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    return FALSE;
}
