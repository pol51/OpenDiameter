
#ifndef __WPA_MAIN_H__
#define __WPA_MAIN_H__

#include "stdafx.h"

#define USAGE "wpactl command [Device Name] [..options]\n\
commands:\n\
    enable - Enables a WLAN device and attempts to associated it with\n\
             a given SSID. If a key is supplied then WPA-PSK will be\n\
             used else Open authentication will be used.\n\
\n\
             wlan_ctl enable [Device Name] [AP SSID] [WPA key]\n\
\n\
             The [Device Name] is the friendly name of the adapter.\n\
             It must be enclosed in quotes if there are spaces in the \n\
             name. This parameter is required. The [AP SSID] is the SSID \n\
             of the AP which the device wants to associated with. The [WPA key]\n\
             is a string of hex characters representing a 63 byte WPA-PSK key.\n\
             If this options is present then WAP authentication will be attempted\n\
             else Open authentication will be used. Note that this program\n\
             currently uses WZC profiles in installing WPA keys. As of this\n\
             writing, not method of removing profiles is avaialable through\n\
             WZC so manual removal will be necessary\n\
\n\
             example:\n\
                wpactl enable \"D-Link 650\" AP 0123456789abcdef....\n\
\n\
                OR\n\
\n\
                wpactl enable \"D-Lin 650\" AP\n\
\n\
    disable - Disables a WLAN device and dis-associate it from an AP\n\
              if it's currently associated. Note that disable will only\n\
              work when there is an existing authentication\n\
\n\
             wpactl disable [Device Name]\n\
\n\
             The [Device Name] is the friendly name of the adapter.\n\
             It must be enclosed in quotes if there are spaces in the \n\
             name. This parameter is required."

#define CMD_ENABLE                      L"enable"
#define CMD_DISABLE                     L"disable"
#define CMD_HELP                        L"help"

#define WZC_DISABLE()                   system("net stop wzcsvc")
#define WZC_ENABLE()                    system("net start wzcsvc")

#endif // __WPA_MAIN_H__