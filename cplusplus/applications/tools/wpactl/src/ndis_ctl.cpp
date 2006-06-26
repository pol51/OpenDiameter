// wpa.cpp : Defines the entry point for the console application.
//

#include "ndis_ctl.h"
#include <ntddndis.h>
#include <nuiouser.h>

typedef unsigned char	                u8;
typedef u8				                MAC_ADDR[6];

typedef struct WPA_NDIS_DEVICE
{
	WCHAR *pDeviceName;
	WCHAR *pDeviceDescription;
} WPA_NDIS_DEVICE;

void LastErrorMessage() 
{
    LPVOID lpMsgBuf;
    if (!FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL ))
    {
       return;
    }
    printf("Last Error: %s\n", (LPTSTR)lpMsgBuf);
    LocalFree( lpMsgBuf );
}

wchar_t *AsciiToWide(CHAR *str)
{
   // Short circuit null pointer case
   if (str == 0)
   {
       return 0;
   }

   UINT cp = GetACP ();
   int len = ::MultiByteToWideChar (cp, 0, str, -1, 0, 0);

   wchar_t *wstr = new wchar_t[len];

   ::MultiByteToWideChar (cp, 0, str, -1, wstr, len);
   return wstr;
}

UCHAR *AsciiToHex(CHAR *str, ULONG &len)
{
    UCHAR *hexValue = new UCHAR[len/2];

    for (ULONG y = 0; y < (len/2); ++y) 
    {
        for (ULONG i = y * 2; 
             i <= ((y * 2) + 1); 
             i ++) 
        {
            UCHAR value = 0;
            if (isxdigit(str[i]))
            {
                if (isdigit(str[i]))
                {
                    value = str[i] - '0';
                }
                else if (islower(str[i]))
                {
                    value = str[i] - 'a';
                    value += 10;
                }
                else
                {
                    value = str[i] - 'A';
                    value += 10;
                }
                if (i == (y * 2)) 
                {
                    hexValue[y] = value << 4;
                }
                else 
                {
                    hexValue[y] |= value;
                }
            }
            else
            {
                delete hexValue;
                return (0);
            }
        }
    }
    len = len/2;
    return hexValue;
}

