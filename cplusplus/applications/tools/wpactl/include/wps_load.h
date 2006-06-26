#ifndef __WPS_LOADER__
#define __WPS_LOADER__

#include "stdafx.h"
#include <windows.h>
#include <objbase.h>
#include <comutil.h>

/*++

Routine Description:

    This routine will take a profile file name and load it into the
    WPS service to configure a local wireless NIC.

Parameters:

    wszFileName
        [in] The name of the profile file.

    wszName
        [in] Adapter display name.

Return Value:

    The error code representing the exit status of the application. Error
    codes can be found in winerror.h.

--*/
INT
LoadWPSProfile (
    IN BSTR bstrXml,
    IN PWSTR wszAdapterName
    );

/*++

Routine Description:

    This function retrieves the XML file contents in BSTR format given the
    name of the XML file.

Parameters:

    wszFileName
        [in] The name of the XML file.

Return Values:

    If the function succeeds, then the return value is a BSTR to the XML
    text. If the function fails, then the return value is NULL.

--*/
BSTR
GetBstrXmlFromFile (
    PWSTR wszFileName
    );

/*++

Routine Description:

    This function generates an BSTR XML text based on the parameters
    passed to it.

Parameters:

    wszFileName
        [in] The name of the XML file.

Return Values:

    If the function succeeds, then the return value is a BSTR to the XML
    text. If the function fails, then the return value is NULL.

--*/
BSTR
ComposeBstrXml (
    PWSTR wszSSId,
    PWSTR wszConnectionType,
    PWSTR wszAuthentication,
    PWSTR wszEncryption,
    PWSTR wzsNetworkKey,
    PWSTR wszKeyProvidedAutomatically,
    PWSTR wszEable8021X
    );

#endif // __WPS_LOADER__