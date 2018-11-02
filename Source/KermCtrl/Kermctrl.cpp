/*************************************************************************
**
**    Automation Controller that uses vtable binding.
**    Controls the Kermit automation object.
**
**    kermctrl.cpp
**
**
**     Written by Microsoft Product Support Services, Windows Developer Support
**    (c) Copyright Microsoft Corp. 1994 All Rights Reserved
**
*************************************************************************/

#define STRICT

#pragma warning(disable: 4201 4514 4702)

#include <windows.h>
#include <windowsx.h>

#ifndef _WIN32
  #pragma warning(disable: 4505)
  #include <ole2.h>
  #include <dispatch.h>
  #include <variant.h>
  #include <olenls.h>
#endif

#include <initguid.h>

#include "IKermOA.h"
#include "KermCtrl.h"

// Globals
HINSTANCE g_hinst;                          // Instance of application
HWND      g_hwnd;                           // Toplevel window handle

// String resource buffers
TCHAR g_szTitle[STR_LEN];                    // Main window caption
TCHAR g_szResult[STR_LEN];                   // "Result"
TCHAR g_szError[STR_LEN];                    // "Error"

/*
 * WinMain
 *
 * Purpose:
 *  Main entry point of application. Should register the app class
 *  if a previous instance has not done so and do any other one-time
 *  initializations.
 *
 */
int APIENTRY WinMain (HINSTANCE hinst, HINSTANCE hinstPrev, LPSTR, int nCmdShow)
{
   MSG msg;

   //  It is recommended that all OLE applications set
   //  their message queue size to 96. This improves the capacity
   //  and performance of OLE's LRPC mechanism.
   int cMsg = 96;                  // Recommend msg queue size for OLE
   while (cMsg && !SetMessageQueue(cMsg))  // take largest size we can get.
       cMsg -= 8;
   if (!cMsg)
       return -1;                  // ERROR: we got no message queue

   // Load string constants
   LoadString(hinst, IDS_PROGNAME, g_szTitle, STR_LEN);
   LoadString(hinst, IDS_RESULT, g_szResult, STR_LEN);
   LoadString(hinst, IDS_ERROR, g_szError, STR_LEN);

   if (!hinstPrev)
      if (!InitApplication(hinst))
         return (FALSE);

   if(OleInitialize(NULL) != NOERROR)
      return FALSE;

   if (!InitInstance(hinst, nCmdShow))
      return (FALSE);

   while (GetMessage(&msg, NULL, NULL, NULL))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }

   OleUninitialize();

   return (msg.wParam); // Returns the value from PostQuitMessage
}

/*
 * InitApplication
 *
 * Purpose:
 *  Registers window class
 *
 * Parameters:
 *  hinst       hInstance of application
 *
 * Return Value:
 *  TRUE if initialization succeeded, FALSE otherwise.
 */
BOOL InitApplication (HINSTANCE hinst)
{
   WNDCLASS wc;

   wc.style = CS_DBLCLKS;
   wc.lpfnWndProc = MainWndProc;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = hinst;
   wc.hIcon = LoadIcon(hinst, "ControlIcon");
   wc.hCursor = LoadCursor(NULL, IDC_ARROW);
   wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
   wc.lpszMenuName = "ControlMenu";
   wc.lpszClassName = "MainWndClass";

   return RegisterClass(&wc);
 }

/*
 * InitInstance
 *
 * Purpose:
 *  Creates and shows main window
 *
 * Parameters:
 *  hinst           hInstance of application
 *  nCmdShow        specifies how window is to be shown
 *
 * Return Value:
 *  TRUE if initialization succeeded, FALSE otherwise.
 */
BOOL InitInstance (HINSTANCE hinst, int nCmdShow)
{

   g_hinst = hinst;
   // Create Main Window
   g_hwnd = CreateWindow("MainWndClass", g_szTitle,
                       WS_OVERLAPPEDWINDOW,
                       CW_USEDEFAULT, CW_USEDEFAULT,
                       400, 200,
                       NULL, NULL, hinst, NULL);
   if (!g_hwnd)
      return FALSE;

   ShowWindow(g_hwnd, nCmdShow);
   UpdateWindow(g_hwnd);
   return TRUE;
}

/*
 * MainWndProc
 *
 * Purpose:
 *  Window procedure for main window
 *
 */
