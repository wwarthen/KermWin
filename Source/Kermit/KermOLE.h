#ifndef _KermOLE_H_
#define _KermOLE_H_

#ifndef _WIN32
#pragma warning(disable: 4505)
#endif

/* Main OLE include file */

#include <ole2.h>

/* Cause real GUID definition if requested (once and only once per app) */

#ifdef INITGUIDS
#include <initguid.h>
#endif

/* OLE Automation (required for OLE Controls) */

#ifndef _WIN32
#include <olenls.h>
#include <dispatch.h>
#endif

/* OLE Controls (required for scripting) */

#include <olectl.h>

/* ActiveX scripting stuff (only possible for WIN32?) */

#ifdef AXSCRIPT
#pragma warning(push)
#pragma warning(disable: 4268)
#include <ActivScp.h>
#pragma warning(pop)
#include <ActivDbg.h>
//#include "VBSGuids.h"
#endif

//Types that OLE2.H et. al. leave out

#ifndef PPVOID
typedef LPVOID * PPVOID;
#endif  //PPVOID


#ifndef PPOINTL
typedef POINTL * PPOINTL;
#endif  //PPOINTL


#ifndef _WIN32
#ifndef OLECHAR
typedef char OLECHAR;
typedef OLECHAR FAR* LPOLESTR;
typedef const OLECHAR FAR* LPCOLESTR;
#endif //OLECHAR
#endif //_WIN32


//DeleteInterfaceImp calls 'delete' and NULLs the pointer
#define DeleteInterfaceImp(p)\
            {\
            if (NULL!=p)\
                {\
                delete p;\
                p=NULL;\
                }\
            }


//ReleaseInterface calls 'Release' and NULLs the pointer
#define ReleaseInterface(p)\
            {\
            if (NULL!=p)\
                {\
                p->Release();\
                p=NULL;\
                }\
            }


#ifdef _WIN32

#define OLETOANSI(s, d, cch) WideCharToMultiByte(CP_ACP \
            , 0, s, -1, d, cch, NULL, NULL)
#define ANSITOOLE(s, d, cch) MultiByteToWideChar(CP_ACP \
            , 0, s, -1, d, cch)

#define OLIT(x) L ## x

#else

#define OLETOANSI(s, d, cch) lstrcpy(d, (OLECHAR FAR *)s);
#define ANSITOOLE(s, d, cch) lstrcpy(d, (OLECHAR FAR *)s);

#define OLIT(x) x

#endif

class CSupportErrorInfo : public ISupportErrorInfo
{
    public:
        static CSupportErrorInfo * Create(LPUNKNOWN, REFIID);
        CSupportErrorInfo(void);

        // IUnknown members that delegate to m_pUnkOuter.
        STDMETHODIMP         QueryInterface(REFIID, void FAR * FAR *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        // ISupportErrorInfo methods
        STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    private:
        LPUNKNOWN   m_pUnkOuter;  // IUnknown of Object that implements this interface
        GUID        m_iid;
};

typedef CSupportErrorInfo * PCSupportErrorInfo;

class CProvideClassInfo : public IProvideClassInfo
{
    public:
        static CProvideClassInfo * Create(LPUNKNOWN, ITypeInfo *);
        CProvideClassInfo(void);
        ~CProvideClassInfo(void);

        // IUnknown members that delegate to m_pUnkOuter.
        STDMETHODIMP         QueryInterface(REFIID, void FAR * FAR *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        // IProvideClassInfo methods
        STDMETHODIMP GetClassInfo(ITypeInfo FAR * FAR *);

    private:
        LPUNKNOWN   m_pUnkOuter;
        ITypeInfo   *m_pITI;     //To return from GetClassInfo
};

typedef CProvideClassInfo * PCProvideClassInfo;

#endif