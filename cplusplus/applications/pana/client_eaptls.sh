#!/bin/sh
#
# start/stop pac

curDir=`pwd`
configDir=$curDir/config
PATH=$PATH:.

case "$1" in
    start)
        echo -n "Starting PANA Client/Diameter: "
	echo -n "Use identity: ohba@um.es"
	sleep 5s
        client_pana_eaptls -f $configDir/pac6.xml
	echo "done."
	;;
    stop)
        echo -n "Shutdown PANA Client/Diameter: "
        kill -9 `ps ax --no-headers | grep -aE "client_pana_eaptls -f"` 2> /dev/null
        echo "done."
	;;
    *)
	echo "Usage: ./client_eaptls.sh {start|stop}"
	exit 1
	;;
esac

exit 0