LRESULT CALLBACK MainWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   static IKermOA FAR* pkermit = NULL;
   HRESULT hr;
   LPUNKNOWN punk;

   switch (msg)
   {
      case WM_COMMAND:
         switch (wParam)
         {
            case IDM_CREATEKERMIT:
                // Create Kermit object and QueryInterface for IKermit interface.
                hr = CoCreateInstance(CLSID_KermOA, NULL, CLSCTX_SERVER,
                     IID_IUnknown, (void FAR* FAR*)&punk);
                if (FAILED(hr))
                {
                    MessageBox(NULL, "CoCreateInstance", g_szError, MB_OK);
                    return 0L;
                }
                hr = punk->QueryInterface(IID_IKermOA,  (void FAR* FAR*)&pkermit);
                if (FAILED(hr))
                {
                    MessageBox(NULL, "QueryInterface(IID_IKermOA)", g_szError, MB_OK);
                    punk->Release();
                    return 0L;
                }
                punk->Release();
                return 0L;

            case IDM_SETVISIBLE:
                // Set Visible property to TRUE
                hr = pkermit->put_Visible(TRUE);
                if (FAILED(hr))
                    DisplayError(pkermit);
                return 0L;

            case IDM_SETINVISIBLE:
                // Set visible property to FALSE
                hr = pkermit->put_Visible(FALSE);
                if (FAILED(hr))
                    DisplayError(pkermit);
                return 0L;

#if 0
            case IDM_GETKERMITMESSAGE:
            {
                // Access Kermit Message property and display it
                // in a MessageBox
                BSTR bstr = NULL;   // BSTR must be intialized before passing
                                    // to get_KermitMessage.
                hr = pkermit->get_KermitMessage(&bstr);
                if (FAILED(hr))
                    DisplayError(pkermit);
                else MessageBox(NULL, FROM_OLE_STRING(bstr), g_szResult, MB_OK);

                // Caller is responsible for freeing parameters and return values.
                if (bstr)
                    SysFreeString(bstr);
                return 0L;
            }
#endif

            case IDM_SAYHELLO:
            {
                BSTR bstr = NULL;   // BSTR must be intialized before passing
                // Invoke SayHello method
                bstr = SysAllocString(TO_OLE_STRING("Hello"));
                hr = pkermit->Write(bstr);
                if (FAILED(hr))
                    DisplayError(pkermit);
                SysFreeString(bstr);
                return 0L;
            }

            case IDM_RELEASEKERMIT:
                // Release the Kermit object
                pkermit->Release();
                pkermit = NULL;
                return 0L;

            case IDM_QUITKERMIT:
                // Release the Kermit object
                pkermit->Quit();
                pkermit = NULL;
                return 0L;
         }
         break;

      case WM_INITMENUPOPUP:
      {
         HMENU hmenu = (HMENU)wParam;

         if (LOWORD(lParam) != 0)
            return 0L;

         // Enable or gray the appropriate menu items. pkermit indicates if an automation object
         //  is currently being controlled.
         EnableMenuItem(hmenu, IDM_CREATEKERMIT,  MF_BYCOMMAND | (pkermit?MF_GRAYED:MF_ENABLED));
         EnableMenuItem(hmenu, IDM_SETVISIBLE,   MF_BYCOMMAND | (pkermit?MF_ENABLED:MF_GRAYED));
         EnableMenuItem(hmenu, IDM_SETINVISIBLE,   MF_BYCOMMAND | (pkermit?MF_ENABLED:MF_GRAYED));
         EnableMenuItem(hmenu, IDM_GETKERMITMESSAGE,   MF_BYCOMMAND | (pkermit?MF_ENABLED:MF_GRAYED));
         EnableMenuItem(hmenu, IDM_SAYHELLO,  MF_BYCOMMAND | (pkermit?MF_ENABLED:MF_GRAYED));
         EnableMenuItem(hmenu, IDM_RELEASEKERMIT, MF_BYCOMMAND | (pkermit?MF_ENABLED:MF_GRAYED));
         EnableMenuItem(hmenu, IDM_QUITKERMIT, MF_BYCOMMAND | (pkermit?MF_ENABLED:MF_GRAYED));
         return 0L;
      }

      case WM_DESTROY:
         if (pkermit)
            pkermit->Release();
         PostQuitMessage(0);
         break;

      default:
         return DefWindowProc(hwnd, msg, wParam, lParam);
   }

   return NULL;
}

/*
 * DisplayError
 *
 * Purpose:
 *  Obtains Rich Error Information about the automation error from
 *  the IErrorInfo interface.
 *
 */
void DisplayError(IKermOA FAR* pkermit)
{
   IErrorInfo FAR* perrinfo;
   BSTR bstrDesc;
   HRESULT hr;
   ISupportErrorInfo FAR* psupporterrinfo;

   hr = pkermit->QueryInterface(IID_ISupportErrorInfo, (LPVOID FAR*)&psupporterrinfo);
   if (FAILED(hr))
   {
      MessageBox(NULL, "QueryInterface(IID_ISupportErrorInfo)", g_szError, MB_OK);
      return;
   }

   hr = psupporterrinfo->InterfaceSupportsErrorInfo(IID_IKermOA);
   if (hr != NOERROR)
   {
       psupporterrinfo->Release();
       return;
   }
   psupporterrinfo->Release();

   // In this example only the error description is obtained and displayed.
   // See the IErrorInfo interface for other information that is available.
   hr = GetErrorInfo(0, &perrinfo);
   if (FAILED(hr))
       return;
   hr = perrinfo->GetDescription(&bstrDesc);
   if (FAILED(hr))
   {
       perrinfo->Release();
       return;
   }

   MessageBox(NULL, FROM_OLE_STRING(bstrDesc), g_szError, MB_OK);
   SysFreeString(bstrDesc);
}

#ifdef _WIN32

#ifndef UNICODE
char* ConvertToAnsi(OLECHAR FAR* szW)
{
  static char achA[STRCONVERT_MAXLEN];

  WideCharToMultiByte(CP_ACP, 0, szW, -1, achA, STRCONVERT_MAXLEN, NULL, NULL);
  return achA;
}

OLECHAR* ConvertToUnicode(char FAR* szA)
{
  static OLECHAR achW[STRCONVERT_MAXLEN];

  MultiByteToWideChar(CP_ACP, 0, szA, -1, achW, STRCONVERT_MAXLEN);
  return achW;
}
#endif

#endif
