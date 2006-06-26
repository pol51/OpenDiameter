REM **** Available options for PaC auth script
IF %1 == open GOTO OPEN_MODE
IF %1 == del  GOTO DELETE_MODE
IF %1 == add  GOTO ADD_MODE
GOTO DONE

REM ------------------- OPEN mode
:OPEN_MODE
REM **** In Open mode, we setup only one odyssey profile for AP1
REM **** and associate with the AP in open mode for pre-PANA auth.
REM **** This mode is called either manually (before PANA) or after
REM **** PaC disconnection (after DEL mode has completed)

REM **** Call the odyssey controller with no keys specified to
REM **** add a profile in open mode. Currently it AP1
OdysseyCtl add wa7000-net

REM **** Call the odyssey controller to associate with the
REM **** newly added profile
OdysseyCtl connect "Intel(R) PRO/Wireless" wa7000-net
GOTO DONE



REM ------------------- ADD mode
:ADD_MODE
REM **** In ADD mode, the PaC has invoked this script to add
REM **** two(2) Odyssey profiles with WPA-PSK keys. The AP
REM **** names are hardcoded as well as the interface name.
REM **** note that we need to add the profiles first then we
REM **** can select which AP to associate first. For roaming,
REM **** we can manually use the Odyssey client manager to
REM **** select between AP's

REM **** Re-associate using authentication keys to AP1
REM **** Let the interface renew dhcp
SET KEY1=%8
SET PMK1=%KEY1:~0,64%
OdysseyCtl add wa7000-net %PMK1%

REM **** Re-associate using authentication keys to AP1
REM **** Let the interface renew dhcp
SET KEY2=%9
SET PMK2=%KEY2:~0,64%
OdysseyCtl add wa7000-net2 %PMK2%

REM **** Now associate with a selected AP (AP1 for now)
OdysseyCtl connect "Intel(R) PRO/Wireless" wa7000-net
GOTO DONE



REM ------------------- DEL mode
:DELETE_MODE
REM **** In this mode, the PaC has initiated a disconnection
REM **** process. We simply disassociate the profiles previously
REM **** added in OPEN_MODE then remove the profiles from the
REM Odyssey client manager

REM **** Disassociate with both AP's first
OdysseyCtl disconn wa7000-net
OdysseyCtl disconn wa7000-net2

REM **** Remove the AP's profiles from the client manager
OdysseyCtl remove wa7000-net
OdysseyCtl remove wa7000-net2

REM **** Note here that after deleting both profiles
REM **** We add an open mode profile so we can come
REM **** back to a pre-PANA mode
GOTO OPEN_MODE


REM -------------------- EXIT
:DONE
REM **** Simply exit

