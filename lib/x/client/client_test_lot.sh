#!/bin/bash

# to test loading alot of files
# have files named with numbers at the end, ofr example if there is
# a directory images/ with files in it named lips.png1 lips.png2 .. 
# upto 1200
# to upload all the files call
# ./client_test_lot.sh images/lips.png 1200

HOST=127.0.0.1 
PORT=9999
i=0

while ((i++ < 1200));do 
	echo "$1$i"; 
	./cli $HOST $PORT PUT "$1$i";
done;

