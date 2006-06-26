/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2004 Open Diameter Project                          */
/*                                                                        */
/* This library is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU Lesser General Public License as         */
/* published by the Free Software Foundation; either version 2.1 of the   */
/* License, or (at your option) any later version.                        */
/*                                                                        */
/* This library is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      */
/* Lesser General Public License for more details.                        */
/*                                                                        */
/* You should have received a copy of the GNU Lesser General Public       */
/* License along with this library; if not, write to the Free Software    */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307    */
/* USA.                                                                   */
/*                                                                        */
/* In addition, when you copy and redistribute some or the entire part of */
/* the source code of this software with or without modification, you     */
/* MUST include this copyright notice in each copy.                       */
/*                                                                        */
/* If you make any changes that are appeared to be useful, please send    */
/* sources that include the changed part to                               */
/* diameter-developers@lists.sourceforge.net so that we can reflect your  */
/* changes to one unified version of this software.                       */
/*                                                                        */
/* END_COPYRIGHT                                                          */
// PANAPacNetworkSupport.cpp : source file
//

#include "stdafx.h"
#include "Iphlpapi.h"
#include "PANAPacNetworkSupport.h"

wchar_t *AsciiToWide(CHAR *str)
{
   if (str == 0) {
       return 0;
   }

   UINT cp = GetACP ();
   int len = ::MultiByteToWideChar (cp, 0, str, -1, 0, 0);

   wchar_t *wstr = new wchar_t[len];

   ::MultiByteToWideChar (cp, 0, str, -1, wstr, len);
   return wstr;
}

CHAR * WideToAscii(PWSTR wstr)
{
  if ( wstr == 0 ) {
       return 0;
  }

  UINT cp = GetACP ();
  int len = ::WideCharToMultiByte (cp,
                                   0,
                                   wstr,
                                   -1,
                                   0,
                                   0,
                                   0,
                                   0);

  CHAR *str = new char[len];

  ::WideCharToMultiByte (cp, 0, wstr, -1, str, len, 0, 0);

  return str;
}

HRESULT VariantArrayToBytes
   (VARIANT Variant, 
    LPBYTE *ppBytes, 
    DWORD *pdwBytes)
{
    if(!(Variant.vt & VT_ARRAY) ||
        !Variant.parray ||
        !ppBytes ||
        !pdwBytes)
    {
        return E_INVALIDARG;
    }

    *ppBytes = NULL;
    *pdwBytes = 0;

    HRESULT hr = E_FAIL;
    SAFEARRAY *pArrayVal = NULL;
    CHAR HUGEP *pArray = NULL;
    
    // Retrieve the safe array.
    pArrayVal = Variant.parray;
    DWORD dwBytes = pArrayVal->rgsabound[0].cElements;
    *ppBytes = (LPBYTE)GlobalAlloc(GPTR, dwBytes);
    if(NULL == *ppBytes) {
        return E_OUTOFMEMORY;
    }

    hr = SafeArrayAccessData(pArrayVal, (void HUGEP * FAR *) &pArray);
    if(SUCCEEDED(hr)) {
        // Copy the bytes to the safe array.
        CopyMemory(*ppBytes, pArray, dwBytes);
        SafeArrayUnaccessData( pArrayVal );
        *pdwBytes = dwBytes;
    }
    
    return hr;
}

