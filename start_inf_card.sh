#!/bin/bash
cd build
while :
do
	./ControllUnit --config-file="../card_config.json"
	echo "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- next start"
	sleep 5
done
