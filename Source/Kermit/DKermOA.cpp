/*
 * DKermOA.CPP
 * KermOA Automation Object Chapter 14
 *
 * Server module code for the KermOA object.
 *
 * Copyright (c)1993-1995 Microsoft Corporation, All Rights Reserved
 *
 * Kraig Brockschmidt, Microsoft
 * Internet  :  kraigb@microsoft.com
 * Compuserve:  >INTERNET:kraigb@microsoft.com
 */


#include "kermit.h"

#define INITGUIDS
#include "DKermOA.h"

CKermOAFactory::CKermOAFactory(void)
{
    m_cRef=0L;
    return;
}


CKermOAFactory::~CKermOAFactory(void)
{
    return;
}






/*
 * CKermOAFactory::QueryInterface
 * CKermOAFactory::AddRef
 * CKermOAFactory::Release
 */

STDMETHODIMP CKermOAFactory::QueryInterface(REFIID riid, void FAR * FAR * ppv)
{
    *ppv=NULL;

    if (IID_IUnknown==riid || IID_IClassFactory==riid)
        *ppv=this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG) CKermOAFactory::AddRef(void)
{
    m_cRef++;

//  KermitFmtMsgBox(MB_OK, "KermOAFactory AddRef, ref now %i", m_cRef);

    return m_cRef;
}


STDMETHODIMP_(ULONG) CKermOAFactory::Release(void)
{
    m_cRef--;

//  KermitFmtMsgBox(MB_OK, "KermOAFactory Release, ref now %i", m_cRef);

    if (m_cRef != 0)
        return m_cRef;

    delete this;

    g_pKermOAF = NULL;

//  KermitFmtMsgBox(MB_OK, "KermOAFactory destroyed");

    return m_cRef;
}



/*
 * CKermOAFactory::CreateInstance
 * CKermOAFactory::LockServer
 */

STDMETHODIMP CKermOAFactory::CreateInstance(LPUNKNOWN pUnkOuter
                                            , REFIID riid, void FAR * FAR * ppvObj)
{
    *ppvObj=NULL;

    //Verify that a controlling unknown asks for IUnknown
    if (NULL!=pUnkOuter && IID_IUnknown!=riid)
        return ResultFromScode(CLASS_E_NOAGGREGATION);

    if (g_pKermOA == NULL)
        return ResultFromScode(E_OUTOFMEMORY);

    return(((IUnknown *)g_pKermOA)->QueryInterface(riid, ppvObj));
}

STDMETHODIMP CKermOAFactory::LockServer(BOOL fLock)
{
    CoLockObjectExternal((IUnknown *)g_pKermOA, fLock, TRUE);
    return NOERROR;
}

CKermOAFactory * CKermOAFactory::Create(void * pKermOA)
{
    HRESULT         hr;
    CKermOAFactory * pKermOAF;

//  KermitFmtMsgBox(MB_OK, "Creating KermOAFactory");

    pKermOAF = new CKermOAFactory;

    if (pKermOAF == NULL)
    {
        KermitFmtMsgBox(MB_OK, "new CKermOAFactory Failed!");
        return NULL;
    }

    /* CoRegisterClassObject calls AddRef and keeps OAF in mem. */

    dwRegisterCF = 0;

    hr = CoRegisterClassObject(CLSID_KermOA, pKermOAF,
        CLSCTX_LOCAL_SERVER, REGCLS_SINGLEUSE,
        &dwRegisterCF);

    if (FAILED(hr))
    {
        KermitFmtMsgBox(MB_OK, "CoRegisterClassObject() Failed!");
        ((IUnknown *)g_pKermOAF)->Release();

        return NULL;
    }

    dwRegisterActiveObject = 0;

#ifdef _WIN32
    hr = RegisterActiveObject((CKermOA *)pKermOA, CLSID_KermOA, ACTIVEOBJECT_WEAK,
        &dwRegisterActiveObject);
#else
    hr = RegisterActiveObject((CKermOA *)pKermOA, CLSID_KermOA, NULL,
        &dwRegisterActiveObject);
#endif

    if (FAILED(hr))
    {
        KermitFmtMsgBox(MB_OK, "RegisterActiveObject() Failed!");
        CoRevokeClassObject(dwRegisterCF);
        dwRegisterCF = 0;
        ((IUnknown *)g_pKermOAF)->Release();

        return NULL;
    }

    return pKermOAF;
}