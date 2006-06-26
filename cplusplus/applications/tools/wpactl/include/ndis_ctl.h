#ifndef __WLAN_CTL_H__
#define __WLAN_CTL_H__

#include "stdafx.h"
#include <windows.h>

#define MAX_NDIS_DEVICE_NAME_LEN        256
#define NUM_DEVICES						12
#define DEVICE_LENGTH					47

INT 
LoadOpenAuth ( 
    BOOL bEnable,
    PWSTR pwszAdapterName, 
    PWSTR wszSSId
    );

#endif // __WLAN_CTL_H__