/*
 * KermOA.H
 * KermOA Automation Object #5 Chapter 14
 *
 * Classes that implement the KermOA object.
 *
 * Copyright (c)1993-1995 Microsoft Corporation, All Right Reserved
 *
 * Kraig Brockschmidt, Microsoft
 * Internet  :  kraigb@microsoft.com
 * Compuserve:  >INTERNET:kraigb@microsoft.com
 */


#ifndef _KermOA_H_
#define _KermOA_H_

#pragma warning(disable: 4201 4514 4702)

#include "KermOLE.h"

// This file is generated from MKTYPLIB
#include "IKermOA.h"

class CKermOA : public IKermOA
{
    public:
        LONG            m_cRef;         //Object reference count
        LPUNKNOWN       m_pUnkOuter;    //Controlling unknown
        long            m_lSound;       //Type of sound
        ITypeInfo      *m_pITypeInfo;   //Type information

    public:
        PCSupportErrorInfo m_pSupportErrorInfo;
        PCProvideClassInfo m_pProvideClassInfo;

    public:
        CKermOA(void);
        ~CKermOA(void);

        static CKermOA * Create(void);

        // IUnknown
        STDMETHODIMP         QueryInterface(REFIID, void FAR * FAR *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        // IDispatch required for dual automation
        STDMETHOD(GetTypeInfoCount)(UINT FAR *);
        STDMETHOD(GetTypeInfo)(UINT, LCID, ITypeInfo FAR * FAR *);
        STDMETHOD(GetIDsOfNames)(REFIID, OLECHAR FAR * FAR *, UINT, LCID, DISPID FAR *);
        STDMETHOD(Invoke)(DISPID, REFIID, LCID, WORD, DISPPARAMS FAR *,
                VARIANT FAR *, EXCEPINFO FAR *, UINT FAR *);

        // IKermOA functions
        STDMETHOD(get_Sound)(long FAR *);
        STDMETHOD(put_Sound)(long);
        STDMETHOD(Beep)(long FAR *);

        // Kermit Interface functions
        STDMETHOD(Write)(BSTR);
        STDMETHOD(Pause)(long);
        STDMETHOD(WaitFor)(BSTR, long, long FAR *);
        STDMETHOD(Send)(BSTR);

        // Standard stuff
        STDMETHOD(get_Application)(IKermOA FAR* FAR* ppIKermOA);
        STDMETHOD(get_FullName)(BSTR FAR* pbstr);
        STDMETHOD(get_Name)(BSTR FAR* pbstr);
        STDMETHOD(get_Parent)(IKermOA FAR* FAR* ppIKermOA);
        STDMETHOD(put_Visible)(boolean bVisible);
        STDMETHOD(get_Visible)(boolean * pbVisible);
        STDMETHOD(Quit)();
};

typedef CKermOA * PCKermOA;

#endif //_KermOA_H_