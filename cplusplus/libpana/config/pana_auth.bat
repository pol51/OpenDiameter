REM Dis-associate from current AP
wpactl disable "Intel(R) PRO/Wireless"

REM Check to make sure auth is adding.
IF %1 == del GOTO OPEN_MODE

REM Argument format: <add|del> 'PaC MAC ADDR' 'unused' 'unused' 'unused' 'unused' 'unused' 'PMK(1)' 'PMK(2)' 'PMK(n) 'ISP info(including VID)' 'unused' 'EP IP addr(1)' 'EP IP addr(2)' 'EP IP addr(n)'
REM Note: This batch file only support up to two (2) PMK installs
REM Re-associate using authentication keys
REM Let the interface renew dhcp

SET KEY0=%8
SET PMK0=%KEY0:~0,64%
wpactl enable "Intel(R) PRO/Wireless" wa7000-net0 %PMK0%

SET KEY1=%9
SET PMK1=%KEY1:~0,64%
wpactl enable "Intel(R) PRO/Wireless" wa7000-net1 %PMK1%

rem ipconfig /release
GOTO DONE

REM Use open mode authentication
:OPEN_MODE
wpactl enable "Intel(R) PRO/Wireless" wa7000-net

REM Exit
:DONE
