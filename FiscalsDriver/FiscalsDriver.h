

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 04:14:07 2038
 */
/* Compiler settings for C:\svn\FiscalsDriverGK\FiscalsDriver\FiscalsDriver.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.01.0628 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __FiscalsDriver_h__
#define __FiscalsDriver_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __IFiscalInterface_FWD_DEFINED__
#define __IFiscalInterface_FWD_DEFINED__
typedef interface IFiscalInterface IFiscalInterface;

#endif 	/* __IFiscalInterface_FWD_DEFINED__ */


#ifndef __FiscalInterface_FWD_DEFINED__
#define __FiscalInterface_FWD_DEFINED__

#ifdef __cplusplus
typedef class FiscalInterface FiscalInterface;
#else
typedef struct FiscalInterface FiscalInterface;
#endif /* __cplusplus */

#endif 	/* __FiscalInterface_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IFiscalInterface_INTERFACE_DEFINED__
#define __IFiscalInterface_INTERFACE_DEFINED__

/* interface IFiscalInterface */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IFiscalInterface;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4DA4BA91-C2A7-4401-B7A3-0C04B5CC4E5C")
    IFiscalInterface : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE initFiscal( 
            BSTR fiscalName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE open( 
            BSTR strPort) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE close( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PrintNonFiscal( 
            /* [in] */ BSTR XML) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE check_Fiscal( 
            /* [in] */ BSTR *XML) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PrintFiscal( 
            /* [in] */ BSTR XML) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OpenDefault( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getProperties( 
            BSTR *xml_properties) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IFiscalInterfaceVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFiscalInterface * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFiscalInterface * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFiscalInterface * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFiscalInterface * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFiscalInterface * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFiscalInterface * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFiscalInterface * This,
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
        
        DECLSPEC_XFGVIRT(IFiscalInterface, initFiscal)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *initFiscal )( 
            IFiscalInterface * This,
            BSTR fiscalName);
        
        DECLSPEC_XFGVIRT(IFiscalInterface, open)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *open )( 
            IFiscalInterface * This,
            BSTR strPort);
        
        DECLSPEC_XFGVIRT(IFiscalInterface, close)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *close )( 
            IFiscalInterface * This);
        
        DECLSPEC_XFGVIRT(IFiscalInterface, PrintNonFiscal)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *PrintNonFiscal )( 
            IFiscalInterface * This,
            /* [in] */ BSTR XML);
        
        DECLSPEC_XFGVIRT(IFiscalInterface, check_Fiscal)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *check_Fiscal )( 
            IFiscalInterface * This,
            /* [in] */ BSTR *XML);
        
        DECLSPEC_XFGVIRT(IFiscalInterface, PrintFiscal)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *PrintFiscal )( 
            IFiscalInterface * This,
            /* [in] */ BSTR XML);
        
        DECLSPEC_XFGVIRT(IFiscalInterface, OpenDefault)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OpenDefault )( 
            IFiscalInterface * This);
        
        DECLSPEC_XFGVIRT(IFiscalInterface, getProperties)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *getProperties )( 
            IFiscalInterface * This,
            BSTR *xml_properties);
        
        END_INTERFACE
    } IFiscalInterfaceVtbl;

    interface IFiscalInterface
    {
        CONST_VTBL struct IFiscalInterfaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFiscalInterface_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFiscalInterface_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFiscalInterface_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFiscalInterface_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IFiscalInterface_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IFiscalInterface_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IFiscalInterface_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IFiscalInterface_initFiscal(This,fiscalName)	\
    ( (This)->lpVtbl -> initFiscal(This,fiscalName) ) 

#define IFiscalInterface_open(This,strPort)	\
    ( (This)->lpVtbl -> open(This,strPort) ) 

#define IFiscalInterface_close(This)	\
    ( (This)->lpVtbl -> close(This) ) 

#define IFiscalInterface_PrintNonFiscal(This,XML)	\
    ( (This)->lpVtbl -> PrintNonFiscal(This,XML) ) 

#define IFiscalInterface_check_Fiscal(This,XML)	\
    ( (This)->lpVtbl -> check_Fiscal(This,XML) ) 

#define IFiscalInterface_PrintFiscal(This,XML)	\
    ( (This)->lpVtbl -> PrintFiscal(This,XML) ) 

#define IFiscalInterface_OpenDefault(This)	\
    ( (This)->lpVtbl -> OpenDefault(This) ) 

#define IFiscalInterface_getProperties(This,xml_properties)	\
    ( (This)->lpVtbl -> getProperties(This,xml_properties) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IFiscalInterface_INTERFACE_DEFINED__ */



#ifndef __FiscalsDriverLib_LIBRARY_DEFINED__
#define __FiscalsDriverLib_LIBRARY_DEFINED__

/* library FiscalsDriverLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_FiscalsDriverLib;

EXTERN_C const CLSID CLSID_FiscalInterface;

#ifdef __cplusplus

class DECLSPEC_UUID("19010E45-D0C0-4B36-AE7C-EBA7028B1FFD")
FiscalInterface;
#endif
#endif /* __FiscalsDriverLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  BSTR_UserSize64(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal64(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal64(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree64(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


