/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                                KERMC3D.H                                    **
**                                                                            **
*******************************************************************************/

BOOL FAR C3DInit(void);
BOOL FAR C3DTerm(void);
BOOL FAR C3DRegister(HINSTANCE);
BOOL FAR C3DUnregister(HINSTANCE);
BOOL FAR C3DColorChange(void);
BOOL FAR C3DAutoSubclass(HINSTANCE);
PSTR FAR C3DStatus(void);
