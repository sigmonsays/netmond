#!/bin/bash

if [ -z $1 ] ; then
	port=6969
fi
echo "Listening on port $port..."
while true
do
	nc -l -p $port
	echo -n "."
	sleep 1
	echo "."
done
