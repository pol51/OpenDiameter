#include <netcfgx.h> // REQUIRES WINDOWS DDK
#include <devguid.h>
#include <stdio.h>
#include "netprov.h"
#include "wps_load.h"


HRESULT 
FindNICGuids ( 
    OUT LPGUID *ppGuids,
    OUT PDWORD pdwNumGuids,
    IN LPWSTR pszwName
    )
/*++

Routine Description:

    This function will retrieve the device GUID for the wireless adapter.

Parameters:

    ppGuids
        [out] Receives the GUID array.

    pdwNumGuids
        [out] Receives the number of GUIDs in the array.

    pszwName
        [in] Adapter display name.

Return Values:

    If the function succeeds, then the return value is S_OK. Otherwise, the
    return value is the HRESULT indicating the failure.

Warning:

    This function uses the INetCfg interface which is included only in the
    Windows DDK, but not in the SDK.

--*/
{
    HRESULT hrStatus = S_OK ;
    LPGUID pGuids = NULL ;
    DWORD dwNumGuids = 0 ;
    INetCfg *pNC = NULL ;

    hrStatus = CoCreateInstance (
        CLSID_CNetCfg,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_INetCfg,
        ( PVOID * ) &pNC
        ) ;

    if ( hrStatus == S_OK ) 
    {
        hrStatus = pNC->Initialize( NULL ) ;

        if ( hrStatus == S_OK )
        {
            IEnumNetCfgComponent *pEnumNCC = NULL ;
            GUID gNetGuid = GUID_DEVCLASS_NET ;

            hrStatus = pNC->EnumComponents( &gNetGuid, &pEnumNCC ) ;

            if ( hrStatus == S_OK )
            {
                INetCfgComponent *pNCC = NULL ;

                while ( pEnumNCC->Next( 1, &pNCC, NULL ) == S_OK )
                {
                    DWORD dwCharacteristics = 0 ;
                    LPWSTR ppszwDisplayName;

                    if (pNCC->GetDisplayName(&ppszwDisplayName) == S_OK)
                    {
                        if (wcsncmp(ppszwDisplayName, pszwName, wcslen(pszwName)))
                        {
                            pNCC->Release();
                            CoTaskMemFree(ppszwDisplayName);
                            continue;
                        }
                    }
                    else
                    {
                        pNCC->Release();
                        continue;
                    }

                    if ( pNCC->GetCharacteristics( &dwCharacteristics ) == S_OK )
                    {
                        if ( dwCharacteristics & NCF_PHYSICAL )
                        {
                            wprintf(L"Found Device: %s\n", ppszwDisplayName);

                            if ( pGuids != NULL )
                            {
                                pGuids = ( LPGUID ) HeapReAlloc (
                                    GetProcessHeap( ),
                                    HEAP_ZERO_MEMORY,
                                    pGuids,
                                    ++dwNumGuids * sizeof( GUID )
                                    ) ;
                            }
                            else
                            {
                                pGuids = ( LPGUID ) HeapAlloc ( 
                                    GetProcessHeap( ), 
                                    HEAP_ZERO_MEMORY, 
                                    ++dwNumGuids * sizeof( GUID )
                                    ) ;
                            }

                            if ( pGuids != NULL )
                            {
                                hrStatus = pNCC->GetInstanceGuid ( 
                                    &pGuids[ dwNumGuids - 1 ] 
                                    ) ;
                            }
                            else
                            {
                                --dwNumGuids ;
                                hrStatus = ERROR_OUTOFMEMORY ;
                            }
                        }
                    }
                    CoTaskMemFree(ppszwDisplayName);
                    pNCC->Release( ) ;
                }

                pEnumNCC->Release( ) ;
            }
        }

        pNC->Release( ) ;
    }
    else
    {
        wprintf(L"CoCreateInstance Failure[%X] in %s\n", 
               hrStatus,
               __FUNCTION__);
    }

    if ( ( hrStatus == S_OK ) && ( dwNumGuids > 0 ) )
    {
        *ppGuids = pGuids ;
        *pdwNumGuids = dwNumGuids ;
    }
    else
    {
        *ppGuids = NULL ;
        *pdwNumGuids = 0 ;
    }

    return hrStatus ;
}

