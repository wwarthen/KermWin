

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0620 */
/* at Mon Jan 18 19:14:07 2038
 */
/* Compiler settings for C:\Users\Wayne\Projects\Kermit\Source\Kermit\ikermoa.odl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.01.0620 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */


#ifndef __ikermoa_h__
#define __ikermoa_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IKermOA_FWD_DEFINED__
#define __IKermOA_FWD_DEFINED__
typedef interface IKermOA IKermOA;

#endif 	/* __IKermOA_FWD_DEFINED__ */


#ifndef __KermOA_FWD_DEFINED__
#define __KermOA_FWD_DEFINED__

#ifdef __cplusplus
typedef class KermOA KermOA;
#else
typedef struct KermOA KermOA;
#endif /* __cplusplus */

#endif 	/* __KermOA_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __KermOATypeLibrary_LIBRARY_DEFINED__
#define __KermOATypeLibrary_LIBRARY_DEFINED__

/* library KermOATypeLibrary */
/* [version][lcid][helpstring][uuid] */ 


EXTERN_C const IID LIBID_KermOATypeLibrary;

#ifndef __IKermOA_INTERFACE_DEFINED__
#define __IKermOA_INTERFACE_DEFINED__

/* interface IKermOA */
/* [object][dual][helpstring][uuid] */ 


EXTERN_C const IID IID_IKermOA;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("cd322b21-0d7b-11d0-b92a-00a024a6f710")
    IKermOA : public IDispatch
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IKermOA **pRet) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_FullName( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IKermOA **pRet) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ boolean VisibleFlag) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ boolean *pRet) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Quit( void) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Sound( 
            /* [retval][out] */ long *pRet) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Sound( 
            /* [in] */ long lSound) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Beep( 
            /* [retval][out] */ long *pRet) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Write( 
            /* [in] */ BSTR bstrText) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Pause( 
            /* [in] */ long lTime) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE WaitFor( 
            /* [in] */ BSTR bstrText,
            /* [in] */ long lTime,
            /* [retval][out] */ long *pRet) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Send( 
            /* [in] */ BSTR bstrText) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IKermOAVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IKermOA * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IKermOA * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IKermOA * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IKermOA * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IKermOA * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IKermOA * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IKermOA * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Application )( 
            IKermOA * This,
            /* [retval][out] */ IKermOA **pRet);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FullName )( 
            IKermOA * This,
            /* [retval][out] */ BSTR *pRet);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IKermOA * This,
            /* [retval][out] */ BSTR *pRet);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IKermOA * This,
            /* [retval][out] */ IKermOA **pRet);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            IKermOA * This,
            /* [in] */ boolean VisibleFlag);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            IKermOA * This,
            /* [retval][out] */ boolean *pRet);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Quit )( 
            IKermOA * This);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Sound )( 
            IKermOA * This,
            /* [retval][out] */ long *pRet);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_Sound )( 
            IKermOA * This,
            /* [in] */ long lSound);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Beep )( 
            IKermOA * This,
            /* [retval][out] */ long *pRet);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Write )( 
            IKermOA * This,
            /* [in] */ BSTR bstrText);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IKermOA * This,
            /* [in] */ long lTime);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *WaitFor )( 
            IKermOA * This,
            /* [in] */ BSTR bstrText,
            /* [in] */ long lTime,
            /* [retval][out] */ long *pRet);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Send )( 
            IKermOA * This,
            /* [in] */ BSTR bstrText);
        
        END_INTERFACE
    } IKermOAVtbl;

    interface IKermOA
    {
        CONST_VTBL struct IKermOAVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IKermOA_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IKermOA_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IKermOA_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IKermOA_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IKermOA_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IKermOA_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IKermOA_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IKermOA_get_Application(This,pRet)	\
    ( (This)->lpVtbl -> get_Application(This,pRet) ) 

#define IKermOA_get_FullName(This,pRet)	\
    ( (This)->lpVtbl -> get_FullName(This,pRet) ) 

#define IKermOA_get_Name(This,pRet)	\
    ( (This)->lpVtbl -> get_Name(This,pRet) ) 

#define IKermOA_get_Parent(This,pRet)	\
    ( (This)->lpVtbl -> get_Parent(This,pRet) ) 

#define IKermOA_put_Visible(This,VisibleFlag)	\
    ( (This)->lpVtbl -> put_Visible(This,VisibleFlag) ) 

#define IKermOA_get_Visible(This,pRet)	\
    ( (This)->lpVtbl -> get_Visible(This,pRet) ) 

#define IKermOA_Quit(This)	\
    ( (This)->lpVtbl -> Quit(This) ) 

#define IKermOA_get_Sound(This,pRet)	\
    ( (This)->lpVtbl -> get_Sound(This,pRet) ) 

#define IKermOA_put_Sound(This,lSound)	\
    ( (This)->lpVtbl -> put_Sound(This,lSound) ) 

#define IKermOA_Beep(This,pRet)	\
    ( (This)->lpVtbl -> Beep(This,pRet) ) 

#define IKermOA_Write(This,bstrText)	\
    ( (This)->lpVtbl -> Write(This,bstrText) ) 

#define IKermOA_Pause(This,lTime)	\
    ( (This)->lpVtbl -> Pause(This,lTime) ) 

#define IKermOA_WaitFor(This,bstrText,lTime,pRet)	\
    ( (This)->lpVtbl -> WaitFor(This,bstrText,lTime,pRet) ) 

#define IKermOA_Send(This,bstrText)	\
    ( (This)->lpVtbl -> Send(This,bstrText) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IKermOA_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_KermOA;

#ifdef __cplusplus

class DECLSPEC_UUID("cd322b22-0d7b-11d0-b92a-00a024a6f710")
KermOA;
#endif
#endif /* __KermOATypeLibrary_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


