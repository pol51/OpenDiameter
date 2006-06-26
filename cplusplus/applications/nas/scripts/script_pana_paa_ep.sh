#!/bin/bash

echo "******************** PANA PAA EP SCRIPT ********************"

#
# This script is called by PANA PAA during the 
# following events:
#
# 1. Authentication succeeds
# 2. PANA IP address update occurs
# 3. PANA session terminates/timeouts/disconnects
#
# This script is called by PANA PAA with the 
# a set of arguments that can be used by the
# local system to configure necessary EP functions.
# 
# Argument list in order:
#
# 1. "add" or "del" or "update" string respective
#    of the events above
# 2. PaC device ID
# 3. PAA device ID
# 4. AAA-Key if present
# 5. AAA-Key ID if AAA-Key is present
# 6. Authentication lifetime in seconds
# 7. Protection capabilities flag (defined in PANA draft)
# 8. Generated DHCP key if DHCP bootstrap is enabled
# 9. List of PMK key if WPA bootstrapping is enabled
# 10. Name of prefered ISP
# 11. Name of prefered NAP
# 12. List of EP Device Id
#

for i in $*; do
  echo " ARG: $i";
done