BSTR
GetBstrXmlFromFile (
    PWSTR wszFileName
    )
{
    BSTR bstrXml = NULL ;
    HANDLE hFile ;

    hFile = CreateFileW (
        wszFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        ) ;

    if ( hFile != INVALID_HANDLE_VALUE )
    {
        LARGE_INTEGER nFileSize = { 0 } ;

        if ( GetFileSizeEx( hFile, &nFileSize ) )
        {
            PVOID pFileBuffer ;

            pFileBuffer = HeapAlloc ( 
                GetProcessHeap( ), HEAP_ZERO_MEMORY, 
                nFileSize.LowPart + 1
                ) ;

            if ( pFileBuffer != NULL )
            {
                BOOL fResult ;
                DWORD dwBytesRead = 0 ;

                fResult = ReadFile (
                    hFile,
                    pFileBuffer,
                    nFileSize.LowPart,
                    &dwBytesRead,
                    NULL
                    ) ;

                if ( fResult != FALSE )
                {
                    INT mask = IS_TEXT_UNICODE_UNICODE_MASK ;
                    PWSTR wszText ;

                    if ( IsTextUnicode( pFileBuffer, nFileSize.LowPart, &mask ) )
                    {
                        wszText = ( PWSTR ) HeapAlloc (
                            GetProcessHeap( ), HEAP_ZERO_MEMORY,
                            nFileSize.LowPart + 1
                            ) ;

                        if ( wszText != NULL )
                        {
                            CopyMemory( wszText, pFileBuffer, nFileSize.LowPart ) ;
                            bstrXml = SysAllocString( wszText ) ;
                            HeapFree( GetProcessHeap( ), 0, wszText ) ;
                        }
                    }
                    else
                    {
                        wszText = ( PWSTR ) HeapAlloc ( 
                            GetProcessHeap( ), HEAP_ZERO_MEMORY, 
                            ( ( nFileSize.LowPart + 1 ) ) * sizeof( WCHAR )
                            ) ;

                        if ( wszText != NULL )
                        {
                            int nBytes = MultiByteToWideChar (
                                                CP_THREAD_ACP,
                                                MB_COMPOSITE,
                                                ( PCSTR ) pFileBuffer,
                                                -1,
                                                wszText,
                                                nFileSize.LowPart + 1
                                                );
                            if ( nBytes )
                            {
                                bstrXml = SysAllocString( wszText ) ;
                            }
                            else
                            {
                                wprintf(L"Wide character convertion failed %d\n", 
                                    GetLastError());
                            }
                            HeapFree( GetProcessHeap( ), 0, wszText ) ;
                        }
                    }
                }
                else
                {
                    wprintf(L"Failed reading file[%X] in %s\n", 
                           fResult,
                           __FUNCTION__);
                }

                HeapFree( GetProcessHeap( ), 0, pFileBuffer ) ;
            }
            else
            {
                wprintf(L"Memory allocation error in %s\n", 
                        __FUNCTION__);
            }
        }
        else
        {
            wprintf(L"GetFileSizeEx failure in %s\n", 
                    __FUNCTION__);
        }

        CloseHandle( hFile ) ;
    }
    else
    {
        wprintf(L"Open file failure in %s\n", 
                __FUNCTION__);
    }

    return bstrXml ;
}

BSTR
ComposeBstrXml (
    PWSTR wszSSId,
    PWSTR wszConnectionType,
    PWSTR wszAuthentication,
    PWSTR wszEncryption,
    PWSTR wzsNetworkKey,
    PWSTR wszKeyProvidedAutomatically,
    PWSTR wszEable8021X
    )
{
    WCHAR wszBuffer[1024];

    BSTR bstrFormat = L"<?xml version=\"1.0\"?><wp:WirelessProfile\
                 xmlns=\"http://www.microsoft.com/provisioning/WirelessProfile\"\
                 xmlns:wp=\"http://www.microsoft.com/provisioning/WirelessProfile\">\
                 <wp:version>1</wp:version>\
                 <wp:ssid>%ws</wp:ssid>\
                 <wp:connectionType>%ws</wp:connectionType>\
                 <wp:authentication>%ws</wp:authentication>\
                 <wp:encryption>%ws</wp:encryption>\
                 <wp:networkKey>%ws</wp:networkKey>\
                 <wp:keyProvidedAutomatically>%ws</wp:keyProvidedAutomatically>\
                 <wp:IEEE802.1XEnabled>%ws</wp:IEEE802.1XEnabled>\
                 </wp:WirelessProfile>";

    swprintf ( wszBuffer, bstrFormat, wszSSId, wszConnectionType,
               wszAuthentication, wszEncryption, wzsNetworkKey,
               wszKeyProvidedAutomatically, wszEable8021X );

    BSTR bstrText = SysAllocString( wszBuffer ) ;

    return bstrText;
}

INT
LoadWPSProfile (
    IN BSTR bstrXml,
    IN PWSTR wszAdapterName
    )
{
    HRESULT hrStatus = S_OK ;
    LPGUID pGuids = NULL ;
    DWORD dwNumGuids = 0 ;

    //
    // Sanity checks
    //

    if ( bstrXml == NULL )
    {
        hrStatus = E_POINTER;
        return hrStatus;
    }

    CoInitialize( NULL ) ;

    //
    // Cannot use anonymous DCOM/RPC in SP2, so we have to
    // initialize security context.
    //

    CoInitializeSecurity (
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_NONE,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
        ) ;

    //
    // Find the device GUID for the wireless device. This
    // code assumes that there is only a single wireless
    // NIC. We could easily expand this by getting a GUID
    // array and the number of GUIDs in the array.
    //

    hrStatus = FindNICGuids( &pGuids, &dwNumGuids, wszAdapterName ) ;

    if ( hrStatus == S_OK )
    {
        IProvisioningProfileWireless *pPPW = NULL ;

        hrStatus = CoCreateInstance (
            CLSID_NetProvisioning,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IProvisioningProfileWireless,
            ( PVOID * ) &pPPW
            ) ;

        if ( hrStatus == S_OK )
        {
            //
            // Call the CreateProfile interface.
            //

            for ( DWORD dwIndex = 0 ; dwIndex < dwNumGuids ; dwIndex++ )
            {
                ULONG wpsResult = 0 ;

                hrStatus = pPPW->CreateProfile (
                    ( BSTR ) bstrXml,
                    ( BSTR ) NULL,
                    ( LPGUID ) &pGuids[ dwIndex ],
                    ( PULONG ) &wpsResult
                    ) ;

                if ( SUCCEEDED( hrStatus ) )
                {
                    wprintf(L"Successfully loaded profile\n");
                }
                else
                {
                    wprintf(L"Failed to loaded profile [%X, %d] in %s\n",
                            hrStatus,
                            wpsResult,
                            __FUNCTION__);
                }
            }

            pPPW->Release( ) ;
        }
        else
        {
            wprintf(L"Log error, failed to instanciate WPS.\n");
        }

        HeapFree( GetProcessHeap( ), 0, pGuids ) ;
    }
    else
    {
        wprintf(L"Log warning, no device GUID was located.\n");
    }

    CoUninitialize( ) ;

    return hrStatus ;
}

