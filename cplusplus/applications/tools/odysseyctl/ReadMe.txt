Document : OdysseyCtl Usage/Overview
Date     : Feb 6, 2005

OdysseyClt is a program that uses windows event messages/notifications
to control the Odyssey Client Manager GUI so it can automatically install,
remove, associate and disassociate a mobile STA to an AP. Since OdysseyClt 
is controlling a GUI program, it requires a very specific version of the 
GUI. As of this writing, the version of the OdysseyCtl works with the
following:

Odyssey Client Ver 3.10.0 1584 BETA
	- Beta version includes WPA support
	- Tested version is 30-day trial(demo) license
	- Website: http://www.funk.com/radius/wlan/wlan_c_radius.asp
	
OdysseyCtl Ver 1.0.0
	- Current contoller version that works with 
	  Odyssey Client Manager ver 3.10.0 BETA
	
The OdysseyCtl program is a command line program designed to be called 
within the PANA authentication script. It is designed to replace the
"wpactl.exe" controller program that works with Windows Wireless Zero
Config. The program has the following four(4) modes:

1. Add mode

     Add an network connection (profile) to the Odyssey Client Manager
     GUI. This allows addition of a network connection either in Open 
     mode or WPA-PSK mode depending on the options specified. In Open 
     mode the following command line format is required:

                 OdysseyCtl add [AP SSID]

     In WPA-PSK mode, the following command line format is required:

                 OdysseyCtl add [AP SSID] [Key in 64 char hex string]

     Note that adding a network connection will not initiate a connection 
     to that network. An explicit call using "OdysseyCtl connect ..." is 
     required. If an existing profile matches the one being added, the 
     existing profile is remove first. If the existing profile is currently
     connection or in open state, it will first be disconnected the removed.
     
 2. Connect  mode
 
     Attempt to connect an existing network connection (profile) in the 
     Odyssey Client Manager GUI. The profile must exists and maybe added 
     using "OdysseyCtl add ..." command. The following command line format 
     is required: 

                 OdysseyCtl connect [Device Name] [AP SSID]
                 
     Note that when connecting, the controller makes several attempts (approx
     10 connect/re-connect retries, one per second) until the profile is
     associated with an AP.

3. Disconnection mode

     Attempt to disconnect an existing network connection (profile) in the 
     Odyssey Client Manager GUI. The connection must exists and maybe added 
     using "OdysseyCtl add ..." command. If the connection is currently 
     associated then a disassociation will occur. If not, then nothing 
     happens. The following command line format is required:

                 OdysseyCtl disconn [AP SSID]
                 
     Note that a disconnection attempt will simply disassociate a profile
     and not remove the profile from the client manager.

4. Remove   mode

     Attempt to remove an existing network connection (profile) in the Odyssey 
     Client Manager GUI. If the connection is currently associated then a 
     disassociation will occur before the network connection is removed. The 
     following command line format is required:

                 OdysseyCtl remove [AP SSID]

     If the profile is currently associated, it will first be disassociated
     (similar to disconnection mode) before the profile is removed from the
     client manager.
     
     
DEBUGGING NOTES:

The OdysseyCtl produces quite a few debugging messages to provide easier 
analysis when failure occurs. Majority of the failures can come from timing
issues when sending control messages to the Odyssey Client Manager GUI. Internally,
the OdysseyCtl performs retry attempts when failure occurs so these timing
issues are kept to a minimum. When these attempts eventually fails, the 
controller gives up permanently and diagnosis can be done via the debugging
messages.