PANA_MonitorAccessPoint::PANA_MonitorAccessPoint() :
    m_pIWbemLocator(NULL),
    m_pWbemServices(NULL)
{
    try {
        CoInitialize(NULL);
        HRESULT hRes = CoInitializeSecurity
                              ( NULL, 
                                -1,
                                NULL,
                                NULL,
                                RPC_C_AUTHN_LEVEL_CONNECT,
                                RPC_C_IMP_LEVEL_IMPERSONATE,
                                NULL,
                                EOAC_NONE,
                                0);
        if (! SUCCEEDED(hRes)) {
            throw FALSE;
        }

        hRes = CoCreateInstance( __uuidof(WbemLocator),
                                 NULL ,
                                 CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER , 
                                 IID_IUnknown ,
                                 ( void ** ) & m_pIWbemLocator ) ;
        if (! SUCCEEDED(hRes)) {
            throw FALSE;
        }

        BSTR bstrNamespace = (L"root\\wmi");
        hRes = m_pIWbemLocator->ConnectServer
                              ( bstrNamespace, // Namespace
                                NULL, // Userid
                                NULL, // PW
                                NULL, // Locale
                                0, // flags
                                NULL, // Authority
                                NULL, // Context
                                &m_pWbemServices );
        if (! SUCCEEDED(hRes)) {
            throw FALSE;
        }
    }
    catch (...) {
        CoUninitialize();
        if (m_pIWbemLocator) {
           m_pIWbemLocator->Release();
        }
        if (m_pWbemServices) {
           m_pWbemServices->Release();
        }
    }
}

PANA_MonitorAccessPoint::~PANA_MonitorAccessPoint()
{
    if (m_pIWbemLocator) {
        m_pIWbemLocator->Release();
    }
    if (m_pWbemServices) {
        m_pWbemServices->Release();
    }
    CoUninitialize();
}

HRESULT PANA_MonitorAccessPoint::GetSSId
   (std::string &ifname,
    std::string &ssid)
{
    HRESULT hRes = E_FAIL;
    LPBYTE pBytes = NULL;
    BSTR bstrQuery = NULL;
    IEnumWbemClassObject * pEnumObject = NULL;    

    if (m_pWbemServices == NULL) {
        return hRes;
    }

    TCHAR szQuery[1024];
    sprintf(szQuery, "SELECT * FROM MSNdis_80211_ServiceSetIdentifier WHERE InstanceName=\"%s\"",
             ifname.data());
    WCHAR *wszQuery = AsciiToWide((char*)szQuery);

    try {
        bstrQuery = SysAllocString(wszQuery);
        if (bstrQuery == NULL) {
            throw hRes;
        }

        BSTR bstrQL = (L"WQL");
        hRes = m_pWbemServices->ExecQuery
                                   (bstrQL, 
                                    bstrQuery,
                                    WBEM_FLAG_RETURN_IMMEDIATELY,
                                    NULL,
                                    &pEnumObject);
        if (! SUCCEEDED(hRes)) {
            throw hRes;
        }

        hRes = pEnumObject->Reset();
        if (! SUCCEEDED(hRes)) {
            throw hRes;
        }

        ULONG uReturned;
        IWbemClassObject * pClassObject = NULL;
        hRes = pEnumObject->Next(WBEM_INFINITE, 
                                 1, 
                                 &pClassObject, 
                                 &uReturned);
        if (! SUCCEEDED(hRes)) {
            throw hRes;
        }

        VARIANT var;
        VariantInit(&var);
        hRes = pClassObject->Get(L"Ndis80211Ssid", 0, &var, 0, 0);
        if (! SUCCEEDED(hRes)) {
            throw hRes;
        }

        DWORD dwBytes = 0;
        hRes = VariantArrayToBytes(var, &pBytes, &dwBytes);
        if (! SUCCEEDED(hRes)) {
            throw hRes;
        }

        // first 4-bytes is length
        DWORD dwSsidLen = 0;
        memcpy(&dwSsidLen, pBytes, 4);
        if (dwSsidLen > 0) {
            ssid.assign((const char*)(pBytes+4), dwSsidLen);
            VariantClear(&var);
        }
        else {
            hRes = E_FAIL;
        }
        throw hRes;
    }
    catch (...) {
        if (bstrQuery) {
		    SysFreeString(bstrQuery);
        }
        if (pEnumObject) {
            pEnumObject->Release();
        }
        if (pBytes) {
            GlobalFree(pBytes);
        }
    }

    return hRes;
}

