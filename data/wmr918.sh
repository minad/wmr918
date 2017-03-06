#! /bin/sh
#
# wmr918.sh	RC-script to start the WMR918 weather station daemon
#		Written by Andre Colomb <acolomb@web.de>
#               Modified by Daniel Mendler
#
# $Id: wmr918.sh,v 1.1.1.1 2003/11/20 20:08:17 daniel Exp $
#

PROGRAM=wmr918
RED='\033[31m'     
GREEN='\033[32m'    
RESET='\033[m'

test -f $PROGRAM || exit 1

case "$1" in
    start)
        echo -n "Starting $PROGRAM: "
	start-stop-daemon --start -ox $PROGRAM > /dev/null 2>&1
        ;;
    stop)
	echo -n "Stopping $PROGRAM: "
	start-stop-daemon --stop -ox $PROGRAM > /dev/null 2>&1
	;;
    restart)
        $0 stop
        $0 start
        exit $?
	;;
    *)
	echo "Usage: $0 {start|stop|restart}"
	exit 1
	;;
esac

if [ $? == 0 ]
then
    echo -e "${GREEN}Done${RESET}"
else
    echo -e "${RED}Failed${RESET}"
fi 

exit $?
