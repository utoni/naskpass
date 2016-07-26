#!/bin/sh
set -e
set -x


if [ "x$1" = "x" ]; then
	echo "$0: [FILE]"
	exit 1
fi
FILE=$1
NAME=$(basename ${FILE})

if [ ! -w ${FILE} ] || [ `file ${FILE} | grep -qoE 'LUKS encrypted file' && echo 0 || echo 1` -ne 0 ]; then
	dd if=/dev/zero of=${FILE} bs=1M count=10
	/sbin/cryptsetup luksFormat ${FILE}
fi

sudo src/naskpass -l -f ./${NAME}.fifo -c "/sbin/cryptsetup open ${FILE} ${NAME}"

set +e
sudo /sbin/cryptsetup status ${NAME}
ret=$?
set -e

if [ $ret -eq 0 ]; then
	sudo /sbin/cryptsetup close ${NAME}
	/bin/echo -e "\n$0: close'd"
fi