CHAR * WideToAscii(PWSTR wstr)
{
  // Short circuit null pointer case
  if ( wstr == 0 )
  {
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

HRESULT EnumerateDevices(HANDLE handle,
                         WPA_NDIS_DEVICE **ppNdisDeviceList, 
                         long *plItems)
{
	CHAR	                Buf[1024];
	DWORD	                dwBytesWritten, i = 0;
	HRESULT	                hRes = S_OK;
	PNDISUIO_QUERY_BINDING  pQueryBinding;
	DWORD	                dwError;

	pQueryBinding = (PNDISUIO_QUERY_BINDING)Buf;
	
	*ppNdisDeviceList = (WPA_NDIS_DEVICE *) malloc(NUM_DEVICES * sizeof(WPA_NDIS_DEVICE));

	for (pQueryBinding->BindingIndex = i;
		 /* NOTHING */;
		 pQueryBinding->BindingIndex = ++i)
	{
        if (DeviceIoControl(handle,
							IOCTL_NDISUIO_QUERY_BINDING,
							pQueryBinding,
							sizeof(NDISUIO_QUERY_BINDING),
							Buf,
							sizeof(Buf),
							&dwBytesWritten,
							NULL))
		{
			/* Get the device name in the list of bindings */

			(*ppNdisDeviceList)[i].pDeviceName = (WCHAR *) malloc(pQueryBinding->DeviceNameLength);
			(*ppNdisDeviceList)[i].pDeviceDescription = (WCHAR *) malloc(pQueryBinding->DeviceDescrLength);
			
			memcpy((*ppNdisDeviceList)[i].pDeviceName, (PUCHAR)pQueryBinding+pQueryBinding->DeviceNameOffset,
					pQueryBinding->DeviceNameLength);

			memcpy((*ppNdisDeviceList)[i].pDeviceDescription, (PUCHAR)pQueryBinding+pQueryBinding->DeviceDescrOffset,
				pQueryBinding->DeviceDescrLength);
			
			memset(Buf, 0, sizeof(Buf));
		}

		else
		{
			dwError = GetLastError();
			if (dwError != ERROR_NO_MORE_ITEMS)
			{
				printf("EnumerateDevices: terminated abnormally, error %d\n", dwError);
				hRes = E_FAIL;
			}
			break;
		}

		*plItems = i + 1;
	}

	return hRes;
}

HRESULT OpenNdisDevice(HANDLE handle, WCHAR *pwDeviceName)
{
	WCHAR   wNdisDeviceName[DEVICE_LENGTH] = {0};
	HRESULT	hRes = S_OK;
	int     wNameLength, i = 0;
	DWORD	dwBytesReturned;

	if (!pwDeviceName)
	{
		hRes = E_POINTER;
	}
	else
	{
		wNameLength = 0;

		for ( i = 0; i < DEVICE_LENGTH-1; i++ )
		{
			wNdisDeviceName[i] = pwDeviceName[i];
			wNameLength++;
		}

		wNdisDeviceName[i] = L'\0';

		if (!DeviceIoControl(handle,
							IOCTL_NDISUIO_OPEN_DEVICE,
							(LPVOID) &wNdisDeviceName[0],
							sizeof(WCHAR)*wNameLength,
							NULL,
							0,
							&dwBytesReturned,
							NULL))
		{
            LastErrorMessage();
			hRes = E_FAIL;
		}
	}

	return hRes;

}

HRESULT GetSSId(HANDLE handle, UCHAR *pSSId)
{
	UCHAR					QueryBuffer[1024];
	PNDISUIO_QUERY_OID		pQueryOid;
	ULONG					lSSIdLength;
	HRESULT	                hRes = S_OK;
	DWORD	                dwBytesReturned;

	if (!pSSId)
	{
		hRes = E_POINTER;
	}

	else
	{
		pQueryOid = (PNDISUIO_QUERY_OID) &QueryBuffer[0];
		pQueryOid->Oid = OID_802_11_SSID;

		if (DeviceIoControl(handle,
							IOCTL_NDISUIO_QUERY_OID_VALUE,
							(LPVOID) &QueryBuffer[0],
							sizeof(QueryBuffer),
							(LPVOID) &QueryBuffer[0],
							sizeof(QueryBuffer),
							&dwBytesReturned,
							NULL))
		{
			printf("IOCTL SSID succeeded\n");
			lSSIdLength = ((PNDIS_802_11_SSID)(pQueryOid->Data))->SsidLength;
			memcpy(pSSId, ((PNDIS_802_11_SSID)(pQueryOid->Data))->Ssid, lSSIdLength);
		}

		else
		{
            LastErrorMessage();
		}

	}
	
	return hRes;

}

HRESULT GetAssociatedAP(HANDLE handle, MAC_ADDR mac_addr)
{
	UCHAR				QueryBuffer[1024];
	PNDISUIO_QUERY_OID	pQueryOid;
	int					i = 0;
	DWORD	            dwBytesReturned;
	HRESULT             hRes = S_OK;

	pQueryOid = (PNDISUIO_QUERY_OID) &QueryBuffer[0];
	pQueryOid->Oid = OID_802_11_BSSID;

	if (DeviceIoControl(handle,
						IOCTL_NDISUIO_QUERY_OID_VALUE,
						(LPVOID) &QueryBuffer[0],
						sizeof(QueryBuffer),
						(LPVOID) &QueryBuffer[0],
						sizeof(QueryBuffer),
						&dwBytesReturned,
						NULL))
	{
		printf("IOCTL GET_BSSID succeeded\n");
		
		for ( i = 0; i < 6; i++ )
		{
			mac_addr[i] = pQueryOid->Data[i];
		}
	}

	else
	{
        LastErrorMessage();
		hRes = E_FAIL;
	}

	return hRes;
}

HRESULT SetSSId(HANDLE handle, UCHAR *pSSId, ULONG& lSSIdLength)
{
	UCHAR				SetBuffer[sizeof(NDIS_OID) + sizeof(NDIS_802_11_SSID)];
	PNDISUIO_SET_OID	pSetOid;
	NDIS_802_11_SSID	SSID;
	DWORD	            dwBytesReturned;
	HRESULT             hRes = S_OK;

	if (!pSSId)
	{
		hRes = E_POINTER;
	}

	else
	{
		pSetOid = (PNDISUIO_SET_OID) &SetBuffer[0];
		pSetOid->Oid = OID_802_11_SSID;
		
		SSID.SsidLength = lSSIdLength;
		memcpy(&SSID.Ssid[0], pSSId, lSSIdLength);
		
		memcpy(&pSetOid->Data[0], &SSID, sizeof(NDIS_802_11_SSID));

		if (!DeviceIoControl(handle,
							IOCTL_NDISUIO_SET_OID_VALUE,
							(LPVOID) &SetBuffer[0],
							sizeof(SetBuffer),
							(LPVOID) &SetBuffer[0],
							0,
							&dwBytesReturned,
							NULL))
		{
            LastErrorMessage();
			hRes = E_FAIL;
		}

	}

	return hRes;
}

HRESULT Disassociate(HANDLE handle)
{
	UCHAR					QueryBuffer[1024];
	PNDISUIO_SET_OID		pSetOid;
	HRESULT                 hRes = S_OK;
	DWORD	                dwBytesReturned = 0;

	pSetOid = (PNDISUIO_SET_OID) &QueryBuffer[0];
	pSetOid->Oid = OID_802_11_DISASSOCIATE;

	if (!DeviceIoControl(handle,
						IOCTL_NDISUIO_SET_OID_VALUE,
						(LPVOID) &QueryBuffer[0],
						sizeof(QueryBuffer),
						(LPVOID) &QueryBuffer[0],
						0,
						&dwBytesReturned,
						NULL))
	{
        LastErrorMessage();
		hRes = E_FAIL;
	}

	return hRes;
}

HRESULT SetNetworkMode(HANDLE handle, int *pnNetworkMode)
{
	UCHAR				SetBuffer[sizeof(NDIS_OID) + sizeof(NDIS_802_11_NETWORK_INFRASTRUCTURE)];
	PNDISUIO_SET_OID	pSetOid;
	HRESULT             hRes = S_OK;
	DWORD	            dwBytesReturned = 0;

	if (!pnNetworkMode)
	{
		hRes = E_POINTER;
	}

	else
	{
        pSetOid = (PNDISUIO_SET_OID) &SetBuffer[0];
		pSetOid->Oid = OID_802_11_INFRASTRUCTURE_MODE;

		memcpy(&pSetOid->Data[0], pnNetworkMode, sizeof(NDIS_802_11_NETWORK_INFRASTRUCTURE));

		if (!DeviceIoControl(handle,
							 IOCTL_NDISUIO_SET_OID_VALUE,
							 (LPVOID) &SetBuffer[0],
							 sizeof(SetBuffer),
							 (LPVOID) &SetBuffer[0],
							 0,
							 &dwBytesReturned,
							 NULL))
		{
            LastErrorMessage();
			hRes = E_FAIL;
		}
	}
	
	return hRes;
}

HRESULT SetWPAKey(HANDLE handle, 
                  UCHAR *pWPAKey, 
                  ULONG lWPAKeyLength,
                  MAC_ADDR *bssid_addr)
{
	UCHAR				SetBuffer[sizeof(NDIS_OID) + sizeof(NDIS_802_11_KEY) + 128];
	PNDISUIO_SET_OID	pSetOid;
	NDIS_802_11_KEY     WPAKey, *pWPAKeyMaterial;
	DWORD	            dwBytesReturned;
	HRESULT             hRes = S_OK;

	if (!pWPAKey)
	{
		hRes = E_POINTER;
	}
	else
	{
        for (int i = 0; i < 2; i ++) {

            memset(&SetBuffer[0], 0x0, sizeof(SetBuffer));
            memset(&WPAKey, 0x0, sizeof(WPAKey));

		    pSetOid = (PNDISUIO_SET_OID) &SetBuffer[0];
		    pSetOid->Oid = OID_802_11_ADD_KEY;
    		
		    WPAKey.Length = sizeof(NDIS_802_11_KEY) + lWPAKeyLength;

            WPAKey.KeyIndex = 0; // clear
            WPAKey.KeyIndex = 0x1 << 31; // transmission bit
            WPAKey.KeyIndex |= i << 30; // pairwise or group key
            WPAKey.KeyIndex |= (i == 0) ? 1 : 0; // index is 1
            WPAKey.KeyLength = lWPAKeyLength;

            if (bssid_addr) 
            {
                memcpy(&WPAKey.BSSID[0], &(*bssid_addr)[0], sizeof(MAC_ADDR));
            }
            else if (i == 0)
            {
                memset(&WPAKey.BSSID[0], 0xff, sizeof(NDIS_802_11_MAC_ADDRESS));
            }
            else
            {
                // TESTING
                UCHAR test[6] = { 0x00, 0x0d, 0x88, 0x51, 0x8f, 0x36 };
                memcpy(&WPAKey.BSSID[0], &test[0], sizeof(test));
            }

		    memcpy(&pSetOid->Data[0], &WPAKey, sizeof(NDIS_802_11_KEY));
            pWPAKeyMaterial = (NDIS_802_11_KEY*)pSetOid->Data;
            memcpy(pWPAKeyMaterial->KeyMaterial,
                   pWPAKey, 
                   lWPAKeyLength);

		    if (!DeviceIoControl(handle,
							    IOCTL_NDISUIO_SET_OID_VALUE,
							    (LPVOID) &SetBuffer[0],
							    sizeof(SetBuffer),
							    (LPVOID) &SetBuffer[0],
							    0,
							    &dwBytesReturned,
							    NULL))
		    {
                LastErrorMessage();
			    hRes = E_FAIL;
                break;
		    }
	    }
    }

	return hRes;
}

HRESULT RemoveWPAKey(HANDLE handle, MAC_ADDR *bssid_addr)
{
	UCHAR				    SetBuffer[sizeof(NDIS_OID) + sizeof(NDIS_802_11_REMOVE_KEY)];
	PNDISUIO_SET_OID	    pSetOid;
	NDIS_802_11_REMOVE_KEY	WPAKey;
	DWORD	                dwBytesReturned;
	HRESULT                 hRes = S_OK;

    for (int i = 0; i < 2; i ++) {
	    pSetOid = (PNDISUIO_SET_OID) &SetBuffer[0];
	    pSetOid->Oid = OID_802_11_REMOVE_KEY;
    		
        WPAKey.Length = sizeof(NDIS_802_11_REMOVE_KEY);

        WPAKey.KeyIndex = 0; // index is 0
        WPAKey.KeyIndex = 0x1 << 31; // transmission bit
        WPAKey.KeyIndex |= i << 30; // pairwise or group key

        if (bssid_addr) 
        {
            memcpy(&WPAKey.BSSID[0], &(*bssid_addr)[0], sizeof(MAC_ADDR));
        }
        else
        {
            memset(&WPAKey.BSSID[0], 0xff, sizeof(NDIS_802_11_MAC_ADDRESS));
        }

		memcpy(&pSetOid->Data[0], &WPAKey, sizeof(NDIS_802_11_KEY));

	    if (!DeviceIoControl(handle,
						    IOCTL_NDISUIO_SET_OID_VALUE,
						    (LPVOID) &SetBuffer[0],
						    sizeof(SetBuffer),
						    (LPVOID) &SetBuffer[0],
						    0,
						    &dwBytesReturned,
						    NULL))
	    {
            LastErrorMessage();
		    hRes = E_FAIL;
            break;
	    }
    }

	return hRes;
}

HRESULT SetAuthMode(HANDLE handle, int *pnAuthMode)
{
	UCHAR				SetBuffer[sizeof(NDIS_OID) + sizeof(NDIS_802_11_AUTHENTICATION_MODE)];
	PNDISUIO_SET_OID	pSetOid;
	HRESULT             hRes = S_OK;
	DWORD	            dwBytesReturned = 0;

	if (!pnAuthMode)
	{
		hRes = E_POINTER;
	}

	else
	{
        pSetOid = (PNDISUIO_SET_OID) &SetBuffer[0];
		pSetOid->Oid = OID_802_11_AUTHENTICATION_MODE;

		memcpy(&pSetOid->Data[0], pnAuthMode, sizeof(NDIS_802_11_AUTHENTICATION_MODE));

		if (!DeviceIoControl(handle,
							 IOCTL_NDISUIO_SET_OID_VALUE,
							 (LPVOID) &SetBuffer[0],
							 sizeof(SetBuffer),
							 (LPVOID) &SetBuffer[0],
							 0,
							 &dwBytesReturned,
							 NULL))
		{
            LastErrorMessage();
			hRes = E_FAIL;
		}
	}
	
	return hRes;
}

HRESULT SetEncryptionStatus(HANDLE handle, int *pnEncryptStatus)
{
	UCHAR				SetBuffer[sizeof(NDIS_OID) + sizeof(NDIS_802_11_ENCRYPTION_STATUS)];
	PNDISUIO_SET_OID	pSetOid;
	HRESULT             hRes = S_OK;
	DWORD	            dwBytesReturned = 0;

	if (!pnEncryptStatus)
	{
		hRes = E_POINTER;
	}
	else
	{
        pSetOid = (PNDISUIO_SET_OID) &SetBuffer[0];
		pSetOid->Oid = OID_802_11_ENCRYPTION_STATUS;

		memcpy(&pSetOid->Data[0], pnEncryptStatus, sizeof(NDIS_802_11_ENCRYPTION_STATUS));

		if (!DeviceIoControl(handle,
							 IOCTL_NDISUIO_SET_OID_VALUE,
							 (LPVOID) &SetBuffer[0],
							 sizeof(SetBuffer),
							 (LPVOID) &SetBuffer[0],
							 0,
							 &dwBytesReturned,
							 NULL))
		{
            LastErrorMessage();
			hRes = E_FAIL;
		}
	}
	
	return hRes;
}

HRESULT SetAssociatedAP(HANDLE handle, MAC_ADDR mac_addr)
{
	UCHAR				SetBuffer[sizeof(NDIS_OID) + sizeof(NDIS_802_11_MAC_ADDRESS)];
	PNDISUIO_SET_OID	pSetOid;
	HRESULT             hRes = S_OK;
	DWORD	            dwBytesReturned = 0;
	int                 i = 0;

	pSetOid = (PNDISUIO_SET_OID) &SetBuffer[0];
	pSetOid->Oid = OID_802_11_BSSID;

	for ( i = 0; i < 6; i++ )
	{
		pSetOid->Data[i] = mac_addr[i];
	}

	if (!DeviceIoControl(handle,
						 IOCTL_NDISUIO_SET_OID_VALUE,
						 (LPVOID) &SetBuffer[0],
						 sizeof(SetBuffer),
						 (LPVOID) &SetBuffer[0],
						 0,
						 &dwBytesReturned,
						 NULL))
	{
        LastErrorMessage();
		hRes = E_FAIL;
	}

	return hRes;
}

void WLanEnableDevice(HANDLE handle,
                      UCHAR *pSSId,
                      ULONG lSSIdLength,
                      UCHAR *pWPAKey,
                      ULONG lWPAKeyLength)
{
    int     nAuthMode;
    int     nNetworkMode = Ndis802_11Infrastructure;
    int     nStatus = Ndis802_11Encryption2Enabled;

    try 
    {
        if (SetNetworkMode(handle, &nNetworkMode) != S_OK)
        {
            throw (0);
        }
        printf("Set network mode to infastructure\n");

        nAuthMode = (pWPAKey) ? Ndis802_11AuthModeWPAPSK :
                                Ndis802_11AuthModeOpen;
        
        if (SetAuthMode(handle, &nAuthMode) != S_OK) 
        {
            throw (0);
        }
        printf("Set authentication mode\n");

        nStatus = (pWPAKey) ? Ndis802_11Encryption2Enabled :
                              Ndis802_11EncryptionDisabled;
        if (SetEncryptionStatus(handle, &nStatus) != S_OK)
        {
            throw (0);
        }
        printf("Set Encryption status\n");

        if (SetSSId(handle, pSSId, lSSIdLength) != S_OK)
        {
            throw (0);
        }
        printf("Set SSID\n");

        if (pWPAKey) 
        {
            if (SetWPAKey(handle, pWPAKey, lWPAKeyLength, 0) != S_OK) 
            {        
                throw (0);
            }
            printf("Set WPA key\n");
        }
    }
    catch (...)
    {
        printf("WLAN device enable failed\n");
    }
}

void WLanDisableDevice(HANDLE handle)
{
    char            szBuf[64];
    MAC_ADDR        MacAddr;

    if (GetAssociatedAP(handle, MacAddr) == S_OK) 
    {
        printf("Disassociating with: ");
        for (int i=0; i<sizeof(MacAddr); i++) 
        {
            sprintf(szBuf, "%02X ", (unsigned char)MacAddr[i]);
            printf(szBuf);
        }
        printf("\n");
    }
    Disassociate(handle);
}

INT LoadOpenAuth ( 
        BOOL bEnable,
        PWSTR pwszAdapterName, 
        PWSTR wszSSId
        )
{
	HANDLE	        hFileHandle;
    WPA_NDIS_DEVICE *pNdisDeviceList;
	DWORD	        dwBytesReturned = 0;
	CHAR *	        pNdisuioDevice = "\\\\.\\\\Ndisuio";
    HRESULT         hRes = S_OK;
    bool            bFound = false;    
    long            lItems;

	// Create a handle to the NDISUIO driver
	hFileHandle = CreateFile(pNdisuioDevice,
							 GENERIC_READ|GENERIC_WRITE,
							 0,
							 NULL,
							 OPEN_EXISTING,
							 FILE_ATTRIBUTE_NORMAL,
							 (HANDLE) INVALID_HANDLE_VALUE);

	//Bind the file handle to the driver
	if (!DeviceIoControl(hFileHandle,
						 IOCTL_NDISUIO_BIND_WAIT,
						 NULL,
						 0,
						 NULL,
						 0,
						 &dwBytesReturned,
						 NULL))
	{
        LastErrorMessage();
	}

    // Get the device list
    hRes = EnumerateDevices(hFileHandle, 
                            &pNdisDeviceList,
                            &lItems);

	// Lookup device list
	for (int i = 0; i < lItems; i++)
	{
        if (! wcsncmp(pNdisDeviceList[i].pDeviceDescription,
                      pwszAdapterName, wcslen(pwszAdapterName))) {
            bFound = true;
            break;
        }
	}

    if (bFound) {
        if (OpenNdisDevice(hFileHandle, pNdisDeviceList[i].pDeviceName) == S_OK) {

            printf("Device: %ws\n", pNdisDeviceList[i].pDeviceDescription);

            if (bEnable) 
            {
                PSTR pstrSSId       = WideToAscii(wszSSId);

                WLanEnableDevice(hFileHandle, 
                                 (UCHAR*)pstrSSId,
                                 (ULONG)strlen(pstrSSId),
                                 NULL,
                                 NULL);

                delete pstrSSId;
            }
            else {
                WLanDisableDevice(hFileHandle);
            }
        }
        else {
            printf("Unable to open device\n");
        }
    }
    else {
        printf("Device not found\n");
    }

	free(pNdisDeviceList);
    CloseHandle(hFileHandle);

    return hRes;
}

