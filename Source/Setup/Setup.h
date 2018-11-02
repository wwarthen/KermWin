/*******************************************************************************
**                                                                            **
**                      Setup  for Microsoft Windows                          **
**                      ----------------------------                          **
**                                SETUP.H                                     **
**                                                                            **
**  This is the primary include file for the application.  It contains        **
**  global definitions and variables.  It also contains global function       **
**  prototypes.                                                               **
**                                                                            **
*******************************************************************************/

/* DEFINITIONS ---------------------------------------------------------------*/


#define STRICT

#define X(x)
#ifdef _WIN32
#define X32(x)
#define X16(x) x
#else
#define X32(x) x
#define X16(x)
#endif

#define PUBFUNC     FAR
#define PRVFUNC     static NEAR

#ifdef _WIN32
#define __export
#define DLLIMPORT __declspec(dllexport)
#pragma warning(disable: 4201 4514 4702)
#else
#define DLLIMPORT __export
#endif

/* INCLUDES ------------------------------------------------------------------*/

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "setupver.h"

/* VARIABLE DECLARATIONS -----------------------------------------------------*/

typedef char FILNAM [16];
typedef char PATHNAM [256];