HRESULT PANA_MonitorAccessPoint::GetMaC
   (std::string &ifname,
    std::string &mac)
{
    HRESULT hRes = E_FAIL;
    LPBYTE pBytes = NULL;
    BSTR bstrQuery = NULL;
    IEnumWbemClassObject * pEnumObject = NULL;    

    if (m_pWbemServices == NULL) {
        return hRes;
    }

    TCHAR szQuery[1024];
    sprintf(szQuery, "SELECT * FROM MSNdis_80211_BaseServiceSetIdentifier WHERE InstanceName=\"%s\"",
             ifname.data());
    WCHAR *wszQuery = AsciiToWide((char*)szQuery);

    try {
        bstrQuery = SysAllocString(wszQuery);
        if (bstrQuery == NULL) {
            throw hRes;
        }

        BSTR bstrQL = (L"WQL");
        hRes = m_pWbemServices->ExecQuery
                                   (bstrQL, 
                                    bstrQuery,
                                    WBEM_FLAG_RETURN_IMMEDIATELY,
                                    NULL,
                                    &pEnumObject);
        if (! SUCCEEDED(hRes)) {
            throw hRes;
        }

        hRes = pEnumObject->Reset();
        if (! SUCCEEDED(hRes)) {
            throw hRes;
        }

        ULONG uReturned;
        IWbemClassObject * pClassObject = NULL;
        hRes = pEnumObject->Next(WBEM_INFINITE, 
                                 1, 
                                 &pClassObject, 
                                 &uReturned);
        if (! SUCCEEDED(hRes)) {
            throw hRes;
        }

        VARIANT var;
        VariantInit(&var);
        hRes = pClassObject->Get(L"Ndis80211MacAddress", 0, &var, 0, 0);
        if (! SUCCEEDED(hRes)) {
            throw hRes;
        }

        DWORD dwBytes = 0;
        hRes = VariantArrayToBytes(var, &pBytes, &dwBytes);
        if (! SUCCEEDED(hRes)) {
            throw hRes;
        }
        for (size_t i = 0; i < dwBytes; i ++) {
            CHAR hexAddr[6];
            sprintf(hexAddr, "%02X", pBytes[i]);
            mac += hexAddr;
        }
        VariantClear(&var);
        throw hRes;
    }
    catch (...) {
        if (bstrQuery) {
		    SysFreeString(bstrQuery);
        }
        if (pEnumObject) {
            pEnumObject->Release();
        }
        if (pBytes) {
            GlobalFree(pBytes);
        }
    }

    return hRes;
}

HRESULT PANA_MonitorAccessPoint::EnumerateDevices
   (PANA_SSID_LIST &SsidList)
{
    HRESULT hRes = E_FAIL;
    LPBYTE pBytes = NULL;
    BSTR bstrQuery = NULL;
    IEnumWbemClassObject * pEnumObject = NULL;    

    if (m_pWbemServices == NULL) {
        return hRes;
    }

    try {
        BSTR bstrQuery = (L"SELECT * FROM MSNdis_80211_ServiceSetIdentifier");
        BSTR bstrQL = (L"WQL");
        hRes = m_pWbemServices->ExecQuery
                                   (bstrQL, 
                                    bstrQuery,
                                    WBEM_FLAG_RETURN_IMMEDIATELY,
                                    NULL,
                                    &pEnumObject);
        if (! SUCCEEDED(hRes)) {
            throw hRes;
        }

        hRes = pEnumObject->Reset();
        if (! SUCCEEDED(hRes)) {
            throw hRes;
        }

        for (;;) {
            ULONG uReturned;
            IWbemClassObject * pClassObject = NULL;
            hRes = pEnumObject->Next(WBEM_INFINITE, 
                                     1, 
                                     &pClassObject, 
                                     &uReturned);
            if (! SUCCEEDED(hRes)) {
                hRes = S_OK;
                break;
            }

            VARIANT var;
            VariantInit(&var);
            hRes = pClassObject->Get(L"InstanceName", 0, &var, 0, 0);
            if (! SUCCEEDED(hRes)) {
                throw hRes;
            }

            WCHAR *wszName = V_BSTR(&var);
            CHAR *szName = WideToAscii(wszName);
            SsidList.push_back(std::string(szName));
            delete szName;
            VariantClear(&var);
        }
        throw hRes;
    }
    catch (...) {
        if (pEnumObject) {
            pEnumObject->Release();
        }
        if (pBytes) {
            GlobalFree(pBytes);
        }
    }

    return hRes;
}

