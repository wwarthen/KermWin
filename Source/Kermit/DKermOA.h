/*
 * DKermOA.H
 * KermOA Automation Object Chapter 14
 *
 * Definitions, classes, and prototypes for the server of KermOA
 * objects.
 *
 * Copyright (c)1993-1995 Microsoft Corporation, All Rights Reserved
 *
 * Kraig Brockschmidt, Microsoft
 * Internet  :  kraigb@microsoft.com
 * Compuserve:  >INTERNET:kraigb@microsoft.com
 */


#ifndef _DKermOA_H_
#define _DKermOA_H_


//Get the object definitions
#include "KermOA.h"


//DKermOA.CPP
void ObjectDestroyed(void);

//This class factory object creates other objects

class CKermOAFactory : public IClassFactory
    {
    protected:
        ULONG           m_cRef;

    public:
        CKermOAFactory(void);
        ~CKermOAFactory(void);

        static CKermOAFactory * Create(void *);

        //IUnknown members
        STDMETHODIMP         QueryInterface(REFIID, void FAR * FAR *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        //IClassFactory members
        STDMETHODIMP     CreateInstance(LPUNKNOWN, REFIID, void FAR * FAR *);
        STDMETHODIMP     LockServer(BOOL);
    };

typedef CKermOAFactory *PCKermOAFactory;

#endif //_DKermOA_H_