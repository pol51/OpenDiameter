

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 6.00.0361 */
/* Compiler settings for netprov.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IProvisioningDomain,0xc96fbd50,0x24dd,0x11d8,0x89,0xfb,0x00,0x90,0x4b,0x2e,0xa9,0xc6);


MIDL_DEFINE_GUID(IID, IID_IProvisioningProfileWireless,0xc96fbd51,0x24dd,0x11d8,0x89,0xfb,0x00,0x90,0x4b,0x2e,0xa9,0xc6);


MIDL_DEFINE_GUID(IID, IID_IFlashConfig,0x8244dedf,0xcf04,0x41fa,0x81,0x2f,0xe1,0x51,0xf4,0x92,0xc5,0xaa);


MIDL_DEFINE_GUID(IID, LIBID_NETPROVLib,0x789ed9b3,0x4873,0x49b9,0x87,0xe0,0x55,0xf5,0x1f,0xcd,0x54,0x20);


MIDL_DEFINE_GUID(CLSID, CLSID_NetProvisioning,0x2aa2b5fe,0xb846,0x4d07,0x81,0x0c,0xb2,0x1e,0xe4,0x53,0x20,0xe3);


MIDL_DEFINE_GUID(CLSID, CLSID_FlashConfig,0x9f63805a,0x42a7,0x4472,0xba,0xbc,0x64,0x24,0x66,0xc9,0x1d,0x59);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/



/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 6.00.0361 */
/* Compiler settings for netprov.idl:
    Oicf, W1, Zp8, env=Win64 (32b run,appending)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#if defined(_M_IA64) || defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IProvisioningDomain,0xc96fbd50,0x24dd,0x11d8,0x89,0xfb,0x00,0x90,0x4b,0x2e,0xa9,0xc6);


MIDL_DEFINE_GUID(IID, IID_IProvisioningProfileWireless,0xc96fbd51,0x24dd,0x11d8,0x89,0xfb,0x00,0x90,0x4b,0x2e,0xa9,0xc6);


MIDL_DEFINE_GUID(IID, IID_IFlashConfig,0x8244dedf,0xcf04,0x41fa,0x81,0x2f,0xe1,0x51,0xf4,0x92,0xc5,0xaa);


MIDL_DEFINE_GUID(IID, LIBID_NETPROVLib,0x789ed9b3,0x4873,0x49b9,0x87,0xe0,0x55,0xf5,0x1f,0xcd,0x54,0x20);


MIDL_DEFINE_GUID(CLSID, CLSID_NetProvisioning,0x2aa2b5fe,0xb846,0x4d07,0x81,0x0c,0xb2,0x1e,0xe4,0x53,0x20,0xe3);


MIDL_DEFINE_GUID(CLSID, CLSID_FlashConfig,0x9f63805a,0x42a7,0x4472,0xba,0xbc,0x64,0x24,0x66,0xc9,0x1d,0x59);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif /* defined(_M_IA64) || defined(_M_AMD64)*/