BOOL PANA_AdapterProperties::Get(std::string &desc)
{
   PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)
       GlobalAlloc(GPTR, sizeof(IP_ADAPTER_INFO));
   ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
   // Make an initial call to GetAdaptersInfo to get
   // the necessary size into the ulOutBufLen variable
   if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
       GlobalFree(pAdapterInfo);
       pAdapterInfo = (IP_ADAPTER_INFO*)GlobalAlloc(GPTR, ulOutBufLen);
   }
   try {
       if (pAdapterInfo == NULL) {
           throw (FALSE);
       }
       GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);

       PIP_ADAPTER_INFO pIterAdapter = pAdapterInfo;
       while (pIterAdapter) {
           if (! strncmp(desc.data(), 
                         pIterAdapter->Description,
                         desc.length())) {
               char szAddrBuf[256];
               m_AdapterName = pIterAdapter->AdapterName;
               m_DhcpEnabled = pIterAdapter->DhcpEnabled;
               sprintf(szAddrBuf, "%s:0", 
                   pIterAdapter->IpAddressList.IpAddress.String);
               m_CurrentAddress.string_to_addr(szAddrBuf);
               throw (TRUE);
           }
           pIterAdapter = pIterAdapter->Next;
       }
       throw (FALSE);
   }
   catch (BOOL rc) {
       if (pAdapterInfo) {
           GlobalFree(pAdapterInfo);
       }
       return rc;
   }
}

BOOL PANA_DhcpRnewal::Get(std::string &name,
                          std::string &errMsg,
                          BOOL release)
{
    // Before calling IpReleaseAddress and IpRenewAddress we use
    // GetInterfaceInfo to retrieve a handle to the adapter
    PIP_INTERFACE_INFO pInfo;
    PIP_ADAPTER_INDEX_MAP pAdapterIndex;
    ULONG ulOutBufLen = sizeof(IP_INTERFACE_INFO);
    DWORD dwRetVal = 0;
    std::string logmsg;

    // Make an initial call to GetInterfaceInfo to get
    // the necessary size into the ulOutBufLen variable
    pInfo = (IP_INTERFACE_INFO*)GlobalAlloc(GPTR, sizeof(IP_INTERFACE_INFO));
    if (GetInterfaceInfo(pInfo, &ulOutBufLen) == ERROR_INSUFFICIENT_BUFFER) {
        GlobalFree(pInfo);
        pInfo = (IP_INTERFACE_INFO*)GlobalAlloc(GPTR, ulOutBufLen);
    }

    // Make a second call to GetInterfaceInfo to get the
    // actual data we want
    if ((dwRetVal = GetInterfaceInfo(pInfo, &ulOutBufLen)) != NO_ERROR ) {
        LPVOID lpMsgBuf;		
        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                          FORMAT_MESSAGE_FROM_SYSTEM | 
                          FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL,
                          dwRetVal,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                          (LPTSTR)&lpMsgBuf,
                          0,
                          NULL )) {
            errMsg = (LPCTSTR)lpMsgBuf;
        }
        LocalFree( lpMsgBuf );
        GlobalFree(pInfo);
        return FALSE;
    }

    for (int i = 0; i < pInfo->NumAdapters; i++) {
        pAdapterIndex = &pInfo->Adapter[i];
        ACE_Wide_To_Ascii converter(pAdapterIndex->Name);
        if (strstr(converter.convert(pAdapterIndex->Name),
                   name.data())) {
           // Call IpReleaseAddress and IpRenewAddress to release and renew
           // the IP address on the specified adapter.
           if (release && IpReleaseAddress(pAdapterIndex) != NO_ERROR) {
               errMsg = LPCTSTR("IP address release failed");
		   }

           if (IpRenewAddress(pAdapterIndex) != NO_ERROR) {
               errMsg = LPCTSTR("IP address renew failed");
			   break;
           }
           GlobalFree(pInfo);
           return TRUE;
        }
    }
    GlobalFree(pInfo);
    return FALSE;
}
