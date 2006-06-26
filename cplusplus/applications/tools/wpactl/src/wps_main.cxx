#include "wpa_main.h"
#include "wps_load.h"
#include "ndis_ctl.h"

INT
ProcessArguments (
    IN INT nArgc,
    IN PWSTR *pwszArgv,
    OUT BOOL *pbEnable,
    OUT PWSTR *pwszAdapterName,
    OUT PWSTR *pwszSSId,
    OUT PWSTR *pwszWPAKey
    )
/*++

Routine Description:

    This routine processes the command line arguments.

Parameters:

    nArgc
        [in] The number of arguments in the argument vector.

    pwszArgv
        [in] The Unicode argument vector.
 
    pbEnable
        [out] Receives the enable or disable flag.

    pwszAdapterName
        [out] Receives the name of the adapter.

    pwszSSId
        [out] Receives the SSID of the AP.

    pwszWPAKey
        [out] WPA key to use as PMK.

Return Value:

    The error code representing the exit status of the application. Error
    codes can be found in winerror.h.

--*/
{
    INT nResult = ERROR_SUCCESS ;

    if ( nArgc < 2 || nArgc > 4 ) 
    {
        return ERROR_INVALID_PARAMETER;
    }

    if ( ! wcsicmp(pwszArgv[0], CMD_ENABLE) )
    {
        *pbEnable = TRUE;
    }
    else if ( ! wcsicmp(pwszArgv[0], CMD_DISABLE) )
    {
        *pbEnable = FALSE;
    }
    else
    {
        return ( ERROR_INVALID_PARAMETER );
    }
    nArgc --;

    try
    {
        *pwszAdapterName = ( PWSTR ) HeapAlloc (
                            GetProcessHeap( ), HEAP_ZERO_MEMORY,
                            ( lstrlenW( pwszArgv[1] ) + 1 ) * sizeof( WCHAR )
                            ) ;
        if ( *pwszAdapterName != NULL )
        {
            lstrcpyW( *pwszAdapterName, pwszArgv[1] ) ;
        }
        else
        {
            throw ( ERROR_NOT_ENOUGH_MEMORY );
        }

        if ( ! *pbEnable )
        {
            throw ( ERROR_SUCCESS );
        }
        else if ( -- nArgc == 0 )
        {
            throw ( ERROR_INVALID_PARAMETER );
        }

        *pwszSSId = ( PWSTR ) HeapAlloc (
                            GetProcessHeap( ), HEAP_ZERO_MEMORY,
                            ( lstrlenW( pwszArgv[2] ) + 1 ) * sizeof( WCHAR )
                            ) ;
        if ( *pwszSSId != NULL )
        {
            lstrcpyW( *pwszSSId, pwszArgv[2] ) ;
        }
        else
        {
            HeapFree( GetProcessHeap( ), 0, *pwszAdapterName ) ;
            throw ( ERROR_NOT_ENOUGH_MEMORY );
        }

        if ( -- nArgc == 0 )
        {
            throw ( ERROR_SUCCESS );
        }

        *pwszWPAKey = ( PWSTR ) HeapAlloc (
                            GetProcessHeap( ), HEAP_ZERO_MEMORY,
                            ( lstrlenW( pwszArgv[3] ) + 1 ) * sizeof( WCHAR )
                            ) ;
        if ( *pwszWPAKey != NULL )
        {
            int nCopyLen = lstrlenW( pwszArgv[3] ) < 64 ?
                           lstrlenW( pwszArgv[3] ) : 64;
            printf( "Key Length: %d\n", lstrlenW( pwszArgv[3] ) );
            lstrcpynW( *pwszWPAKey, pwszArgv[3], lstrlenW( pwszArgv[3] ) ) ; // 256-bit PMK
        }
        else
        {
            HeapFree( GetProcessHeap( ), 0, *pwszAdapterName ) ;
            HeapFree( GetProcessHeap( ), 0, *pwszSSId ) ;
            throw ( ERROR_NOT_ENOUGH_MEMORY );
        }
    }
    catch ( LONG nRes )
    {
        nResult = nRes;
    }

    return nResult;
}

extern "C"
int
__cdecl
wmain (
    int nArgc,
    wchar_t **pwszArgv
    )
/*++

Routine Description:

    This is the main entry-point routine for the wpstool application.

Arguments:

    nArgc
        [in] The number of arguments in the argument vector.

    pwszArgv
        [in] The Unicode argument vector.
 
Return Value:

    The error code representing the exit status of the application. Error
    codes can be found in winerror.h.

--*/
{
    INT nResult = ERROR_SUCCESS ;
    BOOL bEnable = TRUE;
    PWSTR pwszAdapterName = NULL ;
    PWSTR pwszSSId = NULL ;
    PWSTR pwszWPAKey = NULL ;

    nResult = ProcessArguments ( 
                 --nArgc, ++pwszArgv, 
                 &bEnable, &pwszAdapterName,
                 &pwszSSId, &pwszWPAKey ) ;

    if ( nResult == ERROR_SUCCESS )
    {
        if ( bEnable )
        {
            if ( pwszWPAKey ) 
            {
                WZC_ENABLE();

                BSTR bstrXml = ComposeBstrXml (
                    pwszSSId, L"ESS", L"WPAPSK", 
                    L"TKIP", pwszWPAKey, L"false",
                    L"false" ) ;

                nResult = LoadWPSProfile (
                            bstrXml,
                            pwszAdapterName
                            );

                HeapFree( GetProcessHeap( ), 0, pwszSSId ) ;
                HeapFree( GetProcessHeap( ), 0, pwszWPAKey ) ;
            }
            else
            {
                WZC_DISABLE();

                nResult = LoadOpenAuth ( 
                            TRUE,
                            pwszAdapterName,
                            pwszSSId
                            );
            }
        }
        else
        {
            WZC_DISABLE();

            nResult = LoadOpenAuth ( 
                        FALSE,
                        pwszAdapterName,
                        NULL
                        );
        }

        HeapFree( GetProcessHeap( ), 0, pwszAdapterName ) ;
    }
    else
    {
        printf(USAGE);
    }

    return nResult ;
}
