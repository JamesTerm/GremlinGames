

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Sat Feb 28 15:08:16 2009
 */
/* Compiler settings for .\osgvAXMFC.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

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

MIDL_DEFINE_GUID(IID, LIBID_osgvAXMFCLib,0x6DF742EE,0xF06E,0x4292,0xBF,0x28,0xAD,0xF2,0x92,0xA7,0xB8,0xF9);


MIDL_DEFINE_GUID(IID, DIID__DosgvAXMFC,0x75C2661F,0x87DB,0x47D6,0x96,0xB4,0xE2,0x93,0xF8,0xC4,0x39,0xAD);


MIDL_DEFINE_GUID(IID, DIID__DosgvAXMFCEvents,0xA09B39CD,0xC74A,0x433C,0x88,0xBC,0x7D,0xDF,0xDA,0x2E,0xED,0x51);


MIDL_DEFINE_GUID(CLSID, CLSID_osgvAXMFC,0x59451646,0xABD9,0x4DE4,0x88,0xA9,0x65,0x55,0x24,0x82,0x8B,0xB5);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



