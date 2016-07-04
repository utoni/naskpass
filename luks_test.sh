#!/bin/sh
set -e


if [ "x$1" = "x" ] || [ "x$2" = "x" ]; then
	echo "$0: [FILE] [NAME]"
	exit 1
fi
FILE=$1
NAME=$2

if [ ! -w ${FILE} ]; then
	dd if=/dev/zero of=${FILE} bs=1M count=10
	/sbin/cryptsetup luksFormat ${FILE}
fi

sudo src/naskpass -l -f ./testfifo -c "/sbin/cryptsetup open ${FILE} ${NAME}"

set +e
sudo /sbin/cryptsetup status ${NAME}
ret=$?
set -e

if [ $ret -eq 0 ]; then
	sudo /sbin/cryptsetup close ${NAME}
	/bin/echo -e "\n$0: close'd"
fi
