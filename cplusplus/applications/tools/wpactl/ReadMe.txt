WPA key install on windows.

Author:        Victor Fajardo
Created:       October 26, 2004
Last Updated:  October 26, 2004

1. Summary of methods used to configure 802.11 adapters:

	1.1 Wireless Zero Configuration Service (WZC) - WPS (Wireless 
		provisioning service) is used to install a XML based 
		"profile" (WPA profile consist of SSID, neworkkey, 
		authentication type, encryption type,  802.1x enable 
		flag) into WZC and let it perform the 4-way handshake 
		(pairwise) and 2-way handshake (group). The advantage 
		is that it is easy to load a WPA key (or even an open 
		auth) with this method. The dis-advantage is that I 
		still have not yet found a way to remove/disable a newly 
		installed profile. The profile is index by the SSID and 
		overwriting a profile is not allowed.

	1.2 NDIS (Raw driver IO using DeviceIoControl, synonymous 
		to UNIX ioctl) - Allows direct access to the driver 
		using OID's (Object Id's) associated with SSID's, 
		networkkey, authentication. The advantage is that 
		it's simple to control the device and set the same 
		parameters as in WZC. The dis-advantage is that it 
		does'nt provide 4-way handshaked. The application 
		becomes responsible for writing it.

2. wpactl.exe - This is a command line executable that can 
    be used to perform the following task:

	2.1 Enabling open authentication - Disables Wireless Zero 
		Configuration service (WZCSVC) in windows and perform a 
		driver level instruction to authenticate with an SSID 
		using open mode. In this scenario, we are bypassing 
		WZCSVC and controling the adapter's driver. The tool 
		prints an extensive usage output that gives more details 
		on how to perform open authentication. An example of 
		this is:

		wpactl enable [Wireless Adapter Name] [SSID of AP]

		Note that the [Wireless Adapter Name] is the friendly name
		of the windows adapter. The tool will try to do a longest
		match of the name passed to it and the name stored in the
		device driver. [SSID of AP] must be a valid SSID.

	2.2 Enable WPA-PSK authentication - Enables Wireless Zero 
		Configuration service (WZCSVC) in windows. It then
		creates and installs a "WZCSVC profile" that contains 
		WPA-PSK authentication details (SSID of AP, network key,
		connection type, encryption ... etc.). If the WZCSVC
		successfully loads the profile, it will attempt to 
		associate with the AP and perform a 4-way handshake.
		If the handshake is successful the association is
		completed. In this scenario, the tool relys on WZCSVC 
		to perform all these task based on the loaded profile.
		The tool prints an extensive usage output that gives 
		more details on how to perform WPA-PSK authentication. 
		An example of this is:

			wpactl enable [Wireless Adapter Name] [SSID of AP] [WPA Key]

		Note that the WPA Key is 64 hex characters representing
		a 256-bit pre-shared key.

	2.3 Disable current association - Disables Wireless Zero 
		Configuration service (WZCSVC) in windows and then
		instruct the driver to disable any current association.
		It is important to note that dis-associating the adapter
		will NOT remove a WZCSVC profile that was previously
		loaded. This has to be done manually. The tool prints 
		an extensive usage output that gives more details on 
		how to perform WPA-PSK authentication. An example of 
		this is:
	     
			wpactl disable [Wireless Adapter Name]

		The wpactl.exe can be invoked inside the authentication 
		script (See 2.2). Since the authentication script is 
		invoked upon successful PANA authentication, wpactl 
		can be used to install a PMK from the auth script. Upon 
		disconnection, wpactl can be used to disassociate the 
		wireless adapter and re-associate using open authentication. 

3. Building WPACtl

   The following are requirements for building wpactl.exe:
   
	a. Windows XP SP2
	b. Windows DDK for XP
	c. WPS SDK (Wireless Provisioning Service SDK)
	
   The rest of the requirements are those describe in the
   README document of Open Diameter as it relates to Windows.